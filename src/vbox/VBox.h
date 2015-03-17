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
#include <map>
#include <mutex>
#include <libXBMC_addon.h>
#include "Channel.h"
#include "Recording.h"
#include "Exceptions.h"
#include "Settings.h"
#include "SoftwareVersion.h"
#include "request/Request.h"
#include "response/Response.h"
#include "util/StartupStateHandler.h"
#include "xmltv/Programme.h"
#include "xmltv/Schedule.h"
#include "xmltv/Guide.h"

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

    /**
     * The minimum backend software version required to use the addon
     */
    static const char * MINIMUM_SOFTWARE_VERSION;

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
    const std::vector<ChannelPtr>& GetChannels() const;
    const ChannelPtr& GetChannel(unsigned int uniqueId) const;

    // Recording methods
    bool SupportsRecordings() const;
    int64_t GetRecordingTotalSpace() const;
    int64_t GetRecordingUsedSpace() const;
    int GetRecordingsAmount() const;
    int GetTimersAmount() const;
    bool DeleteRecordingOrTimer(unsigned int id);
    void AddTimer(const ChannelPtr &channel, time_t startTime, time_t endTime);
    const std::vector<RecordingPtr>& GetRecordingsAndTimers() const;

    // EPG methods
    bool HasSchedule(const ChannelPtr &channel) const;
    const xmltv::SchedulePtr& GetSchedule(const ChannelPtr &channel);

    // Helpers
    static void Log(const ADDON::addon_log level, const char *format, ...);
    static void LogException(VBoxException &e);

  private:
    
    void RetrieveChannels();
    void RetrieveRecordings();
    void RetrieveGuide();
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
    SoftwareVersion m_backendVersion;

    /**
     * The list of channels
     */
    std::vector<ChannelPtr> m_channels;

    /**
     * The list of recordings, including timeres
     */
    std::vector<RecordingPtr> m_recordings;

    /**
     * The guide data. The XMLTV channel name is the key, the value is the 
     * schedule for the channel
     */
    xmltv::Guide m_guide;

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