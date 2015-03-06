#pragma once
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
#include <vector>
#include <mutex>
#include <libXBMC_addon.h>
#include "Channel.h"
#include "Recording.h"
#include "Exceptions.h"
#include "Settings.h"
#include "request/Request.h"
#include "response/Response.h"
#include "util/StartupStateHandler.h"

namespace vbox {

  /**
   * Represents the status of any external media attached to the gateway
   */
  struct ExternalMediaStatus
  {
    bool present = false;
    int64_t spaceTotal = 0;
    int64_t spaceUsed = 0;
  };

  /**
   * The main class for interfacing with the VBox Gateway
   */
  class VBox
  {
  public:
    VBox(const Settings &settings);
    ~VBox();

    /**
     * Initializes the addon
     */
    void Initialize();
    const Settings& GetSettings() const;
    util::StartupStateHandler& GetStateHandler();
    std::string GetApiBaseUrl() const;

    // General API methods
    std::string GetBackendName() const;
    std::string GetBackendHostname() const;
    std::string GetBackendVersion() const;
    std::string GetConnectionString() const;
    
    // Channel methods
    int GetChannelsAmount() const;
    std::vector<Channel> GetChannels();

    // Recording methods
    bool SupportsRecordings() const;
    int64_t GetRecordingTotalSpace() const;
    int64_t GetRecordingUsedSpace() const;
    int GetRecordingsAmount() const;
    int GetTimersAmount() const;
    bool DeleteRecordingOrTimer(unsigned int id);
    void AddTimer(const std::string channelId, time_t startTime, time_t endTime);
    std::vector<Recording> GetRecordingsAndTimers();

    // Helpers
    static void Log(const ADDON::addon_log level, const char *format, ...);
    static void LogException(VBoxException &e);

  private:
    
    std::vector<Channel> RetrieveChannels();
    std::vector<Recording> RetrieveRecordings();
    response::ResponsePtr PerformRequest(const request::Request &request) const;

    /**
     * The addons settings
     */
    const Settings m_settings;

    /**
     * Name of the backend
     */
    std::string m_backendName;

    /**
     * The backend version
     */
    std::string m_backendVersion;

    /**
     * The list of channels
     */
    std::vector<Channel> m_channels;

    /**
     * The list of recordings, including timeres
     */
    std::vector<Recording> m_recordings;

    /**
     * The external media status
     */
    ExternalMediaStatus m_externalMediaStatus;

    /**
     * Handler for the startup state
     */
    util::StartupStateHandler m_stateHandler;

    /**
     * Mutex for protecting access to m_channels and m_recordings
     */
    mutable std::mutex m_mutex;
  };
}