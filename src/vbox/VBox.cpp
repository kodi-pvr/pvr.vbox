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

using namespace ADDON;
using namespace vbox;
using namespace vbox::util;

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
  std::string softwareVersion = content.GetString("SoftwareVersion");

  Log(LOG_INFO, "device information: ");
  Log(LOG_INFO, std::string("                 model: " + model).c_str());
  Log(LOG_INFO, std::string("     hardware revision: " + content.GetString("HWRev")).c_str());
  Log(LOG_INFO, std::string("     firmware revision: " + content.GetString("FWRev")).c_str());
  Log(LOG_INFO, std::string("         uboot version: " + content.GetString("UbootVersion")).c_str());
  Log(LOG_INFO, std::string("        kernel version: " + content.GetString("KernelVersion")).c_str());
  Log(LOG_INFO, std::string("      software version: " + softwareVersion).c_str());
  Log(LOG_INFO, std::string("      number of tuners: " + content.GetInteger("TunersNumber")).c_str());

  m_backendName = model;
  m_backendVersion = softwareVersion;

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
    return m_backendVersion;

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
    return !recording->IsTimer();
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

void VBox::AddTimer(const std::string channelId, time_t startTime, time_t endTime)
{
  // TODO: Implement
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

std::string VBox::GetApiBaseUrl() const
{
  return "http://" + m_settings.m_hostname + "/cgi-bin/HttpControl/HttpControlApp?OPTION=1";
}

void VBox::RetrieveChannels()
{
  request::Request request("GetXmltvChannelsList");
  request.AddParameter("FromChIndex", "FirstChannel");
  request.AddParameter("ToChIndex", "LastChannel");
  response::ResponsePtr response = PerformRequest(request);
  response::XMLTVResponseContent content(response->GetReplyElement());
  
  std::unique_lock<std::mutex> lock(m_mutex);
  m_channels = std::move(content.GetChannels());

  m_stateHandler.EnterState(StartupState::CHANNELS_LOADED);
}

void VBox::RetrieveRecordings()
{
  std::vector<RecordingPtr> recordings;

  // Only attempt to retrieve recordings when external media is present
  if (m_externalMediaStatus.present)
  {
    request::Request request("GetRecordsList");
    request.AddParameter("Externals", "YES");
    response::ResponsePtr response = PerformRequest(request);
    response::RecordingResponseContent content(response->GetReplyElement());

    std::unique_lock<std::mutex> lock(m_mutex);
    m_recordings = std::move(content.GetRecordings());
  }

  m_stateHandler.EnterState(StartupState::RECORDINGS_LOADED);
}

response::ResponsePtr VBox::PerformRequest(const request::Request &request) const
{
  // Attempt to open a HTTP file handle
  void *fileHandle = XBMC->OpenFile(request.GetUrl().c_str(), READ_NO_CACHE);

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
