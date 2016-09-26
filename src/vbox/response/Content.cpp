/*
*      Copyright (C) 2015 Sam Stenvall
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with XBMC; see the file COPYING.  If not, write to
*  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
*  MA 02110-1301  USA
*  http://www.gnu.org/copyleft/gpl.html
*
*/

#include "Content.h"
#include "lib/tinyxml2/tinyxml2.h"
#include "../Channel.h"
#include "../../xmltv/Utilities.h"
#include "../../xmltv/Guide.h"
#include "../../compat.h"

using namespace tinyxml2;
using namespace vbox;
using namespace vbox::response;

std::string Content::GetString(const std::string &parameter) const
{
  const XMLElement *element = GetParameterElement(parameter);
  
  if (element && element->GetText())
    return std::string(element->GetText());

  return "";
}

int Content::GetInteger(const std::string &parameter) const
{
  int value = 0;

  const XMLElement *element = GetParameterElement(parameter);
  if (element)
    value = xmltv::Utilities::QueryIntText(element);

  return value;
}

unsigned int Content::GetUnsignedInteger(const std::string &parameter) const
{
  unsigned int value = 0;

  XMLElement *element = GetParameterElement(parameter);
  if (element)
    value = xmltv::Utilities::QueryUnsignedText(element);

  return value;
}

tinyxml2::XMLElement* Content::GetParameterElement(const std::string &parameter) const
{
  return m_content->FirstChildElement(parameter.c_str());
}

std::vector<ChannelPtr> XMLTVResponseContent::GetChannels() const
{
  std::vector<ChannelPtr> channels;

  // Remember the index each channel had, it's needed for certain API requests
  unsigned int index = 1;

  for (XMLElement *element = m_content->FirstChildElement("channel");
    element != NULL; element = element->NextSiblingElement("channel"))
  {
    ChannelPtr channel = CreateChannel(element);
    channel->m_index = index++;
    channels.push_back(channel);
  }

  return channels;
}

::xmltv::Guide XMLTVResponseContent::GetGuide() const
{
  return ::xmltv::Guide(m_content);
}

ChannelPtr XMLTVResponseContent::CreateChannel(const tinyxml2::XMLElement *xml) const
{
  // Extract data from the various <display-name> elements
  const XMLElement *displayElement = xml->FirstChildElement("display-name");

  std::string name = displayElement->GetText();
  displayElement = displayElement->NextSiblingElement("display-name");
  std::string type = displayElement->GetText();
  displayElement = displayElement->NextSiblingElement("display-name");
  std::string uniqueId = displayElement->GetText();
  displayElement = displayElement->NextSiblingElement("display-name");
  std::string encryption = displayElement->GetText();
  std::string xmltvName = ::xmltv::Utilities::UrlDecode(xml->Attribute("id"));

  // Create the channel with some basic information
  ChannelPtr channel(new Channel(uniqueId, xmltvName, name,
    xml->FirstChildElement("url")->Attribute("src")));

  // Extract the LCN (optional)
  displayElement = displayElement->NextSiblingElement("display-name");

  if (displayElement)
  {
    // The LCN is sometimes just a digit and sometimes lcn_X
    std::string lcnValue = displayElement->GetText();

    if (lcnValue.find("lcn_") != std::string::npos)
      lcnValue = lcnValue.substr(4);

    channel->m_number = compat::stoui(lcnValue);
  }

  // Set icon URL if it exists
  const char *iconUrl = xml->FirstChildElement("icon")->Attribute("src");
  if (iconUrl != NULL)
    channel->m_iconUrl = iconUrl;

  // Set radio and encryption status
  channel->m_radio = type == "Radio";
  channel->m_encrypted = encryption == "Encrypted";

  return channel;
}

std::vector<RecordingPtr> RecordingResponseContent::GetRecordings() const
{
  std::vector<RecordingPtr> recordings;

  for (XMLElement *element = m_content->FirstChildElement("record");
    element != NULL; element = element->NextSiblingElement("record"))
  {
    RecordingPtr recording = CreateRecording(element);
    recordings.push_back(std::move(recording));
  }

  return recordings;
}

RecordingPtr RecordingResponseContent::CreateRecording(const tinyxml2::XMLElement *xml) const
{
  // Extract mandatory properties
  std::string channelId = xmltv::Utilities::UrlDecode(xml->Attribute("channel"));
  std::string channelName = xml->FirstChildElement("channel-name")->GetText();
  RecordingState state = GetState(xml->FirstChildElement("state")->GetText());

  // Construct the object
  RecordingPtr recording(new Recording(channelId, channelName, state));

  // Add additional properties
  recording->m_startTime = xml->Attribute("start");

  if (xml->FirstChildElement("record-id"))
    recording->m_id = xmltv::Utilities::QueryUnsignedText(xml->FirstChildElement("record-id"));

  // TODO: External recordings don't have an end time, default to one hour
  if (xml->Attribute("stop") != NULL)
    recording->m_endTime = xml->Attribute("stop");
  else
    recording->m_endTime = xmltv::Utilities::UnixTimeToXmltv(time(nullptr) + 86400);

  if (xml->FirstChildElement("programme-title"))
    recording->m_title = xml->FirstChildElement("programme-title")->GetText();
  else
  {
    // TODO: Some recordings don't have a name, especially external ones
    if (state == RecordingState::EXTERNAL)
      recording->m_title = "External recording (channel " + channelName + ")";
    else
      recording->m_title = "Unnamed recording (channel " + channelName + ")";
  }

  // Some recordings may have certain tags, but they can be empty
  const XMLElement *element = xml->FirstChildElement("programme-desc");

  if (element && element->GetText())
    recording->m_description = element->GetText();

  element = xml->FirstChildElement("url");

  if (element && element->GetText())
    recording->m_url = element->GetText();

  // Extract the "local target" (filename), it is needed on rare occasions
  element = xml->FirstChildElement("LocalTarget");

  if (element)
    recording->m_filename = element->GetText();

  return recording;
}

RecordingState RecordingResponseContent::GetState(const std::string &state) const
{
  if (state == "recorded")
    return RecordingState::RECORDED;
  else if (state == "recording")
    return RecordingState::RECORDING;
  else if (state == "scheduled")
    return RecordingState::SCHEDULED;
  else if (state == "Error")
    return RecordingState::RECORDING_ERROR;
  else
    return RecordingState::EXTERNAL;
}
