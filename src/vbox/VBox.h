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
#include <thread>
#include <atomic>
#include <functional>
#include <libXBMC_addon.h>
#include "Channel.h"
#include "ChannelStreamingStatus.h"
#include "Recording.h"
#include "Exceptions.h"
#include "Settings.h"
#include "SoftwareVersion.h"
#include "request/Request.h"
#include "response/Response.h"
#include "StartupStateHandler.h"
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
    bool ValidateSettings() const;
    const Settings& GetSettings() const;
    StartupStateHandler& GetStateHandler();
    std::string GetApiBaseUrl() const;

    // General API methods
    std::string GetBackendName() const;
    std::string GetBackendHostname() const;
    std::string GetBackendVersion() const;
    std::string GetConnectionString() const;
    
    // Channel methods
    int GetChannelsAmount() const;
    const std::vector<ChannelPtr>& GetChannels() const;
    const Channel* GetChannel(unsigned int uniqueId) const;
    const Channel& GetCurrentChannel() const;
    void SetCurrentChannel(const Channel &channel);
    ChannelStreamingStatus GetChannelStreamingStatus(const Channel* channel) const;

    // Recording methods
    bool SupportsRecordings() const;
    int64_t GetRecordingTotalSpace() const;
    int64_t GetRecordingUsedSpace() const;
    int GetRecordingsAmount() const;
    int GetTimersAmount() const;
    bool DeleteRecordingOrTimer(unsigned int id);
    void AddTimer(const Channel *channel, const ::xmltv::Programme* programme);
    void AddTimer(const Channel *channel, time_t startTime, time_t endTime);
    const std::vector<RecordingPtr>& GetRecordingsAndTimers() const;

    // EPG methods
    const ::xmltv::Schedule* GetSchedule(const Channel *channel) const;
    const ::xmltv::Programme* GetProgramme(int programmeUniqueId) const;

    // Helpers
    static void Log(const ADDON::addon_log level, const char *format, ...);
    static void LogException(VBoxException &e);

    // Event handlers
    std::function<void()> OnChannelsUpdated;
    std::function<void()> OnRecordingsUpdated;
    std::function<void()> OnTimersUpdated;
    std::function<void()> OnGuideUpdated;

  private:
    
    void BackgroundUpdater();
    void RetrieveChannels(bool triggerEvent = true);
    void RetrieveRecordings(bool triggerEvent = true);
    void RetrieveGuide(bool triggerEvent = true);
    void RetrieveExternalGuide(bool triggerEvent = true);

    void LogGuideStatistics(const ::xmltv::Guide &guide) const;
    response::ResponsePtr PerformRequest(const request::IRequest &request) const;

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
    ::xmltv::Guide m_guide;

    /**
     * The external guide data
     */
    ::xmltv::Guide m_externalGuide;

    /**
     * The external media status
     */
    ExternalMediaStatus m_externalMediaStatus;

    /**
     * Handler for the startup state
     */
    StartupStateHandler m_stateHandler;

    /**
     * The background update thread
     */
    std::thread m_backgroundThread;

    /**
     * Controls whether the background update thread should keep running or not
     */
    std::atomic<bool> m_active;

    /**
     * The currently active channel, or the last active channel when no 
     * channel is playing
     */
    Channel m_currentChannel;

    /**
     * Mutex for protecting access to m_channels and m_recordings
     */
    mutable std::mutex m_mutex;
  };
}