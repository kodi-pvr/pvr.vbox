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

#include <algorithm>
#include "p8-platform/util/util.h"
#include "xbmc_pvr_dll.h"
#include "client.h"
#include "compat.h"
#include "vbox/Exceptions.h"
#include "vbox/VBox.h"
#include "vbox/ContentIdentifier.h"
#include "timeshift/DummyBuffer.h"
#include "timeshift/FilesystemBuffer.h"
#include "xmltv/Utilities.h"

using namespace ADDON;
using namespace vbox;

// Initialize helpers
CHelper_libXBMC_addon *XBMC = NULL;
CHelper_libXBMC_pvr   *PVR = NULL;
CHelper_libKODI_guilib  *GUI = NULL;

// Initialize globals
ADDON_STATUS   g_status = ADDON_STATUS_UNKNOWN;
VBox *g_vbox = nullptr;
timeshift::Buffer *g_timeshiftBuffer = nullptr;

std::string g_internalHostname;
std::string g_externalHostname;
int g_internalHttpPort;
int g_externalHttpPort;
int g_internalHttpsPort;
int g_externalHttpsPort;
int g_internalUpnpPort;
int g_externalUpnpPort;
int g_internalConnectionTimeout;
int g_externalConnectionTimeout;

bool g_useExternalXmltv;
std::string g_externalXmltvPath;
bool g_preferExternalXmltv;
bool g_useExternalXmltvIcons;
ChannelOrder g_setChannelIdUsingOrder;
unsigned int g_remindMinsBeforeProg;
bool g_timeshiftEnabled;
std::string g_timeshiftBufferPath;

// settings context menu
unsigned int MENUHOOK_ID_RESCAN_EPG = 1;
unsigned int MENUHOOK_ID_SYNC_EPG = 2;
// EPG context menu
unsigned int MENUHOOK_ID_EPG_REMINDER = 1;
unsigned int MENUHOOK_ID_CANCEL_EPG_REMINDER = 2;
// channels context menu
unsigned int MENUHOOK_ID_MANUAL_REMINDER = 1;
unsigned int MENUHOOK_ID_CANCEL_CHANNEL_REMINDER = 2;

extern "C" {

  void ADDON_ReadSettings()
  {
#define UPDATE_INT(var, key, def)\
  if (!XBMC->GetSetting(key, &var))\
    var = def;

#define UPDATE_STR(var, key, tmp, def)\
  if (XBMC->GetSetting(key, tmp))\
    var = tmp;\
    else\
    var = def;

    char buffer[1024];

    UPDATE_STR(g_internalHostname, "hostname", buffer, "");
    UPDATE_INT(g_internalHttpPort, "http_port", 80);
    UPDATE_INT(g_internalHttpsPort, "https_port", 0);
    UPDATE_INT(g_internalUpnpPort, "upnp_port", 55555);
    UPDATE_STR(g_externalHostname, "external_hostname", buffer, "");
    UPDATE_INT(g_externalHttpPort, "external_http_port", 19999);
    UPDATE_INT(g_externalHttpsPort, "external_https_port", 0);
    UPDATE_INT(g_externalUpnpPort, "external_upnp_port", 55555);
    UPDATE_INT(g_internalConnectionTimeout, "connection_timeout", 3);
    UPDATE_INT(g_externalConnectionTimeout, "external_connection_timeout", 10);
    UPDATE_INT(g_useExternalXmltv, "use_external_xmltv", false);
    UPDATE_STR(g_externalXmltvPath, "external_xmltv_path", buffer, "");
    UPDATE_INT(g_preferExternalXmltv, "prefer_external_xmltv", false);
    UPDATE_INT(g_useExternalXmltvIcons, "use_external_xmltv_icons", false);
    UPDATE_INT(g_setChannelIdUsingOrder, "set_channelid_using_order", CH_ORDER_BY_LCN);
    UPDATE_INT(g_remindMinsBeforeProg, "reminder_mins_before_prog", 0);
    UPDATE_INT(g_timeshiftEnabled, "timeshift_enabled", false);
    UPDATE_STR(g_timeshiftBufferPath, "timeshift_path", buffer, "");

#undef UPDATE_INT
#undef UPDATE_STR
  }

  ADDON_STATUS ADDON_Create(void* hdl, void* props)
  {
    if (!hdl || !props)
      return ADDON_STATUS_UNKNOWN;

    // Instantiate helpers
    XBMC = new CHelper_libXBMC_addon;
    PVR = new CHelper_libXBMC_pvr;
    GUI = new CHelper_libKODI_guilib;

    if (!XBMC->RegisterMe(hdl) ||
      !PVR->RegisterMe(hdl) || 
      !GUI->RegisterMe(hdl) ) 
    {
      SAFE_DELETE(XBMC);
      SAFE_DELETE(PVR);
      SAFE_DELETE(GUI);
      return ADDON_STATUS_PERMANENT_FAILURE;
    }

    // Read the settings. The addon is restarted whenever a setting is changed 
    // so we only need to read them here.
    ADDON_ReadSettings();
    Settings settings;

    settings.m_internalConnectionParams =
    {
      g_internalHostname,
      g_internalHttpPort,
      g_internalHttpsPort,
      g_internalUpnpPort,
      g_internalConnectionTimeout
    };

    settings.m_externalConnectionParams =
    {
      g_externalHostname,
      g_externalHttpPort,
      g_externalHttpsPort,
      g_externalUpnpPort,
      g_externalConnectionTimeout
    };

    settings.m_useExternalXmltv = g_useExternalXmltv;
    settings.m_externalXmltvPath = g_externalXmltvPath;
    settings.m_preferExternalXmltv = g_preferExternalXmltv;
    settings.m_useExternalXmltvIcons = g_useExternalXmltvIcons;
    settings.m_setChannelIdUsingOrder = g_setChannelIdUsingOrder;
    settings.m_remindMinsBeforeProg = g_remindMinsBeforeProg;
    settings.m_timeshiftEnabled = g_timeshiftEnabled;
    settings.m_timeshiftBufferPath = g_timeshiftBufferPath;

    // Create the addon
    VBox::Log(LOG_DEBUG, "creating VBox Gateway PVR addon");
    g_status = ADDON_STATUS_UNKNOWN;
    g_vbox = new VBox(settings);

    // Validate settings
    if (g_vbox->ValidateSettings())
    {
      // Start the addon
      try {
        g_vbox->Initialize();
        g_status = ADDON_STATUS_OK;

        // Attach event handlers
        g_vbox->OnChannelsUpdated = []() { PVR->TriggerChannelUpdate(); };
        g_vbox->OnRecordingsUpdated = []() { PVR->TriggerRecordingUpdate(); };
        g_vbox->OnTimersUpdated = []() { PVR->TriggerTimerUpdate(); };
        g_vbox->OnGuideUpdated = []()
        {
          for (const auto &channel : g_vbox->GetChannels())
            PVR->TriggerEpgUpdate(ContentIdentifier::GetUniqueId(channel));
        };

        // Create the timeshift buffer
        if (settings.m_timeshiftEnabled)
          g_timeshiftBuffer = new timeshift::FilesystemBuffer(settings.m_timeshiftBufferPath);
        else
          g_timeshiftBuffer = new timeshift::DummyBuffer();

        g_timeshiftBuffer->SetReadTimeout(g_vbox->GetConnectionParams().timeout);

        // initializing TV Settings Client Specific menu hooks
        PVR_MENUHOOK hooks[] = { { MENUHOOK_ID_RESCAN_EPG, 30106, PVR_MENUHOOK_SETTING },
                                 { MENUHOOK_ID_SYNC_EPG, 30107, PVR_MENUHOOK_SETTING },
                                 { MENUHOOK_ID_EPG_REMINDER, 30110, PVR_MENUHOOK_EPG },
                                 { MENUHOOK_ID_CANCEL_EPG_REMINDER, 30112, PVR_MENUHOOK_EPG },
                                 { MENUHOOK_ID_MANUAL_REMINDER, 30111, PVR_MENUHOOK_CHANNEL },
                                 { MENUHOOK_ID_CANCEL_CHANNEL_REMINDER, 30113, PVR_MENUHOOK_CHANNEL } };

        for (int i = 0; i < sizeof(hooks) / sizeof(PVR_MENUHOOK); ++i)
          PVR->AddMenuHook(&hooks[i]);
      }
      catch (FirmwareVersionException &e) {
        XBMC->QueueNotification(ADDON::QUEUE_ERROR, e.what());
        g_status = ADDON_STATUS_PERMANENT_FAILURE;
      }
      catch (VBoxException &e) {
        VBox::LogException(e);
        g_status = ADDON_STATUS_LOST_CONNECTION;
      }
    }
    else
      g_status = ADDON_STATUS_NEED_SETTINGS;

    return g_status;
  }

  ADDON_STATUS ADDON_GetStatus()
  {
    return g_status;
  }

  void ADDON_Destroy()
  {
    SAFE_DELETE(g_vbox);
    SAFE_DELETE(g_timeshiftBuffer);
    g_status = ADDON_STATUS_UNKNOWN;
  } 

  ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
  {
#define UPDATE_STR(key, var)\
  if (!strcmp(settingName, key))\
  {\
    if (strcmp(var.c_str(), (const char*)settingValue) != 0)\
    {\
      VBox::Log(LOG_INFO, "updated setting %s from '%s' to '%s'",\
        settingName, var.c_str(), settingValue);\
      return ADDON_STATUS_NEED_RESTART;\
    }\
    return ADDON_STATUS_OK;\
  }

#define UPDATE_INT(key, type, var)\
  if (!strcmp(settingName, key))\
  {\
    if (var != *(type*)settingValue)\
    {\
      VBox::Log(LOG_INFO, "updated setting %s from '%d' to '%d'",\
        settingName, var, (int)*(type*)settingValue);\
      return ADDON_STATUS_NEED_RESTART;\
    }\
    return ADDON_STATUS_OK;\
  }

    const vbox::Settings &settings = g_vbox->GetSettings();

    UPDATE_STR("hostname", settings.m_internalConnectionParams.hostname);
    UPDATE_INT("http_port", int, settings.m_internalConnectionParams.httpPort);
    UPDATE_INT("https_port", int, settings.m_internalConnectionParams.httpsPort);
    UPDATE_INT("upnp_port", int, settings.m_internalConnectionParams.upnpPort);
    UPDATE_INT("connection_timeout", int, settings.m_internalConnectionParams.timeout);
    UPDATE_STR("external_hostname", settings.m_externalConnectionParams.hostname);
    UPDATE_INT("external_http_port", int, settings.m_externalConnectionParams.httpPort);
    UPDATE_INT("external_https_port", int, settings.m_externalConnectionParams.httpsPort);
    UPDATE_INT("external_upnp_port", int, settings.m_externalConnectionParams.upnpPort);
    UPDATE_INT("external_connection_timeout", int, settings.m_externalConnectionParams.timeout);
    UPDATE_INT("use_external_xmltv", bool, settings.m_useExternalXmltv);
    UPDATE_STR("external_xmltv_path", settings.m_externalXmltvPath);
    UPDATE_INT("prefer_external_xmltv", bool, settings.m_preferExternalXmltv);
    UPDATE_INT("use_external_xmltv_icons", bool, settings.m_useExternalXmltvIcons);
    UPDATE_INT("set_channelid_using_order", ChannelOrder, settings.m_setChannelIdUsingOrder);
    UPDATE_INT("reminder_mins_before_prog", unsigned int, settings.m_remindMinsBeforeProg)
    UPDATE_INT("timeshift_enabled", bool, settings.m_timeshiftEnabled);
    UPDATE_STR("timeshift_path", settings.m_timeshiftBufferPath);

    return ADDON_STATUS_OK;
#undef UPDATE_INT
#undef UPDATE_STR
  }

  void OnSystemSleep()
  {
  }

  void OnSystemWake()
  {
  }

  void OnPowerSavingActivated()
  {
  }

  void OnPowerSavingDeactivated()
  {
  }

  PVR_ERROR GetAddonCapabilities(PVR_ADDON_CAPABILITIES* pCapabilities)
  {
    pCapabilities->bSupportsTV = true;
    pCapabilities->bSupportsRadio = true;
    pCapabilities->bSupportsChannelGroups = false;
    pCapabilities->bSupportsEPG = true;
    pCapabilities->bHandlesInputStream = true;

    // Recording capability is determined further down, we'll assume false 
    // in case the real capabilities cannot be determined for some reason
    pCapabilities->bSupportsRecordings = false;
    pCapabilities->bSupportsTimers = false;

    // Unsupported features
    pCapabilities->bSupportsRecordingsUndelete = false;
    pCapabilities->bSupportsChannelScan = false;
    pCapabilities->bSupportsChannelSettings = false;
    pCapabilities->bHandlesDemuxing = false;
    pCapabilities->bSupportsRecordingPlayCount = false;
    pCapabilities->bSupportsLastPlayedPosition = false;
    pCapabilities->bSupportsRecordingEdl = false;

    // Wait for initialization until we decide if we support recordings or not.
    // Recording is only possible when external media is present
    if (g_vbox->GetStateHandler().WaitForState(StartupState::INITIALIZED)
      && g_vbox->SupportsRecordings())
    {
      pCapabilities->bSupportsRecordings = true;
      pCapabilities->bSupportsTimers = true;
    }

    pCapabilities->bSupportsRecordingsRename = false;
    pCapabilities->bSupportsRecordingsLifetimeChange = false;
    pCapabilities->bSupportsDescrambleInfo = false;

    return PVR_ERROR_NO_ERROR;
  }

  const char *GetBackendName(void)
  {
    static std::string backendName = g_vbox->GetBackendName();
    return backendName.c_str();
  }

  const char *GetBackendVersion(void)
  {
    static std::string backendVersion = g_vbox->GetBackendVersion();
    return backendVersion.c_str();
  }

  const char *GetConnectionString(void)
  {
    static std::string connectionString = g_vbox->GetConnectionString();
    return connectionString.c_str();
  }

  const char *GetBackendHostname(void)
  {
    static std::string backendHostname = g_vbox->GetBackendHostname();
    return backendHostname.c_str();
  }

  int GetChannelsAmount(void)
  {
    try {
      return g_vbox->GetChannelsAmount();
    }
    catch (VBoxException &e)
    {
      g_vbox->LogException(e);
      return PVR_ERROR_SERVER_ERROR;
    }
  }

  PVR_ERROR GetChannels(ADDON_HANDLE handle, bool bRadio)
  {
    auto &channels = g_vbox->GetChannels();
    unsigned int i = 0;

    for (const auto &item : channels)
    {
      // Skip those that are not of the correct type
      if (item->m_radio != bRadio)
        continue;

      PVR_CHANNEL channel;
      memset(&channel, 0, sizeof(PVR_CHANNEL));

      channel.iUniqueId = ContentIdentifier::GetUniqueId(item);
      channel.bIsRadio = item->m_radio;

      // Override LCN if backend channel order should be forced
      ++i;
      if (g_vbox->GetSettings().m_setChannelIdUsingOrder == CH_ORDER_BY_INDEX)
        channel.iChannelNumber = i;
      // default - CH_ORDER_BY_LCN
      else
        channel.iChannelNumber = item->m_number;

      channel.iEncryptionSystem = item->m_encrypted ? 0xFFFF : 0x0000;

      strncpy(channel.strChannelName, item->m_name.c_str(),
        sizeof(channel.strChannelName));
      strncpy(channel.strIconPath, item->m_iconUrl.c_str(),
        sizeof(channel.strIconPath));

      // Set stream format for TV channels
      if (!item->m_radio)
      {
        strncpy(channel.strInputFormat, "video/mp2t",
          sizeof(channel.strInputFormat));
      }
      else
      {
        // TODO: Kodi can't reliably play radio channels using ReadLiveStream()
        strncpy(channel.strStreamURL, item->m_url.c_str(),
          sizeof(channel.strStreamURL));
      }
      VBox::Log(LOG_INFO, "Adding channel %d: %s. Icon: %s",
                channel.iChannelNumber, channel.strChannelName, channel.strIconPath);

      PVR->TransferChannelEntry(handle, &channel);
    }

    return PVR_ERROR_NO_ERROR;
  }

  PVR_ERROR GetDriveSpace(long long *iTotal, long long *iUsed)
  {
    *iTotal = g_vbox->GetRecordingTotalSpace() / 1024;
    *iUsed = g_vbox->GetRecordingUsedSpace() / 1024;

    return PVR_ERROR_NO_ERROR;
  }

  int GetRecordingsAmount(bool deleted)
  {
    return g_vbox->GetRecordingsAmount();
  }

  PVR_ERROR GetRecordings(ADDON_HANDLE handle, bool deleted)
  {
    auto &recordings = g_vbox->GetRecordingsAndTimers();

    for (const auto &item : recordings)
    {
      // Skip timers
      if (!item->IsRecording())
        continue;

      PVR_RECORDING recording;
      memset(&recording, 0, sizeof(PVR_RECORDING));

      time_t startTime = xmltv::Utilities::XmltvToUnixTime(item->m_startTime);
      time_t endTime = xmltv::Utilities::XmltvToUnixTime(item->m_endTime);
      unsigned int id = item->m_id;

      recording.recordingTime = startTime;
      recording.iDuration = static_cast<int>(endTime - startTime);
      recording.iEpgEventId = id;

      strncpy(recording.strChannelName, item->m_channelName.c_str(),
        sizeof(recording.strChannelName));

      strncpy(recording.strRecordingId, compat::to_string(id).c_str(),
        sizeof(recording.strRecordingId));

      strncpy(recording.strStreamURL, item->m_url.c_str(),
        sizeof(recording.strStreamURL));

      strncpy(recording.strTitle, item->m_title.c_str(),
        sizeof(recording.strTitle));

      strncpy(recording.strPlot, item->m_description.c_str(),
        sizeof(recording.strPlot));

      /* TODO: PVR API 5.0.0: Implement this */
      recording.iChannelUid = PVR_CHANNEL_INVALID_UID;

      /* TODO: PVR API 5.1.0: Implement this */
      recording.channelType = PVR_RECORDING_CHANNEL_TYPE_UNKNOWN;

      PVR->TransferRecordingEntry(handle, &recording);
    }

    return PVR_ERROR_NO_ERROR;
  }

  PVR_ERROR DeleteRecording(const PVR_RECORDING &recording)
  {
    try {
      unsigned int id = compat::stoui(recording.strRecordingId);

      if (g_vbox->DeleteRecordingOrTimer(id))
        return PVR_ERROR_NO_ERROR;
      else
        return PVR_ERROR_FAILED;
    }
    catch (...)
    {
      return PVR_ERROR_INVALID_PARAMETERS;
    }
  }

  PVR_ERROR GetTimerTypes(PVR_TIMER_TYPE types[], int *size)
  {
    int numOfTimerTypes = 0;
    
    memset(&types[numOfTimerTypes], 0, sizeof(types[numOfTimerTypes]));
    // EPG based single recording
    types[numOfTimerTypes].iId = vbox::TIMER_VBOX_TYPE_EPG_BASED_SINGLE;
    strcpy(types[numOfTimerTypes].strDescription, "EPG-based one time recording");
    types[numOfTimerTypes].iAttributes = 
      PVR_TIMER_TYPE_REQUIRES_EPG_TAG_ON_CREATE |
      PVR_TIMER_TYPE_SUPPORTS_START_TIME |
      PVR_TIMER_TYPE_SUPPORTS_END_TIME;
    ++numOfTimerTypes;
    // episode recording
    memset(&types[numOfTimerTypes], 0, sizeof(types[numOfTimerTypes]));
    types[numOfTimerTypes].iId = TIMER_VBOX_TYPE_EPISODE;
    strcpy(types[numOfTimerTypes].strDescription, "Episode recording");
    types[numOfTimerTypes].iAttributes = 
      PVR_TIMER_TYPE_IS_READONLY |
      PVR_TIMER_TYPE_SUPPORTS_START_TIME |
      PVR_TIMER_TYPE_SUPPORTS_END_TIME;
    ++numOfTimerTypes;
    // manual recording
    memset(&types[numOfTimerTypes], 0, sizeof(types[numOfTimerTypes]));
    types[numOfTimerTypes].iId = TIMER_VBOX_TYPE_MANUAL_SINGLE;
    strcpy(types[numOfTimerTypes].strDescription, "Manual one time recording");
    types[numOfTimerTypes].iAttributes = 
      PVR_TIMER_TYPE_IS_MANUAL |
      PVR_TIMER_TYPE_FORBIDS_EPG_TAG_ON_CREATE |
      PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
      PVR_TIMER_TYPE_SUPPORTS_START_TIME | 
      PVR_TIMER_TYPE_SUPPORTS_END_TIME;
    ++numOfTimerTypes;

    //Automatic series recording
    memset(&types[numOfTimerTypes], 0, sizeof(types[numOfTimerTypes]));
    types[numOfTimerTypes].iId = TIMER_VBOX_TYPE_EPG_BASED_AUTO_SERIES;
    strcpy(types[numOfTimerTypes].strDescription, "EPG-based automatic series recording");
	types[numOfTimerTypes].iAttributes =
		PVR_TIMER_TYPE_REQUIRES_EPG_SERIES_ON_CREATE;
    ++numOfTimerTypes;

    // EPG based series recording
    memset(&types[numOfTimerTypes], 0, sizeof(types[numOfTimerTypes]));
    types[numOfTimerTypes].iId = TIMER_VBOX_TYPE_EPG_BASED_MANUAL_SERIES;
    strcpy(types[numOfTimerTypes].strDescription, "EPG-based manual series recording");
    types[numOfTimerTypes].iAttributes =
      PVR_TIMER_TYPE_IS_REPEATING |
      PVR_TIMER_TYPE_REQUIRES_EPG_TAG_ON_CREATE |
      PVR_TIMER_TYPE_SUPPORTS_START_TIME |
      PVR_TIMER_TYPE_SUPPORTS_END_TIME |
      PVR_TIMER_TYPE_SUPPORTS_WEEKDAYS;
    ++numOfTimerTypes;

    //Manual series recording
    memset(&types[numOfTimerTypes], 0, sizeof(types[numOfTimerTypes]));
    types[numOfTimerTypes].iId = TIMER_VBOX_TYPE_MANUAL_SERIES;
    strcpy(types[numOfTimerTypes].strDescription, "Manual series recording");
    types[numOfTimerTypes].iAttributes =
      PVR_TIMER_TYPE_IS_MANUAL |
      PVR_TIMER_TYPE_IS_REPEATING |
      PVR_TIMER_TYPE_FORBIDS_EPG_TAG_ON_CREATE |
      PVR_TIMER_TYPE_SUPPORTS_CHANNELS |
      PVR_TIMER_TYPE_SUPPORTS_START_TIME |
      PVR_TIMER_TYPE_SUPPORTS_END_TIME |
      PVR_TIMER_TYPE_SUPPORTS_WEEKDAYS;
    ++numOfTimerTypes;

    *size = numOfTimerTypes;
    return PVR_ERROR_NO_ERROR;
  }

  int GetTimersAmount(void)
  {
    return g_vbox->GetTimersAmount();
  }

  PVR_ERROR GetTimers(ADDON_HANDLE handle)
  {
    /* TODO: Change implementation to get support for the timer features introduced with PVR API 1.9.7 */
    auto &recordings = g_vbox->GetRecordingsAndTimers();

    // first get timers from single recordings (scheduled)
    for (const auto &item : recordings)
    {
      // Skip recordings
      if (!item->IsTimer())
        continue;

      PVR_TIMER timer;
      memset(&timer, 0, sizeof(PVR_TIMER));
      timer.iTimerType = (item->m_seriesId > 0)? vbox::TIMER_VBOX_TYPE_EPISODE : vbox::TIMER_VBOX_TYPE_MANUAL_SINGLE;
      timer.startTime = xmltv::Utilities::XmltvToUnixTime(item->m_startTime);
      timer.endTime = xmltv::Utilities::XmltvToUnixTime(item->m_endTime);
      timer.iClientIndex = item->m_id;

      // Convert the internal timer state to PVR_TIMER_STATE
      switch (item->GetState())
      {
      case RecordingState::SCHEDULED:
        timer.state = PVR_TIMER_STATE_SCHEDULED;
        break;
      case RecordingState::RECORDED:
      case RecordingState::EXTERNAL:
        timer.state = PVR_TIMER_STATE_COMPLETED;
        break;
      case RecordingState::RECORDING:
        timer.state = PVR_TIMER_STATE_RECORDING;
        break;
      }

      // Find the timer's channel and use its unique ID
      auto &channels = g_vbox->GetChannels();
      auto it = std::find_if(channels.cbegin(), channels.cend(),
        [&item](const ChannelPtr &channel)
      {
        return channel->m_xmltvName == item->m_channelId;
      });

      if (it != channels.cend())
        timer.iClientChannelUid = ContentIdentifier::GetUniqueId(*it);
      else
        continue;

      strncpy(timer.strTitle, item->m_title.c_str(),
        sizeof(timer.strTitle));

      strncpy(timer.strSummary, item->m_description.c_str(),
        sizeof(timer.strSummary));

      g_vbox->Log(LOG_DEBUG, "GetTimers(): adding timer to show %s", item->m_title.c_str());
      // TODO: Set margins to whatever the API reports
      PVR->TransferTimerEntry(handle, &timer);
    }
    // second: get timer rules for series
    auto &series = g_vbox->GetSeriesTimers();
    for (const auto &item : series)
    {
      PVR_TIMER timer;
      memset(&timer, 0, sizeof(PVR_TIMER));

      timer.iTimerType = (item->m_fIsAuto)? vbox::TIMER_VBOX_TYPE_EPG_BASED_AUTO_SERIES : vbox::TIMER_VBOX_TYPE_MANUAL_SERIES;
      timer.iClientIndex = item->m_id;
      timer.state = PVR_TIMER_STATE_SCHEDULED;

      // Find the timer's channel and use its unique ID
      auto &channels = g_vbox->GetChannels();
      auto it = std::find_if(channels.cbegin(), channels.cend(),
        [&item](const ChannelPtr &channel)
      {
        return channel->m_xmltvName == item->m_channelId;
      });

      if (it != channels.cend())
        timer.iClientChannelUid = ContentIdentifier::GetUniqueId(*it);

      unsigned int nextScheduledId = item->m_scheduledId;
      // Find next recording of the series
      auto recIt = std::find_if(recordings.begin(), recordings.end(),
        [nextScheduledId](const RecordingPtr &recording)
      {
        return nextScheduledId == recording->m_id;
      });
      // if it doesn't exist (canceled) - don't add series
      if (recIt == recordings.end())
        continue;
    
      timer.startTime = xmltv::Utilities::XmltvToUnixTime(item->m_startTime);
      // automatic starts & stops whenever detected (will appear as episode)		
      if (item->m_fIsAuto)
      {
        timer.bStartAnyTime = true;
        timer.bEndAnyTime = true;
      }
      else
      {
        // set periodic times
        timer.firstDay = xmltv::Utilities::XmltvToUnixTime(item->m_startTime);
        timer.iWeekdays = item->m_weekdays;
        timer.endTime = xmltv::Utilities::XmltvToUnixTime(item->m_endTime);
      }
      strncpy(timer.strTitle, item->m_title.c_str(),
        sizeof(timer.strTitle));

      strncpy(timer.strSummary, item->m_description.c_str(),
        sizeof(timer.strSummary));
    
      // TODO: Set margins to whatever the API reports
      PVR->TransferTimerEntry(handle, &timer);
    }
    return PVR_ERROR_NO_ERROR;
  }

  PVR_ERROR AddTimer(const PVR_TIMER &timer)
  {
     VBox::Log(LOG_DEBUG, "AddTimer() : entering with timer type 0x%x", timer.iTimerType);
    // Find the channel the timer is for
    auto &channels = g_vbox->GetChannels();
    auto it = std::find_if(channels.cbegin(), channels.cend(),
      [&timer](const ChannelPtr &channel)
    {
      return timer.iClientChannelUid == ContentIdentifier::GetUniqueId(channel);
    });

    if (it == channels.end())
      return PVR_ERROR_INVALID_PARAMETERS;

    const ChannelPtr channel = *it;

    // Find the channel's schedule
    const Schedule schedule = g_vbox->GetSchedule(channel);

    try {
      // update the recording margins in the backend
      g_vbox->UpdateRecordingMargins( {timer.iMarginStart, timer.iMarginEnd} );
      // Set start time to now if it's missing
      time_t startTime = timer.startTime;
      time_t endTime = timer.endTime;
      std::string title(timer.strTitle);
      std::string desc(timer.strSummary);

      if (startTime == 0)
        startTime = time(nullptr);

      // Add a programme-based timer if the programme exists in the schedule
      const xmltv::ProgrammePtr programme = (schedule.schedule)? schedule.schedule->GetProgramme(timer.iEpgUid) : nullptr;
    
      switch (timer.iTimerType)
      {
      case TIMER_VBOX_TYPE_EPG_BASED_SINGLE:
      case TIMER_VBOX_TYPE_EPISODE:
        if (programme)
        {
          switch (schedule.origin)
          {
          case Schedule::Origin::INTERNAL_GUIDE:
            g_vbox->AddTimer(channel, programme);
            break;
          case Schedule::Origin::EXTERNAL_GUIDE:
            title = programme->m_title;
            desc = programme->m_description;
            g_vbox->AddTimer(channel, startTime, endTime, title, desc);
            break;
          }
          return PVR_ERROR_NO_ERROR;
        }
        else
        {
          g_vbox->AddTimer(channel, startTime, endTime, title, desc);
          return PVR_ERROR_NO_ERROR;
        }
      case TIMER_VBOX_TYPE_MANUAL_SINGLE:
        g_vbox->AddTimer(channel, startTime, endTime, title, desc);
        return PVR_ERROR_NO_ERROR;
      case TIMER_VBOX_TYPE_EPG_BASED_AUTO_SERIES:
      {
        if (!programme)
          return PVR_ERROR_INVALID_PARAMETERS;
        g_vbox->AddSeriesTimer(channel, programme);
        return PVR_ERROR_NO_ERROR;
      }
      case TIMER_VBOX_TYPE_EPG_BASED_MANUAL_SERIES:
      {
        if (!programme)
            return PVR_ERROR_INVALID_PARAMETERS;
        g_vbox->AddTimer(channel, startTime, endTime, title, desc, timer.iWeekdays);
        return PVR_ERROR_NO_ERROR;
      }
      case TIMER_VBOX_TYPE_MANUAL_SERIES:
      {
        g_vbox->AddTimer(channel, startTime, endTime, title, desc, timer.iWeekdays);
        return PVR_ERROR_NO_ERROR;
      }
      default:
        // any other timer type is wrong
        return PVR_ERROR_INVALID_PARAMETERS;
      }
    }
    catch (VBoxException &e)
    {
      g_vbox->LogException(e);
      return PVR_ERROR_FAILED;
    }
  }

  PVR_ERROR DeleteTimer(const PVR_TIMER &timer, bool bForceDelete)
  {
    if (g_vbox->DeleteRecordingOrTimer(timer.iClientIndex))
      return PVR_ERROR_NO_ERROR;

    return PVR_ERROR_FAILED;
  }

  PVR_ERROR UpdateTimer(const PVR_TIMER &timer) 
  {
    PVR_ERROR err = DeleteTimer(timer, true);

    if (err == PVR_ERROR_NO_ERROR)
      return AddTimer(timer);
    return err;
  }

  PVR_ERROR GetEPGForChannel(ADDON_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd)
  {
    const ChannelPtr channelPtr = g_vbox->GetChannel(channel.iUniqueId);

    if (!channelPtr)
      return PVR_ERROR_INVALID_PARAMETERS;

    // Retrieve the schedule
    const auto schedule = g_vbox->GetSchedule(channelPtr);

    if (!schedule.schedule)
      return PVR_ERROR_NO_ERROR;
      
    // Transfer the programmes between the start and end times
    for (const auto &programme : schedule.schedule->GetSegment(iStart, iEnd))
    {
      EPG_TAG event;
      memset(&event, 0, sizeof(EPG_TAG));

      event.startTime = xmltv::Utilities::XmltvToUnixTime(programme->m_startTime);
      event.endTime = xmltv::Utilities::XmltvToUnixTime(programme->m_endTime);
      event.iChannelNumber = channel.iChannelNumber;
      event.iUniqueBroadcastId = ContentIdentifier::GetUniqueId(programme.get());
      event.strTitle = programme->m_title.c_str();
      event.strPlot = programme->m_description.c_str();
      event.iYear = programme->m_year;
      event.strEpisodeName = programme->m_subTitle.c_str();
      event.strIconPath = programme->m_icon.c_str();

      std::string directors = xmltv::Utilities::ConcatenateStringList(programme->GetDirectors());
      std::string writers = xmltv::Utilities::ConcatenateStringList(programme->GetWriters());
      std::vector<std::string> categories = programme->GetCategories();
      std::string catStrings = xmltv::Utilities::ConcatenateStringList(categories);

      event.strDirector = directors.c_str();
      event.strWriter = writers.c_str();

      // use genre mapper to find the most common genre type (from categories)
      event.iGenreType = g_vbox->GetCategoriesGenreType(categories);
      event.strGenreDescription = catStrings.c_str();

      // Extract up to five cast members only
      std::vector<std::string> actorNames;
      const auto &actors = programme->GetActors();
      int numActors = std::min(static_cast<int>(actors.size()), 5);

      for (unsigned int i = 0; i < numActors; i++)
        actorNames.push_back(actors.at(i).name);

      std::string cast = xmltv::Utilities::ConcatenateStringList(actorNames);
      event.strCast = cast.c_str();

      event.iFlags = EPG_TAG_FLAG_UNDEFINED;
      
      if (!programme->m_seriesIds.empty())
      {
        VBox::Log(LOG_DEBUG, "GetEPGForChannel():programme %s marked as belonging to a series", programme->m_title.c_str());
        event.iFlags |= EPG_TAG_FLAG_IS_SERIES;
      }
      PVR->TransferEpgEntry(handle, &event);
    }

    return PVR_ERROR_NO_ERROR;
  }

  PVR_ERROR SetEPGTimeFrame(int)
  {
    return PVR_ERROR_NOT_IMPLEMENTED;
  }

  PVR_ERROR SignalStatus(PVR_SIGNAL_STATUS &signalStatus)
  {
    const ChannelPtr currentChannel = g_vbox->GetCurrentChannel();

    if (!currentChannel)
      return PVR_ERROR_NO_ERROR;

    try {
      ChannelStreamingStatus status = g_vbox->GetChannelStreamingStatus(currentChannel);

      // Adjust for Kodi's weird handling of the signal strength
      signalStatus.iSignal = static_cast<int>(status.GetSignalStrength()) * 655; 
      signalStatus.iSNR = static_cast<int>(status.m_signalQuality) * 655;
      signalStatus.iBER = status.GetBer();

      strncpy(signalStatus.strAdapterName,
        status.GetTunerName().c_str(), sizeof(signalStatus.strAdapterName));

      strncpy(signalStatus.strAdapterStatus,
        status.m_lockStatus.c_str(), sizeof(signalStatus.strAdapterStatus));
        
      strncpy(signalStatus.strServiceName,
        status.GetServiceName().c_str(), sizeof(signalStatus.strServiceName));

      strncpy(signalStatus.strMuxName,
        status.GetMuxName().c_str(), sizeof(signalStatus.strMuxName));
    }
    catch (VBoxException &e)
    {
      g_vbox->LogException(e);
    }

    return PVR_ERROR_NO_ERROR;
  }

  PVR_ERROR GetDescrambleInfo(PVR_DESCRAMBLE_INFO*)
  {
    return PVR_ERROR_NOT_IMPLEMENTED;
  }

  bool OpenLiveStream(const PVR_CHANNEL &channel)
  {
    // Find the channel
    const ChannelPtr channelPtr = g_vbox->GetChannel(channel.iUniqueId);
    
    if (!channelPtr)
      return false;

    // Remember the current channel if the buffer was successfully opened
    if (g_timeshiftBuffer->Open(channelPtr->m_url))
    {
      g_vbox->SetCurrentChannel(channelPtr);
      return true;
    }

    CloseLiveStream();
    g_vbox->SetChannelStreamingStatus(channelPtr);
    return false;
  }

  void CloseLiveStream(void)
  {
    g_timeshiftBuffer->Close();
  }

  int ReadLiveStream(unsigned char *pBuffer, unsigned int iBufferSize)
  {
    return g_timeshiftBuffer->Read(pBuffer, iBufferSize);
  }

  long long PositionLiveStream(void)
  {
    return g_timeshiftBuffer->Position();
  }

  long long LengthLiveStream(void)
  {
    return g_timeshiftBuffer->Length();
  }

  long long SeekLiveStream(long long iPosition, int iWhence /* = SEEK_SET */)
  {
    return g_timeshiftBuffer->Seek(iPosition, iWhence);
  }

  bool CanPauseStream(void)
  {
    return g_vbox->GetSettings().m_timeshiftEnabled;
  }

  bool CanSeekStream(void)
  {
    return g_vbox->GetSettings().m_timeshiftEnabled;
  }

  time_t GetBufferTimeStart()
  {
    return g_timeshiftBuffer->GetStartTime();
  }

  time_t GetBufferTimeEnd()
  {
    return g_timeshiftBuffer->GetEndTime();
  }
  
  bool IsRealTimeStream(void) 
  {
	  const ChannelPtr currentChannel = g_vbox->GetCurrentChannel();

    return currentChannel != nullptr;
  }

  bool SetProgramReminder(unsigned int epgUid)
  {
    ChannelPtr selectedChannel = nullptr;

    // find channel with the program in context
    auto &channels = g_vbox->GetChannels();
    auto it = std::find_if(channels.cbegin(), channels.cend(),
      [&epgUid](const ChannelPtr &channel)
    {
      const Schedule schedule = g_vbox->GetSchedule(channel);
      const xmltv::ProgrammePtr programme = (schedule.schedule) ? schedule.schedule->GetProgramme(epgUid) : nullptr;
      return (programme != nullptr);
    });
    // if channel doesn't exist - return error
    if (it == channels.cend())
    {
      XBMC->QueueNotification(QUEUE_ERROR, "Program not found for that channel");
      return false;
    }
    else
    {
      // otherwise - get the program object and add a reminder with it as parameter
      selectedChannel = *it;
      const Schedule schedule = g_vbox->GetSchedule(selectedChannel);
      const xmltv::ProgrammePtr programme = (schedule.schedule) ? schedule.schedule->GetProgramme(epgUid) : nullptr;
      if (programme)
      {
        try {
          g_vbox->AddReminder(selectedChannel, programme);
        }
        catch (VBoxException &e)
        {
          g_vbox->LogException(e);
          return false;
        }
        XBMC->QueueNotification(QUEUE_INFO, "Reminder added");
      }
    }
    return true;
  }

  const ChannelPtr FindChannelForEPGReminder(int epgUid)
  {
    const xmltv::ProgrammePtr programme = nullptr;
    const std::vector<ChannelPtr> &channels = g_vbox->GetChannels();

    // Find channel that contains this programme
    const std::vector<ChannelPtr>::const_iterator it = std::find_if(channels.cbegin(), channels.cend(),
      [&epgUid](const ChannelPtr &channel)
    {
      const Schedule schedule = g_vbox->GetSchedule(channel);
      const xmltv::ProgrammePtr programme = (schedule.schedule) ? schedule.schedule->GetProgramme(epgUid) : nullptr;
      return (programme);
    });
    // Find the channel's schedule
    if (it == channels.cend())
      XBMC->QueueNotification(QUEUE_WARNING, "Reminder could not find the requested channel");
    return *it;
  }

  static time_t GetOffsetTime(time_t time)
  {
    std::string xmltvTime = g_vbox->CreateTimestamp(time);
    std::string tzString = ::xmltv::Utilities::GetTimezoneOffset(xmltvTime);
    return ::xmltv::Utilities::GetTimezoneAdjustment(tzString);
  }

  static bool SetManualReminder(const PVR_MENUHOOK_DATA &item)
  {
    time_t currTime = time(nullptr), reminderTime;
    ChannelPtr selectedChannel = nullptr;
    char buffer[256];

    memset(buffer, 0, sizeof(buffer));
    // get channel in context
    selectedChannel = g_vbox->GetChannel(item.data.channel.iUniqueId);
    if (!selectedChannel)
      return false;

    try {
      // create the current time's formatted timestamp (for user input)
      time_t tzOs = GetOffsetTime(currTime);
      // add timezone offset
      currTime += tzOs;
      std::tm tm = *std::gmtime(&currTime);

      // get program time & name (from user dialogs)
      if (!GUI->Dialog_Numeric_ShowAndGetDate(tm, "Program starts at"))
        return false;
      if (!GUI->Dialog_Numeric_ShowAndGetTime(tm, "Program starts at"))
        return false;
      if (!GUI->Dialog_Keyboard_ShowAndGetInput(*buffer, sizeof(buffer), "Program title", true, false))
        return false;

      std::string progTitle(buffer); 
      // remove timezone offset
      reminderTime = compat::timegm(&tm) - tzOs;
      // add reminder using manual time & title
      g_vbox->AddReminder(selectedChannel, reminderTime, progTitle);
    }
    catch (VBoxException &e)
    {
      g_vbox->LogException(e);
      return false;
    }
    XBMC->QueueNotification(QUEUE_INFO, "Reminder added");
    return true;
  }

  PVR_ERROR CallMenuHook(const PVR_MENUHOOK &menuhook, const PVR_MENUHOOK_DATA &item)
  {
    if (menuhook.category == PVR_MENUHOOK_SETTING)
    {
      if (menuhook.iHookId == MENUHOOK_ID_RESCAN_EPG)
      {
        XBMC->QueueNotification(ADDON::QUEUE_INFO, "Rescanning EPG, this will take a while");
        g_vbox->StartEPGScan();
        return PVR_ERROR_NO_ERROR;
      }
      else if (menuhook.iHookId == MENUHOOK_ID_SYNC_EPG)
      {
        XBMC->QueueNotification(ADDON::QUEUE_INFO, "Getting EPG from VBox device");
        g_vbox->SyncEPGNow();
        return PVR_ERROR_NO_ERROR;
      }
      return PVR_ERROR_INVALID_PARAMETERS;
    }
    if (menuhook.category == PVR_MENUHOOK_EPG)
    {
      if (menuhook.iHookId == MENUHOOK_ID_EPG_REMINDER)
      {
        if (SetProgramReminder(item.data.iEpgUid))
          return PVR_ERROR_NO_ERROR;
      }
      else if (menuhook.iHookId == MENUHOOK_ID_CANCEL_EPG_REMINDER)
      {
        if (g_vbox->DeleteProgramReminders(item.data.iEpgUid))
          XBMC->QueueNotification(ADDON::QUEUE_INFO, "Reminder canceled");
        else
          XBMC->QueueNotification(ADDON::QUEUE_WARNING, "Program does not have a reminder to cancel");
        return PVR_ERROR_NO_ERROR;
      }
      return PVR_ERROR_INVALID_PARAMETERS;
    }
    else if (menuhook.category == PVR_MENUHOOK_CHANNEL)
    {
      if (menuhook.iHookId == MENUHOOK_ID_MANUAL_REMINDER)
      {
        if (SetManualReminder(item))
          return PVR_ERROR_NO_ERROR;
      }
      else if (menuhook.iHookId == MENUHOOK_ID_CANCEL_CHANNEL_REMINDER)
      {
        if (g_vbox->DeleteChannelReminders(g_vbox->GetChannel(item.data.channel.iUniqueId)))
          XBMC->QueueNotification(ADDON::QUEUE_INFO, "Removed channel's existing reminders");
        else
          XBMC->QueueNotification(ADDON::QUEUE_WARNING, "Channel does not have reminders to cancel");
        return PVR_ERROR_NO_ERROR;
      }
      return PVR_ERROR_INVALID_PARAMETERS;
    }
    return PVR_ERROR_NOT_IMPLEMENTED;
  }

  // Management methods
  PVR_ERROR DialogChannelScan(void) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR DeleteChannel(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR RenameChannel(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR MoveChannel(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR DialogChannelSettings(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR DialogAddChannel(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR OpenDialogChannelScan(void) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR OpenDialogChannelSettings(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR OpenDialogChannelAdd(const PVR_CHANNEL &channel) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR UndeleteRecording(const PVR_RECORDING& recording) { return PVR_ERROR_NOT_IMPLEMENTED; }

  // Channel group methods
  int GetChannelGroupsAmount(void) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR GetChannelGroups(ADDON_HANDLE handle, bool bRadio) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR GetChannelGroupMembers(ADDON_HANDLE handle, const PVR_CHANNEL_GROUP &group) { return PVR_ERROR_NOT_IMPLEMENTED; }

  // Recording stream methods
  bool OpenRecordedStream(const PVR_RECORDING &recording) { return false; }
  void CloseRecordedStream(void) {}
  int ReadRecordedStream(unsigned char *pBuffer, unsigned int iBufferSize) { return 0; }
  long long SeekRecordedStream(long long iPosition, int iWhence /* = SEEK_SET */) { return 0; }
  long long PositionRecordedStream(void) { return -1; }
  long long LengthRecordedStream(void) { return 0; }

  // Channel stream methods
  bool SwitchChannel(const PVR_CHANNEL &channel) { CloseLiveStream(); return OpenLiveStream(channel); }

  // Demuxer methods
  void DemuxReset(void) {}
  void DemuxFlush(void) {}
  void DemuxAbort(void) {}
  DemuxPacket* DemuxRead(void) { return NULL; }
  PVR_ERROR GetStreamProperties(PVR_STREAM_PROPERTIES* pProperties) { return PVR_ERROR_NOT_IMPLEMENTED; }
  
  // Recording methods (not supported by VBox)
  PVR_ERROR RenameRecording(const PVR_RECORDING &recording) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR SetRecordingLifetime(const PVR_RECORDING*) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR SetRecordingPlayCount(const PVR_RECORDING &recording, int count) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR SetRecordingLastPlayedPosition(const PVR_RECORDING &recording, int lastplayedposition) { return PVR_ERROR_NOT_IMPLEMENTED; }
  int GetRecordingLastPlayedPosition(const PVR_RECORDING &recording) { return -1; }
  PVR_ERROR GetRecordingEdl(const PVR_RECORDING&, PVR_EDL_ENTRY[], int*) { return PVR_ERROR_NOT_IMPLEMENTED; };
  PVR_ERROR DeleteAllRecordingsFromTrash() { return PVR_ERROR_NOT_IMPLEMENTED; }

  // Miscellaneous unimplemented methods
  unsigned int GetChannelSwitchDelay(void) { return 0; }

  // Timeshift methods
  void PauseStream(bool bPaused) {}
  bool SeekTime(double, bool, double*) { return false; }
  void SetSpeed(int) {};
  time_t GetPlayingTime() { return 0; }
  bool IsTimeshifting(void) { return false; }

  // Deprecated (unused)
  const char * GetLiveStreamURL(const PVR_CHANNEL &channel) { return ""; }
}
