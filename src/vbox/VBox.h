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
#include <thread>
#include <atomic>
#include <functional>
#include <libXBMC_addon.h>
#include "Channel.h"
#include "ChannelStreamingStatus.h"
#include "GuideChannelMapper.h"
#include "Recording.h"
#include "Exceptions.h"
#include "Settings.h"
#include "SoftwareVersion.h"
#include "request/Request.h"
#include "request/ApiRequest.h"
#include "response/Response.h"
#include "StartupStateHandler.h"
#include "../xmltv/Programme.h"
#include "../xmltv/Schedule.h"
#include "../xmltv/Guide.h"

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
   * Represents various pieces of information about the connected backend
   */
  struct BackendInformation
  {
    std::string name = "";
    std::string timezoneOffset = "";
    SoftwareVersion version;
    ExternalMediaStatus externalMediaStatus;
  };

  /**
   * Represents a schedule. It contains an actual schedule and an indicator 
   * which tells if the schedule is from the internal or external guide
   */
  struct Schedule
  {
    enum Origin
    {
      INTERNAL_GUIDE,
      EXTERNAL_GUIDE
    };

    ::xmltv::SchedulePtr schedule = nullptr;
    Origin origin = Origin::INTERNAL_GUIDE;
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
    void DetermineConnectionParams();
    bool ValidateSettings() const;
    const Settings& GetSettings() const;
    const ConnectionParameters& GetConnectionParams() const;
    StartupStateHandler& GetStateHandler();
    std::string GetApiBaseUrl() const;

    /**
     * Converts a UTC UNIX timestamp to an XMLTV timestamp localized for the 
     * backends timezone offset
     * @param unixTimestamp a UTC UNIX timestamp
     * @return XMLTV timestamp localized for the current backend
     */
    std::string CreateTimestamp(const time_t unixTimestamp) const;

    // General API methods
    std::string GetBackendName() const;
    std::string GetBackendHostname() const;
    std::string GetBackendVersion() const;
    std::string GetConnectionString() const;
    
    // Channel methods
    int GetChannelsAmount() const;
    const std::vector<ChannelPtr>& GetChannels() const;
    const ChannelPtr GetChannel(unsigned int uniqueId) const;
    const ChannelPtr GetCurrentChannel() const;
    void SetCurrentChannel(const ChannelPtr &channel);
    ChannelStreamingStatus GetChannelStreamingStatus(const ChannelPtr &channel) const;

    // Recording methods
    bool SupportsRecordings() const;
    int64_t GetRecordingTotalSpace() const;
    int64_t GetRecordingUsedSpace() const;
    int GetRecordingsAmount() const;
    int GetTimersAmount() const;
    request::ApiRequest CreateDeleteRecordingRequest(const RecordingPtr &recording) const;
    bool DeleteRecordingOrTimer(unsigned int id);
    void AddTimer(const ChannelPtr &channel, const ::xmltv::ProgrammePtr programme);
    void AddTimer(const ChannelPtr &channel, time_t startTime, time_t endTime,
      const std::string title, const std::string description);
    void AddTimer(const ChannelPtr &channel, time_t startTime, time_t endTime);
    const std::vector<RecordingPtr>& GetRecordingsAndTimers() const;

    // EPG methods
    const Schedule GetSchedule(const ChannelPtr &channel) const;

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
    void InitializeChannelMapper();
    void SwapChannelIcons(std::vector<ChannelPtr> &channels);

    void LogGuideStatistics(const ::xmltv::Guide &guide) const;
    response::ResponsePtr PerformRequest(const request::Request &request) const;

    /**
     * The addons settings
     */
    const Settings m_settings;

    /**
     * The connection parameters to use for requests
     */
    ConnectionParameters m_currentConnectionParameters;

    /**
     * The backend information
     */
    BackendInformation m_backendInformation;

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
     * The guide channel mapper
     */
    GuideChannelMapperPtr m_guideChannelMapper;

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
    ChannelPtr m_currentChannel;

    /**
     * Mutex for protecting access to m_channels and m_recordings
     */
    mutable std::mutex m_mutex;
  };
}