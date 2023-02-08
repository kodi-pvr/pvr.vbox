/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2015 Sam Stenvall
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "vbox/VBox.h"

#include <kodi/addon-instance/PVR.h>

namespace timeshift
{
class Buffer;
}

namespace vbox
{
class RecordingReader;
}

class ATTR_DLL_LOCAL CVBoxInstance : public kodi::addon::CInstancePVRClient, private vbox::VBox
{
public:
  CVBoxInstance(const kodi::addon::IInstanceInfo& instance);
  ~CVBoxInstance();

  // kodi::addon::CInstancePVRClient -> kodi::addon::IAddonInstance overrides
  ADDON_STATUS SetInstanceSetting(const std::string& settingName,
                                  const kodi::addon::CSettingValue& settingValue) override;

  ADDON_STATUS Initialize();
  const vbox::InstanceSettings& GetSettings() const { return vbox::VBox::GetSettings(); }

  PVR_ERROR GetCapabilities(kodi::addon::PVRCapabilities& capabilities) override;
  PVR_ERROR GetBackendName(std::string& name) override;
  PVR_ERROR GetBackendVersion(std::string& version) override;
  PVR_ERROR GetBackendHostname(std::string& hostname) override;
  PVR_ERROR GetConnectionString(std::string& connection) override;
  PVR_ERROR GetDriveSpace(uint64_t& total, uint64_t& used) override;

  PVR_ERROR CallSettingsMenuHook(const kodi::addon::PVRMenuhook& menuhook) override;

  PVR_ERROR GetChannelsAmount(int& amount) override;
  PVR_ERROR GetChannels(bool radio, kodi::addon::PVRChannelsResultSet& results) override;
  PVR_ERROR GetSignalStatus(int channelUid, kodi::addon::PVRSignalStatus& signalStatus) override;

  PVR_ERROR GetRecordingsAmount(bool deleted, int& amount) override;
  PVR_ERROR GetRecordings(bool deleted, kodi::addon::PVRRecordingsResultSet& results) override;
  PVR_ERROR DeleteRecording(const kodi::addon::PVRRecording& recording) override;

  PVR_ERROR GetTimerTypes(std::vector<kodi::addon::PVRTimerType>& types) override;
  PVR_ERROR GetTimersAmount(int& amount) override;
  PVR_ERROR GetTimers(kodi::addon::PVRTimersResultSet& results) override;
  PVR_ERROR AddTimer(const kodi::addon::PVRTimer& timer) override;
  PVR_ERROR DeleteTimer(const kodi::addon::PVRTimer& timer, bool forceDelete) override;
  PVR_ERROR UpdateTimer(const kodi::addon::PVRTimer& timer) override;

  PVR_ERROR GetEPGForChannel(int channelUid, time_t start, time_t end, kodi::addon::PVREPGTagsResultSet& results) override;

  bool OpenLiveStream(const kodi::addon::PVRChannel& channel) override;
  void CloseLiveStream() override;
  int ReadLiveStream(unsigned char* pBuffer, unsigned int iBufferSize) override;
  int64_t LengthLiveStream() override;
  int64_t SeekLiveStream(int64_t iPosition, int iWhence /* = SEEK_SET */) override;
  bool CanPauseStream() override;
  bool CanSeekStream() override;
  bool IsRealTimeStream() override;
  PVR_ERROR GetStreamTimes(kodi::addon::PVRStreamTimes& times) override;

  bool OpenRecordedStream(const kodi::addon::PVRRecording & recording) override;
  void CloseRecordedStream() override;
  int ReadRecordedStream(unsigned char* buffer, unsigned int size) override;
  int64_t SeekRecordedStream(int64_t position, int whence) override;
  int64_t LengthRecordedStream() override;

private:
  vbox::RecordingReader* m_recordingReader = nullptr;
  timeshift::Buffer* m_timeshiftBuffer = nullptr;
};
