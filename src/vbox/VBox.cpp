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

#include <string>
#include <sstream>
#include <thread>
#include "VBox.h"
#include "../client.h"
#include "Exceptions.h"
#include "response/Factory.h"
#include "request/Request.h"
#include "response/Content.h"
#include "xmltv/Utilities.h"

using namespace ADDON;
using namespace vbox;
using namespace vbox::util;

const char * VBox::MINIMUM_SOFTWARE_VERSION = "2.46.20";

VBox::VBox(const Settings &settings) :
m_settings(settings), m_stateHandler(settings.m_timeout)
{
}

VBox::~VBox()
{
}

void VBox::Initialize()
{
  // Query board info to get backend name and version
  request::Request request("QueryBoardInfo");
  response::ResponsePtr response = PerformRequest(request);
  response::Content content(response->GetReplyElement());

  std::string model = content.GetString("ProductName") + " " + content.GetString("ProductNumber");
  SoftwareVersion softwareVersion =  SoftwareVersion::ParseString(content.GetString("SoftwareVersion"));

  Log(LOG_INFO, "device information: ");
  Log(LOG_INFO, std::string("                 model: " + model).c_str());
  Log(LOG_INFO, std::string("     hardware revision: " + content.GetString("HWRev")).c_str());
  Log(LOG_INFO, std::string("     firmware revision: " + content.GetString("FWRev")).c_str());
  Log(LOG_INFO, std::string("         uboot version: " + content.GetString("UbootVersion")).c_str());
  Log(LOG_INFO, std::string("        kernel version: " + content.GetString("KernelVersion")).c_str());
  Log(LOG_INFO, std::string("      software version: " + softwareVersion.GetString()).c_str());
  Log(LOG_INFO, std::string("      number of tuners: " + std::to_string(content.GetInteger("TunersNumber"))).c_str());

  m_backendName = model;
  m_backendVersion = softwareVersion;

  // Check that the backend uses a compatible software version
  SoftwareVersion minimumVersion = SoftwareVersion::ParseString(MINIMUM_SOFTWARE_VERSION);

  if (m_backendVersion < minimumVersion)
  {
    std::string error = std::string("Firmware version ") + MINIMUM_SOFTWARE_VERSION + " or higher is required";
    throw FirmwareVersionException(error);
  }

  // Query external media status. The request will error if no external media 
  // is attached
  try {
    request = request::Request("QueryExternalMediaStatus");
    response = PerformRequest(request);
    content = response::Content(response->GetReplyElement());

    m_externalMediaStatus.present = true;
    m_externalMediaStatus.spaceTotal = (int64_t)content.GetInteger("TotalMem") * 1048576;
    m_externalMediaStatus.spaceUsed = (int64_t)content.GetInteger("UsedMem") * 1048576;
  }
  catch (VBoxException &e)
  {
    LogException(e);
  }

  // Consider the addon initialized
  m_stateHandler.EnterState(StartupState::INITIALIZED);

  // Import channels, recordings and guide data asynchronously
  std::thread([=]() {
    RetrieveChannels();
    RetrieveRecordings();
    RetrieveGuide();
  }).detach();
}

const Settings& VBox::GetSettings() const
{
  return m_settings;
}

StartupStateHandler& VBox::GetStateHandler()
{
  return m_stateHandler;
}

std::string VBox::GetBackendName() const
{
  if (m_stateHandler.WaitForState(StartupState::INITIALIZED))
    return m_backendName;

  return "";
}

std::string VBox::GetBackendHostname() const
{
  return m_settings.m_hostname;
}

std::string VBox::GetBackendVersion() const
{
  if (m_stateHandler.WaitForState(StartupState::INITIALIZED))
    return m_backendVersion.GetString();

  return "";
}

std::string VBox::GetConnectionString() const
{
  if (!m_stateHandler.WaitForState(StartupState::INITIALIZED))
    return "";

  std::stringstream ss;
  ss << "VBox TV Gateway model ";
  ss << GetBackendName();
  ss << " @ " << GetBackendHostname() << ":" << m_settings.m_port;

  return ss.str();
}

int VBox::GetChannelsAmount() const
{
  m_stateHandler.WaitForState(StartupState::CHANNELS_LOADED);
  std::unique_lock<std::mutex> lock(m_mutex);

  return m_channels.size();
}

const std::vector<ChannelPtr>& VBox::GetChannels() const
{
  m_stateHandler.WaitForState(StartupState::CHANNELS_LOADED);
  std::unique_lock<std::mutex> lock(m_mutex);

  return m_channels;
}

const ChannelPtr& VBox::GetChannel(unsigned int uniqueId) const
{
  m_stateHandler.WaitForState(StartupState::CHANNELS_LOADED);
  std::unique_lock<std::mutex> lock(m_mutex);

  auto it = std::find_if(m_channels.cbegin(), m_channels.cend(), [uniqueId](const ChannelPtr &channel) {
    return uniqueId == channel->GetUniqueId();
  });

  if (it == m_channels.cend())
    throw VBoxException("Unknown channel");

  return *it;
}

bool VBox::SupportsRecordings() const
{
  return m_externalMediaStatus.present;
}

int64_t VBox::GetRecordingTotalSpace() const
{
  return m_externalMediaStatus.spaceTotal;
}

int64_t VBox::GetRecordingUsedSpace() const
{
  return m_externalMediaStatus.spaceUsed;
}

int VBox::GetRecordingsAmount() const
{
  m_stateHandler.WaitForState(StartupState::RECORDINGS_LOADED);
  std::unique_lock<std::mutex> lock(m_mutex);

  return std::count_if(m_recordings.begin(), m_recordings.end(), [](const RecordingPtr &recording) {
    return recording->IsRecording();
  });
}

bool VBox::DeleteRecordingOrTimer(unsigned int id)
{
  m_stateHandler.WaitForState(StartupState::RECORDINGS_LOADED);

  // The request fails if the recording doesn't exist
  try {
    request::Request request("DeleteRecord");
    request.AddParameter("RecordID", id);
    response::ResponsePtr response = PerformRequest(request);

    // Delete the recording from memory too
    std::unique_lock<std::mutex> lock(m_mutex);
    
    auto it = std::find_if(m_recordings.begin(), m_recordings.end(), [id](const RecordingPtr &recording) {
      return recording->m_id == id;
    });

    if (it != m_recordings.end())
      m_recordings.erase(it);

    return true;
  }
  catch (VBoxException &e)
  {
    LogException(e);
  }

  return false;
}

void VBox::AddTimer(const ChannelPtr &channel, time_t startTime, time_t endTime)
{
  // Add the timer
  request::Request request("ScheduleChannelRecord");
  request.AddParameter("ChannelID", channel->m_xmltvName);
  request.AddParameter("StartTime", xmltv::Utilities::UnixTimeToXmltv(startTime));
  request.AddParameter("EndTime", xmltv::Utilities::UnixTimeToXmltv(endTime));
  PerformRequest(request);

  // Refresh the recordings and timers
  RetrieveRecordings();
}

int VBox::GetTimersAmount() const
{
  m_stateHandler.WaitForState(StartupState::RECORDINGS_LOADED);
  std::unique_lock<std::mutex> lock(m_mutex);

  return std::count_if(m_recordings.begin(), m_recordings.end(), [](const RecordingPtr &recording) {
    return recording->IsTimer();
  });
}

const std::vector<RecordingPtr>& VBox::GetRecordingsAndTimers() const
{
  m_stateHandler.WaitForState(StartupState::RECORDINGS_LOADED);
  std::unique_lock<std::mutex> lock(m_mutex);

  return m_recordings;
}

bool VBox::HasSchedule(const ChannelPtr &channel) const
{
  // Wait until the guide has been retrieved
  m_stateHandler.WaitForState(StartupState::GUIDE_LOADED);

  std::unique_lock<std::mutex> lock(m_mutex);
  return m_guide.find(channel->m_xmltvName) != m_guide.cend();
}

const xmltv::SchedulePtr& VBox::GetSchedule(const ChannelPtr &channel)
{
  // Wait until the guide has been retrieved
  m_stateHandler.WaitForState(StartupState::GUIDE_LOADED);

  std::unique_lock<std::mutex> lock(m_mutex);

  // Return an empty schedule if no guide data exists for the specified channel
  if (m_guide.find(channel->m_xmltvName) == m_guide.cend())
    return xmltv::CreateSchedule();

  return m_guide[channel->m_xmltvName];
}

std::string VBox::GetApiBaseUrl() const
{
  return "http://" + m_settings.m_hostname + "/cgi-bin/HttpControl/HttpControlApp?OPTION=1";
}

void VBox::RetrieveChannels()
{
  try {
    request::Request request("GetXmltvChannelsList");
    request.AddParameter("FromChIndex", "FirstChannel");
    request.AddParameter("ToChIndex", "LastChannel");
    response::ResponsePtr response = PerformRequest(request);
    response::XMLTVResponseContent content(response->GetReplyElement());

    std::unique_lock<std::mutex> lock(m_mutex);
    m_channels = std::move(content.GetChannels());
  }
  catch (VBoxException &e)
  {
    LogException(e);
  }

  m_stateHandler.EnterState(StartupState::CHANNELS_LOADED);
}

void VBox::RetrieveRecordings()
{
  // Only attempt to retrieve recordings when external media is present
  if (m_externalMediaStatus.present)
  {
    try {
      request::Request request("GetRecordsList");
      request.AddParameter("Externals", "YES");
      response::ResponsePtr response = PerformRequest(request);
      response::RecordingResponseContent content(response->GetReplyElement());

      std::unique_lock<std::mutex> lock(m_mutex);
      m_recordings = std::move(content.GetRecordings());
    }
    catch (VBoxException &e)
    {
      LogException(e);
    }
  }

  m_stateHandler.EnterState(StartupState::RECORDINGS_LOADED);
}

void VBox::RetrieveGuide()
{
  Log(LOG_INFO, "Fetching guide data from backend (this will take a while)");

  try {
    // Retrieving the whole XMLTV file is too slow so we fetch sections in 
    // batches of 10 channels and merge the results
    int lastChannelIndex = 0;

    {
      std::unique_lock<std::mutex> lock(m_mutex);
      lastChannelIndex = m_channels.size();
    }

    for (int fromIndex = 1; fromIndex <= lastChannelIndex; fromIndex += 10)
    {
      int toIndex = std::min(fromIndex + 9, lastChannelIndex);

      request::Request request("GetXmltvSection");
      request.AddParameter("FromChIndex", fromIndex);
      request.AddParameter("ToChIndex", toIndex);
      response::ResponsePtr response = PerformRequest(request);
      response::XMLTVResponseContent content(response->GetReplyElement());

      auto partialGuide = content.GetGuide();
      std::unique_lock<std::mutex> lock(m_mutex);

      for (auto &entry : partialGuide)
        m_guide[entry.first] = std::move(entry.second);
    }

    // Loop through the guide once for logging purposes
    std::unique_lock<std::mutex> lock(m_mutex);

    for (const auto &schedule : m_guide)
    {
      Log(LOG_INFO, "Fetched %d events for channel %s", schedule.second->size(),
        schedule.first.c_str());
    }
  }
  catch (VBoxException &e)
  {
    LogException(e);
  }

  m_stateHandler.EnterState(StartupState::GUIDE_LOADED);
}

response::ResponsePtr VBox::PerformRequest(const request::Request &request) const
{
  // Attempt to open a HTTP file handle
  void *fileHandle = XBMC->OpenFile(request.GetUrl().c_str(), 0x08 /* READ_NO_CACHE */);

  if (fileHandle)
  {
    // Read the response string
    std::unique_ptr<std::string> responseContent(new std::string());

    char buffer[1024];
    while (XBMC->ReadFileString(fileHandle, buffer, sizeof(buffer) - 1))
      responseContent->append(buffer);

    XBMC->CloseFile(fileHandle);

    // Parse the response
    response::ResponsePtr response = response::Factory::CreateResponse(request);
    response->ParseRawResponse(*responseContent.get());

    // Check if the response was successful
    if (!response->IsSuccessful())
    {
      std::stringstream ss;
      ss << response->GetErrorDescription();
      ss << " (error code: " << static_cast<int>(response->GetErrorCode()) << ")";

      throw InvalidResponseException(ss.str());
    }

    return response;
  }

  // The request failed completely
  throw RequestFailedException("Unable to perform request");
}

void VBox::Log(const ADDON::addon_log level, const char *format, ...)
{
  char buf[16384];
  size_t c = sprintf(buf, "pvr.vbox - ");
  va_list va;
  va_start(va, format);
  vsnprintf(buf + c, sizeof(buf) - c, format, va);
  va_end(va);
  XBMC->Log(level, "%s", buf);
}

void VBox::LogException(VBoxException &e)
{
  std::string message = "Request failed: " + std::string(e.what());
  Log(LOG_ERROR, message.c_str());
}
