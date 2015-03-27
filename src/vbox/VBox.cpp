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

#include "VBox.h"
#include <string>
#include <sstream>
#include <thread>
#include <algorithm>
#include "../client.h"
#include "Exceptions.h"
#include "response/Factory.h"
#include "request/Request.h"
#include "request/FileRequest.h"
#include "response/Content.h"
#include "xmltv/Utilities.h"

using namespace ADDON;
using namespace vbox;

const char * VBox::MINIMUM_SOFTWARE_VERSION = "2.46.20";

VBox::VBox(const Settings &settings)
  : m_settings(settings), m_stateHandler(settings.m_timeout),
  m_currentChannel(nullptr)
{
}

VBox::~VBox()
{
}

void VBox::Initialize()
{
  // Query the software version, we need a few elements from that response
  request::Request versionRequest("QuerySwVersion");
  response::ResponsePtr response = PerformRequest(versionRequest);
  response::Content versionContent(response->GetReplyElement());

  // Query the board info, we need some elements from that as well
  request::Request boardRequest("QueryBoardInfo");
  response::ResponsePtr boardResponse = PerformRequest(boardRequest);
  response::Content boardInfo(boardResponse->GetReplyElement());

  // Construct the model string
  std::string model = versionContent.GetString("Custom"); // VBox
  model += " " + versionContent.GetString("DeviceType"); // e.g. XTI
  model += " " + boardInfo.GetString("ProductNumber"); // e.g. 3352

  Log(LOG_INFO, "device information: ");
  Log(LOG_INFO, std::string("                 model: " + model).c_str());
  Log(LOG_INFO, std::string("     hardware revision: " + boardInfo.GetString("HWRev")).c_str());
  Log(LOG_INFO, std::string("     firmware revision: " + boardInfo.GetString("FWRev")).c_str());
  Log(LOG_INFO, std::string("         uboot version: " + boardInfo.GetString("UbootVersion")).c_str());
  Log(LOG_INFO, std::string("        kernel version: " + boardInfo.GetString("KernelVersion")).c_str());
  Log(LOG_INFO, std::string("      software version: " + boardInfo.GetString("SoftwareVersion")).c_str());
  Log(LOG_INFO, std::string("      number of tuners: " + std::to_string(boardInfo.GetInteger("TunersNumber"))).c_str());

  m_backendName = model;
  m_backendVersion = SoftwareVersion::ParseString(boardInfo.GetString("SoftwareVersion"));

  // Check that the backend uses a compatible software version
  if (m_backendVersion < SoftwareVersion::ParseString(MINIMUM_SOFTWARE_VERSION))
  {
    std::string error = std::string("Firmware version ") + 
      MINIMUM_SOFTWARE_VERSION + " or higher is required";

    throw FirmwareVersionException(error);
  }

  // Query external media status. The request will error if no external media 
  // is attached
  try {
    request::Request mediaRequest = request::Request("QueryExternalMediaStatus");
    response::ResponsePtr mediaResponse = PerformRequest(mediaRequest);
    response::Content mediaStatus = response::Content(mediaResponse->GetReplyElement());

    m_externalMediaStatus.present = true;
    m_externalMediaStatus.spaceTotal = (int64_t)mediaStatus.GetInteger("TotalMem") * 1048576;
    m_externalMediaStatus.spaceUsed = (int64_t)mediaStatus.GetInteger("UsedMem") * 1048576;
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

    // Retrieve the external guide if configured
    if (m_settings.m_useExternalXmltv)
      RetrieveExternalGuide();

  }).detach();
}

bool VBox::ValidateSettings() const
{
  return !m_settings.m_hostname.empty() &&
    m_settings.m_port != 0 &&
    m_settings.m_timeout != 0;
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
  std::stringstream ss;
  ss << GetBackendHostname() << ":" << m_settings.m_port;

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

const Channel* VBox::GetChannel(unsigned int uniqueId) const
{
  m_stateHandler.WaitForState(StartupState::CHANNELS_LOADED);
  std::unique_lock<std::mutex> lock(m_mutex);

  auto it = std::find_if(m_channels.cbegin(), m_channels.cend(), [uniqueId](const ChannelPtr &channel) {
    return uniqueId == channel->GetUniqueId();
  });

  if (it == m_channels.cend())
    return nullptr;

  return it->get();
}

const Channel* VBox::GetCurrentChannel() const
{
  return m_currentChannel;
}

void VBox::SetCurrentChannel(const Channel* channel)
{
  m_currentChannel = channel;
}

ChannelStreamingStatus VBox::GetChannelStreamingStatus(const Channel* channel) const
{
  ChannelStreamingStatus status;

  request::Request request("QueryChannelStreamingStatus");
  request.AddParameter("ChannelID", channel->m_xmltvName);
  response::ResponsePtr response = PerformRequest(request);
  response::Content content(response->GetReplyElement());

  status.SetServiceId(content.GetUnsignedInteger("SID"));
  status.SetTunerId(content.GetString("TunerID"));
  status.SetTunerType(content.GetString("TunerType"));
  status.m_lockStatus = content.GetString("LockStatus");
  status.m_frequency = content.GetString("Frequency");
  status.SetRfLevel(content.GetString("RFLevel"));
  status.m_signalQuality = content.GetUnsignedInteger("SignalQuality");
  status.SetBer(content.GetString("BER"));

  return status;
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

void VBox::AddTimer(const Channel *channel, const ::xmltv::Programme* programme)
{
  // Add the timer
  request::Request request("ScheduleProgramRecord");
  request.AddParameter("ChannelID", channel->m_xmltvName);
  request.AddParameter("ProgramTitle", programme->m_title);
  request.AddParameter("StartTime", programme->m_startTime);
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

const ::xmltv::Schedule* VBox::GetSchedule(const Channel *channel) const
{
  // Load the schedule
  m_stateHandler.WaitForState(StartupState::GUIDE_LOADED);
  std::unique_lock<std::mutex> lock(m_mutex);

  auto *schedule = m_guide.GetSchedule(channel->m_xmltvName);

  // Try to use the external guide data if a) it's loaded, b) the user prefers 
  // it or c) if no scehdule was found
  if (m_stateHandler.GetState() >= StartupState::EXTERNAL_GUIDE_LOADED &&
    (m_settings.m_preferExternalXmltv || !schedule))
  {
    std::string xmltvName = m_externalGuide.GetChannelId(channel->m_name);

    if (!xmltvName.empty())
    {
      Log(LOG_DEBUG, "Using external guide data for channel %s", channel->m_name.c_str());
      schedule = m_externalGuide.GetSchedule(xmltvName);
    }
  }

  return schedule;
}

const ::xmltv::Programme* VBox::GetProgramme(int programmeUniqueId) const
{
  for (const auto &entry : m_guide.GetSchedules())
  {
    const ::xmltv::SchedulePtr &schedule = entry.second;

    auto it = std::find_if(
      schedule->cbegin(),
      schedule->cend(),
      [programmeUniqueId](const ::xmltv::ProgrammePtr &programme)
    {
      return programme->GetUniqueId() == programmeUniqueId;
    });

    if (it != schedule->cend())
      return it->get();
  }

  return nullptr;
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

      m_guide += partialGuide;
    }

    LogGuideStatistics(m_guide);
  }
  catch (VBoxException &e)
  {
    LogException(e);
  }

  m_stateHandler.EnterState(StartupState::GUIDE_LOADED);
}

void VBox::RetrieveExternalGuide()
{
  Log(LOG_INFO, "Loading external guide data");
  request::FileRequest request(m_settings.m_externalXmltvPath);
  response::ResponsePtr response = PerformRequest(request);
  response::XMLTVResponseContent content(response->GetReplyElement());

  {
    auto externalGuide = content.GetGuide();
    std::unique_lock<std::mutex> lock(m_mutex);
    m_externalGuide = externalGuide;
  }

  LogGuideStatistics(m_externalGuide);
  m_stateHandler.EnterState(StartupState::EXTERNAL_GUIDE_LOADED);
  
  TriggerGuideUpdate();
}

void VBox::TriggerGuideUpdate() const
{
  std::unique_lock<std::mutex> lock(m_mutex);

  for (const auto &channel : m_channels)
  {
    Log(LOG_DEBUG, "Triggering EPG update of channel %s", channel->m_name.c_str());
    PVR->TriggerEpgUpdate(channel->GetUniqueId());
  }
}

void VBox::LogGuideStatistics(const xmltv::Guide &guide) const
{
  std::unique_lock<std::mutex> lock(m_mutex);

  for (const auto &schedule : guide.GetSchedules())
  {
    Log(LOG_INFO, "Fetched %d events for channel %s", schedule.second->size(),
      schedule.first.c_str());
  }
}

response::ResponsePtr VBox::PerformRequest(const request::IRequest &request) const
{
  // Attempt to open a HTTP file handle
  void *fileHandle = XBMC->OpenFile(request.GetLocation().c_str(), 0x08 /* READ_NO_CACHE */);

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
