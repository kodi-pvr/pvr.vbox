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
#include "../Channel.h"
#include "../xmltv/Utilities.h"

using namespace tinyxml2;
using namespace vbox;
using namespace vbox::response;

std::string Content::GetString(const std::string &parameter) const
{
  return std::string(m_content->GetText());
}

int Content::GetInteger(const std::string &parameter) const
{
  int value;
  m_content->QueryIntText(&value);

  return value;
}

std::vector<Channel> XMLTVResponseContent::GetChannels() const
{
  std::vector<Channel> channels;

  // Channels don't have any number, although they appear ordered in the XML
  unsigned int channelNumber = 1;

  for (XMLElement *element = m_content->FirstChildElement("channel");
    element != NULL; element = m_content->NextSiblingElement("channel"))
  {
    Channel channel = CreateChannel(element);
    channel.SetNumber(++channelNumber);
    channels.push_back(channel);
  }

  return channels;
}

Channel XMLTVResponseContent::CreateChannel(const tinyxml2::XMLElement *xml) const
{
  // Extract data from the various <display-name> elements
  const XMLElement *displayElement = xml->FirstChildElement("display-name");

  std::string name = displayElement->GetText();
  displayElement = displayElement->NextSiblingElement("display-name");
  std::string type = displayElement->GetText();

  // Skip the "unique name", we won't be needing it
  displayElement = displayElement->NextSiblingElement("display-name");
  displayElement = displayElement->NextSiblingElement("display-name");
  std::string encryption = displayElement->GetText();

  // Create the channel with some basic information
  Channel channel(xml->Attribute("id"), name,
    xml->FirstChildElement("url")->Attribute("src"));

  // Set icon URL if it exists
  const char *iconUrl = xml->FirstChildElement("icon")->Attribute("src");
  if (iconUrl != NULL)
    channel.SetIconUrl(iconUrl);

  // Set radio and encryption status
  channel.SetRadio(type == "Radio");
  channel.SetEncrypted(encryption == "Encrypted");

  return channel;
}

std::vector<Recording> RecordingResponseContent::GetRecordings() const
{
  std::vector<Recording> recordings;

  for (XMLElement *element = m_content->FirstChildElement("record");
    element != NULL; element = m_content->NextSiblingElement("record"))
  {
    Recording recording = CreateRecording(element);
    recordings.push_back(recording);
  }

  return recordings;
}

Recording RecordingResponseContent::CreateRecording(const tinyxml2::XMLElement *xml) const
{
  // Extract mandatory properties
  std::string channelId = xml->Attribute("channel");
  std::string channelName = xml->FirstChildElement("channel-name")->GetText();
  unsigned int id;
  xml->FirstChildElement("record-id")->QueryUnsignedText(&id);
  RecordingState state = GetState(xml->FirstChildElement("state")->GetText());

  // Construct the object
  Recording recording(id, channelId, channelName, state);

  // Add additional properties
  recording.m_start = xmltv::Utilities::ConvertXMLTVTime(xml->Attribute("start"));

  // TODO: External recordings don't have an end time, default to one hour
  if (xml->Attribute("end") != NULL)
    recording.m_end = xmltv::Utilities::ConvertXMLTVTime(xml->Attribute("end"));
  else
    recording.m_end = recording.m_start + 86400;

  if (xml->FirstChildElement("programme-title"))
    recording.m_title = xml->FirstChildElement("programme-title")->GetText();

  if (xml->FirstChildElement("url"))
    recording.m_url = xml->FirstChildElement("url")->GetText();

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
  else
    return RecordingState::EXTERNAL;
}
