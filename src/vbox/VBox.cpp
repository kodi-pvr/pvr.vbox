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
#include <chrono>
#include <algorithm>
#include "platform/util/timeutils.h"
#include "../client.h"
#include "../compat.h"
#include "ContentIdentifier.h"
#include "Exceptions.h"
#include "Utilities.h"
#include "response/Factory.h"
#include "request/Request.h"
#include "request/FileRequest.h"
#include "response/Content.h"
#include "../xmltv/Utilities.h"

using namespace ADDON;
using namespace vbox;

const char * VBox::MINIMUM_SOFTWARE_VERSION = "2.47";

VBox::VBox(const Settings &settings)
  : m_settings(settings), m_currentChannel("dummy", "dummy", "dummy", "dummy")
{
}

VBox::~VBox()
{
  // Wait for the background thread to stop
  m_active = false;

  if (m_backgroundThread.joinable())
    m_backgroundThread.join();
}

void VBox::Initialize()
{
  // Determine which connection parameters should be used
  DetermineConnectionParams();

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
  Log(LOG_INFO, std::string("      number of tuners: " + compat::to_string(boardInfo.GetInteger("TunersNumber"))).c_str());

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

  // Start the background updater thread
  m_active = true;
  m_backgroundThread = std::thread([this]()
  {
    BackgroundUpdater();
  });
}

void VBox::DetermineConnectionParams()
{
  // Attempt to perform a request using the internal connection parameters
  m_currentConnectionParameters = m_settings.m_internalConnectionParams;

  try {
    request::Request request("QuerySwVersion");
    request.SetTimeout(m_currentConnectionParameters.timeout);
    response::ResponsePtr response = PerformRequest(request);
  }
  catch (VBoxException&)
  {
    // Retry the request with the external parameters
    if (m_settings.m_externalConnectionParams.AreValid())
    {
      Log(LOG_INFO, "Unable to connect using internal connection settings, trying with external");
      m_currentConnectionParameters = m_settings.m_externalConnectionParams;

      request::Request request("QuerySwVersion");
      request.SetTimeout(m_currentConnectionParameters.timeout);
      response::ResponsePtr response = PerformRequest(request);
    }
  }

  auto &params = m_currentConnectionParameters;
  Log(LOG_INFO, "Connection parameters used: ");
  Log(LOG_INFO, "    Hostname: %s", params.hostname.c_str());
  Log(LOG_INFO, "    HTTP port: %d", params.httpPort);
  Log(LOG_INFO, "    UPnP port: %d", params.upnpPort);
}

void VBox::BackgroundUpdater()
{
  // Keep count of how many times the loop has run so we can perform some 
  // tasks only on some iterations
  static unsigned int lapCounter = 1;

  // Retrieve everything in order once before starting the loop, without 
  // triggering the event handlers
  RetrieveChannels(false);
  RetrieveRecordings(false);
  RetrieveGuide(false);

  if (m_settings.m_useExternalXmltv)
    RetrieveExternalGuide();

  while (m_active)
  {
    // Update recordings every iteration
    RetrieveRecordings();

    // Update channels every six iterations = 30 seocnds
    if (lapCounter % 6 == 0)
      RetrieveChannels();

    // Update the internal guide data every 12 * 60 iterations = 1 hour
    if (lapCounter % (12 * 60) == 0)
      RetrieveGuide();

    // Update the external guide data every 12 * 60 * 12 = 12 hours
    if (m_settings.m_useExternalXmltv && lapCounter % (12 * 60 * 12) == 0)
      RetrieveExternalGuide();

    lapCounter++;
    usleep(5000 * 1000); // for some infinitely retarded reason, std::thread::sleep_for doesn't work
  }
}

bool VBox::ValidateSettings() const
{
  // Check connection settings
  if (!m_settings.m_internalConnectionParams.AreValid())
    return false;

  // Check guide settings
  if (m_settings.m_useExternalXmltv && m_settings.m_externalXmltvPath.empty())
    return false;

  // Check timeshift settings
  if (m_settings.m_timeshiftEnabled && !XBMC->CanOpenDirectory(m_settings.m_timeshiftBufferPath.c_str()))
    return false;

  return true;
}

const Settings& VBox::GetSettings() const
{
  return m_settings;
}

const ConnectionParameters& VBox::GetConnectionParams() const
{
  return m_currentConnectionParameters;
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
  return m_currentConnectionParameters.hostname;
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
  ss << GetBackendHostname() << ":" << m_currentConnectionParameters.httpPort;

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
    return uniqueId == ContentIdentifier::GetUniqueId(channel.get());
  });

  if (it == m_channels.cend())
    return nullptr;

  return it->get();
}

const Channel& VBox::GetCurrentChannel() const
{
  return m_currentChannel;
}

void VBox::SetCurrentChannel(const Channel &channel)
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

  // Only attempt to parse the status if streaming is active
  std::string active = content.GetString("Active");

  if (active == "YES")
  {
    status.m_active = true;
    status.SetServiceId(content.GetUnsignedInteger("SID"));
    status.SetTunerId(content.GetString("TunerID"));
    status.SetTunerType(content.GetString("TunerType"));
    status.m_lockStatus = content.GetString("LockStatus");
    status.m_modulation = content.GetString("Modulation");
    status.m_frequency = content.GetString("Frequency");
    status.SetRfLevel(content.GetString("RFLevel"));
    status.m_signalQuality = content.GetUnsignedInteger("SignalQuality");
    status.SetBer(content.GetString("BER"));
  }

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

request::Request VBox::CreateDeleteRecordingRequest(const RecordingPtr &recording) const
{
  RecordingState state = recording->GetState();
  unsigned int recordId = recording->m_id;

  // Determine the request method to use. If a recording is active we want to 
  // cancel it instead of deleting it
  std::string requestMethod = "DeleteRecord";

  if (state == RecordingState::RECORDING)
    requestMethod = "CancelRecord";

  // Create the request
  request::Request request(requestMethod);
  request.AddParameter("RecordID", recordId);

  // Determine request parameters
  if (state == RecordingState::EXTERNAL)
    request.AddParameter("FileName", recording->m_filename);

  return request;
}

bool VBox::DeleteRecordingOrTimer(unsigned int id)
{
  m_stateHandler.WaitForState(StartupState::RECORDINGS_LOADED);

  // Find the recording/timer
  auto it = std::find_if(m_recordings.begin(), m_recordings.end(), 
    [id](const RecordingPtr &recording)
  {
    return id == ContentIdentifier::GetUniqueId(recording.get());
  });

  if (it == m_recordings.end())
    return false;

  // The request fails if the item doesn't exist
  try {
    request::Request request = CreateDeleteRecordingRequest(*it);
    response::ResponsePtr response = PerformRequest(request);

    // Delete the item from memory too
    std::unique_lock<std::mutex> lock(m_mutex);
    
    if (it != m_recordings.end())
      m_recordings.erase(it);

    // Fire events
    OnRecordingsUpdated();
    OnTimersUpdated();

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

void VBox::AddTimer(const Channel *channel, time_t startTime, time_t endTime)
{
  // Add the timer
  request::Request request("ScheduleChannelRecord");
  request.AddParameter("ChannelID", channel->m_xmltvName);
  request.AddParameter("StartTime", ::xmltv::Utilities::UnixTimeToXmltv(startTime));
  request.AddParameter("EndTime", ::xmltv::Utilities::UnixTimeToXmltv(endTime));
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
  return m_guide.GetProgramme(programmeUniqueId);
}

std::string VBox::GetApiBaseUrl() const
{
  std::stringstream ss;
  ss << "http://" << m_currentConnectionParameters.hostname;
  ss << ":" << m_currentConnectionParameters.httpPort;
  ss << "/cgi-bin/HttpControl/HttpControlApp?OPTION=1";

  return ss.str();
}

void VBox::RetrieveChannels(bool triggerEvent/* = true*/)
{
  try {
    request::Request request("GetXmltvChannelsList");
    request.AddParameter("FromChIndex", "FirstChannel");
    request.AddParameter("ToChIndex", "LastChannel");
    response::ResponsePtr response = PerformRequest(request);
    response::XMLTVResponseContent content(response->GetReplyElement());

    // Swap and notify if the contents have changed
    auto channels = content.GetChannels();
    std::unique_lock<std::mutex> lock(m_mutex);

    if (!utilities::deref_equals(m_channels, channels))
    {
      m_channels = std::move(channels);

      if (triggerEvent)
        OnChannelsUpdated();
    }
  }
  catch (VBoxException &e)
  {
    LogException(e);
  }

  if (m_stateHandler.GetState() < StartupState::CHANNELS_LOADED)
    m_stateHandler.EnterState(StartupState::CHANNELS_LOADED);
}

void VBox::RetrieveRecordings(bool triggerEvent/* = true*/)
{
  // Only attempt to retrieve recordings when external media is present
  if (m_externalMediaStatus.present)
  {
    try {
      request::Request request("GetRecordsList");
      request.AddParameter("Externals", "YES");
      response::ResponsePtr response = PerformRequest(request);
      response::RecordingResponseContent content(response->GetReplyElement());

      // Compare the results
      auto recordings = content.GetRecordings();
      std::unique_lock<std::mutex> lock(m_mutex);

      // Swap and notify if the contents have changed
      if (!utilities::deref_equals(m_recordings, recordings))
      {
        m_recordings = std::move(content.GetRecordings());

        if (triggerEvent)
        {
          OnRecordingsUpdated();
          OnTimersUpdated();
        }
      }
    }
    catch (VBoxException &e)
    {
      LogException(e);
    }
  }

  if (m_stateHandler.GetState() < StartupState::RECORDINGS_LOADED)
    m_stateHandler.EnterState(StartupState::RECORDINGS_LOADED);
}

void VBox::RetrieveGuide(bool triggerEvent/* = true*/)
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

    xmltv::Guide guide;

    for (int fromIndex = 1; fromIndex <= lastChannelIndex; fromIndex += 10)
    {
      // Abort immediately if the addon just got terminated
      if (!m_active)
        return;

      int toIndex = std::min(fromIndex + 9, lastChannelIndex);

      request::Request request("GetXmltvSection");
      request.AddParameter("FromChIndex", fromIndex);
      request.AddParameter("ToChIndex", toIndex);
      response::ResponsePtr response = PerformRequest(request);
      response::XMLTVResponseContent content(response->GetReplyElement());

      auto partialGuide = content.GetGuide();
      guide += partialGuide;
    }

    LogGuideStatistics(guide);

    // Swap the guide with the new one
    std::unique_lock<std::mutex> lock(m_mutex);
    m_guide = guide;

    if (triggerEvent)
      OnGuideUpdated();
  }
  catch (VBoxException &e)
  {
    LogException(e);
  }

  if (m_stateHandler.GetState() < StartupState::GUIDE_LOADED)
    m_stateHandler.EnterState(StartupState::GUIDE_LOADED);
}

void VBox::RetrieveExternalGuide(bool triggerEvent/* = true*/)
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

  if (triggerEvent)
    OnGuideUpdated();

  if (m_stateHandler.GetState() < StartupState::EXTERNAL_GUIDE_LOADED)
    m_stateHandler.EnterState(StartupState::EXTERNAL_GUIDE_LOADED);
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
    std::unique_ptr<std::string> responseContent = utilities::ReadFileContents(fileHandle);
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
