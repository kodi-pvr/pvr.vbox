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
#include "platform/util/util.h"
#include "xbmc_pvr_dll.h"
#include "client.h"
#include "vbox/Exceptions.h"
#include "vbox/VBox.h"

using namespace ADDON;
using namespace vbox;

// Initialize helpers
CHelper_libXBMC_addon *XBMC = NULL;
CHelper_libXBMC_pvr   *PVR = NULL;

// Initialize globals
ADDON_STATUS   g_status = ADDON_STATUS_UNKNOWN;
VBox *g_vbox;

// Initialize settings
std::string g_hostname = "";
int g_port = 80;

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

    UPDATE_STR(g_hostname, "hostname", buffer, "localhost");
    UPDATE_INT(g_port, "port", 80);

#undef UPDATE_INT
#undef UPDATE_STR
  }

  ADDON_STATUS ADDON_Create(void* hdl, void* props)
  {
    if (!hdl || !props)
      return ADDON_STATUS_UNKNOWN;

    PVR_PROPERTIES* pvrprops = (PVR_PROPERTIES*)props;

    // Instantiate helpers
    XBMC = new CHelper_libXBMC_addon;
    PVR = new CHelper_libXBMC_pvr;

    if (!XBMC->RegisterMe(hdl) ||
      !PVR->RegisterMe(hdl))
    {
      SAFE_DELETE(XBMC);
      SAFE_DELETE(PVR);
      return ADDON_STATUS_PERMANENT_FAILURE;
    }

    // Read the settings
    ADDON_ReadSettings();
    Settings settings;
    settings.m_hostname = g_hostname;
    settings.m_port = g_port;
    settings.m_timeout = 5000; // TODO: Make configurable

    // Start the addon
    VBox::Log(LOG_DEBUG, "creating VBox Gateway PVR addon");
    g_status = ADDON_STATUS_UNKNOWN;
    g_vbox = new VBox(settings);

    try {
      g_vbox->Initialize();
      g_status = ADDON_STATUS_OK;
    }
    catch (VBoxException &e) {
      VBox::LogException(e);
      g_status = ADDON_STATUS_LOST_CONNECTION;
    }

    return g_status;
  }

  ADDON_STATUS ADDON_GetStatus()
  {
    return g_status;
  }

  void ADDON_Destroy()
  {
    delete g_vbox;
    g_status = ADDON_STATUS_UNKNOWN;
  } 

  bool ADDON_HasSettings()
  {
    return true;
  }

  unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
  {
    return 0;
  }

  ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
  {
    return ADDON_STATUS_OK;
  }

  void ADDON_Stop()
  {
  }

  void ADDON_FreeSettings()
  {
  }

  void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
  {
  }

  const char* GetPVRAPIVersion(void)
  {
    static const char *strApiVersion = XBMC_PVR_API_VERSION;
    return strApiVersion;
  }

  const char* GetMininumPVRAPIVersion(void)
  {
    static const char *strMinApiVersion = XBMC_PVR_MIN_API_VERSION;
    return strMinApiVersion;
  }

  const char* GetGUIAPIVersion(void)
  {
    static const char *strGuiApiVersion = XBMC_GUI_API_VERSION;
    return strGuiApiVersion;
  }

  const char* GetMininumGUIAPIVersion(void)
  {
    static const char *strMinGuiApiVersion = XBMC_GUI_MIN_API_VERSION;
    return strMinGuiApiVersion;
  }

  PVR_ERROR GetAddonCapabilities(PVR_ADDON_CAPABILITIES* pCapabilities)
  {
    pCapabilities->bSupportsTV = true;
    pCapabilities->bSupportsRadio = true;

    // Wait for initialization until we decide if we support recordings or not.
    // Recording is only possible when external media is present
    if (g_vbox->GetStateHandler().WaitForState(vbox::StartupState::INITIALIZED)
      && g_vbox->SupportsRecordings())
    {
      pCapabilities->bSupportsRecordings = true;
      pCapabilities->bSupportsTimers = true;
    }

    return PVR_ERROR_NO_ERROR;
  }

  const char *GetBackendName(void)
  {
    return g_vbox->GetBackendName().c_str();
  }

  const char *GetBackendVersion(void)
  {
    return g_vbox->GetBackendVersion().c_str();
  }

  const char *GetConnectionString(void)
  {
    return g_vbox->GetConnectionString().c_str();
  }

  const char *GetBackendHostname(void)
  {
    return g_vbox->GetBackendHostname().c_str();
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
    try {
      auto channels = g_vbox->GetChannels();

      for (const auto &item : channels)
      {
        PVR_CHANNEL channel;
        memset(&channel, 0, sizeof(PVR_CHANNEL));

        channel.iUniqueId = item.GetUniqueId();
        channel.bIsRadio = item.IsRadio();
        channel.iChannelNumber = item.GetNumber();
        channel.iEncryptionSystem = item.IsEncrypted() ? 0xFFFF : 0x0000;

        strncpy(channel.strChannelName, item.GetName().c_str(),
          sizeof(channel.strChannelName) - 1);
        strncpy(channel.strIconPath, item.GetIconUrl().c_str(),
          sizeof(channel.strIconPath) - 1);

        PVR->TransferChannelEntry(handle, &channel);
      }
    }
    catch (VBoxException &e)
    {
      g_vbox->LogException(e);
      return PVR_ERROR_FAILED;
    }

    return PVR_ERROR_NO_ERROR;
  }

  PVR_ERROR GetDriveSpace(long long *iTotal, long long *iUsed)
  {
    *iTotal = g_vbox->GetRecordingTotalSpace();
    *iUsed = g_vbox->GetRecordingUsedSpace();

    return PVR_ERROR_NO_ERROR;
  }

  int GetRecordingsAmount(bool deleted)
  {
    return g_vbox->GetRecordingsAmount();
  }

  PVR_ERROR GetRecordings(ADDON_HANDLE handle, bool deleted)
  {
    try {
      auto recordings = g_vbox->GetRecordingsAndTimers();

      for (const auto &item : recordings)
      {
        // Skip timers
        if (item.IsTimer())
          continue;

        PVR_RECORDING recording;
        memset(&recording, 0, sizeof(PVR_RECORDING));

        // TODO: Duration, plot
        recording.recordingTime = item.m_start;
        recording.iDuration = static_cast<int>(item.m_end - item.m_start);

        strncpy(recording.strChannelName, item.m_channelName.c_str(),
          sizeof(recording.strChannelName) - 1);

        strncpy(recording.strRecordingId, std::to_string(item.m_id).c_str(),
          sizeof(recording.strRecordingId));

        strncpy(recording.strStreamURL, item.m_url.c_str(),
          sizeof(recording.strStreamURL));

        strncpy(recording.strTitle, item.m_title.c_str(),
          sizeof(recording.strTitle));

        PVR->TransferRecordingEntry(handle, &recording);
      }
    }
    catch (VBoxException &e)
    {
      g_vbox->LogException(e);
      return PVR_ERROR_FAILED;
    }

    return PVR_ERROR_NO_ERROR;
  }

  PVR_ERROR DeleteRecording(const PVR_RECORDING &recording)
  {
    // DeleteRecording swallows its exceptions, but std::stoul throws if the 
    // value can't be converted
    try {
      unsigned int id = std::stoul(recording.strRecordingId);

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

  int GetTimersAmount(void)
  {
    return g_vbox->GetTimersAmount();
  }

  PVR_ERROR GetTimers(ADDON_HANDLE handle)
  {
    try {
      std::vector<Recording> recordings = g_vbox->GetRecordingsAndTimers();

      for (const auto &item : recordings)
      {
        // Skip recordings
        if (!item.IsTimer())
          continue;

        PVR_TIMER timer;
        memset(&timer, 0, sizeof(PVR_TIMER));

        timer.startTime = item.m_start;
        timer.endTime = item.m_end;
        timer.iClientIndex = item.m_id;

        // Convert the internal timer state to PVR_TIMER_STATE
        switch (item.GetState())
        {
        case RecordingState::SCHEDULED:
          timer.state = PVR_TIMER_STATE_SCHEDULED;
        case RecordingState::RECORDING:
          timer.state = PVR_TIMER_STATE_RECORDING;
        }

        // Find the timer's channel and use its unique ID
        auto channels = g_vbox->GetChannels();
        auto it = std::find_if(channels.cbegin(), channels.cend(),
          [&item](const Channel &channel)
        {
          return channel.GetInternalId() == item.m_channelId;
        });

        if (it != channels.cend())
          timer.iClientChannelUid = it->GetUniqueId();

        strncpy(timer.strTitle, item.m_title.c_str(),
          sizeof(timer.strTitle));

        // TODO: Set margins to whatever the API reports
        PVR->TransferTimerEntry(handle, &timer);
      }
    }
    catch (VBoxException &e)
    {
      g_vbox->LogException(e);
      return PVR_ERROR_FAILED;
    }

    return PVR_ERROR_NO_ERROR;
  }

  PVR_ERROR AddTimer(const PVR_TIMER &timer)
  {
    // Get the internal channel ID based on the channel unique ID found in the 
    // timer
    auto channels = g_vbox->GetChannels();
    auto it = std::find_if(channels.cbegin(), channels.cend(),
      [&timer](const Channel &channel)
    {
      return channel.GetUniqueId() == timer.iClientChannelUid;
    });

    if (it == channels.end())
      return PVR_ERROR_INVALID_PARAMETERS;

    std::string channelId = it->GetInternalId();

    try {
      g_vbox->AddTimer(channelId, timer.startTime, timer.endTime);
    }
    catch (VBoxException &e)
    {
      g_vbox->LogException(e);
      return PVR_ERROR_FAILED;
    }

    return PVR_ERROR_NO_ERROR;
  }

  PVR_ERROR DeleteTimer(const PVR_TIMER &timer, bool bForceDelete)
  {
    if (g_vbox->DeleteRecordingOrTimer(timer.iClientIndex))
      return PVR_ERROR_NO_ERROR;

    return PVR_ERROR_FAILED;
  }

  PVR_ERROR GetEPGForChannel(ADDON_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd)
  {
    return PVR_ERROR_SERVER_ERROR;
  }

  // Unused API methods
  // Management methods
  PVR_ERROR DialogChannelScan(void) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR CallMenuHook(const PVR_MENUHOOK &menuhook, const PVR_MENUHOOK_DATA &item) { return PVR_ERROR_NOT_IMPLEMENTED; }
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
  bool OpenLiveStream(const PVR_CHANNEL &channel) { return false; }
  void CloseLiveStream(void) {}
  bool SwitchChannel(const PVR_CHANNEL &channel) { CloseLiveStream(); return OpenLiveStream(channel); }
  int ReadLiveStream(unsigned char *pBuffer, unsigned int iBufferSize) { return 0; }
  long long SeekLiveStream(long long iPosition, int iWhence /* = SEEK_SET */) { return -1; }
  long long PositionLiveStream(void) { return -1; }
  long long LengthLiveStream(void) { return -1; }

  // Demuxer methods
  void DemuxReset(void) {}
  void DemuxFlush(void) {}
  void DemuxAbort(void) {}
  DemuxPacket* DemuxRead(void) { return NULL; }
  PVR_ERROR GetStreamProperties(PVR_STREAM_PROPERTIES* pProperties) { return PVR_ERROR_NOT_IMPLEMENTED; }
  
  // Recording methods (not supported by VBox)
  PVR_ERROR RenameRecording(const PVR_RECORDING &recording) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR SetRecordingPlayCount(const PVR_RECORDING &recording, int count) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR SetRecordingLastPlayedPosition(const PVR_RECORDING &recording, int lastplayedposition) { return PVR_ERROR_NOT_IMPLEMENTED; }
  int GetRecordingLastPlayedPosition(const PVR_RECORDING &recording) { return -1; }
  PVR_ERROR GetRecordingEdl(const PVR_RECORDING&, PVR_EDL_ENTRY[], int*) { return PVR_ERROR_NOT_IMPLEMENTED; };
  PVR_ERROR UpdateTimer(const PVR_TIMER &timer) { return PVR_ERROR_NOT_IMPLEMENTED; }
  PVR_ERROR DeleteAllRecordingsFromTrash() { return PVR_ERROR_NOT_IMPLEMENTED; }

  // Miscellaneous unimplemented methods
  PVR_ERROR SignalStatus(PVR_SIGNAL_STATUS &signalStatus) { return PVR_ERROR_NOT_IMPLEMENTED; }
  unsigned int GetChannelSwitchDelay(void) { return 0; }

  // Timeshift methods
  void PauseStream(bool bPaused) {}
  bool CanPauseStream(void) { return false; }
  bool CanSeekStream(void) { return false; }
  bool SeekTime(int, bool, double*) { return false; }
  void SetSpeed(int) {};
  time_t GetPlayingTime() { return 0; }
  time_t GetBufferTimeStart() { return 0; }
  time_t GetBufferTimeEnd() { return 0; }

  // Deprecated (unused)
  const char * GetLiveStreamURL(const PVR_CHANNEL &channel) { return ""; }
  int GetCurrentClientChannel(void) { return -1; }
}
