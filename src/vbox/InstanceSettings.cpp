/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "InstanceSettings.h"

using namespace vbox;

InstanceSettings::InstanceSettings(kodi::addon::IAddonInstance& instance)
  : m_instance(instance)
{
  ReadSettings();
}

void InstanceSettings::ReadSettings()
{
  m_internalConnectionParams.hostname = kodi::addon::GetSettingString("hostname", "");
  m_internalConnectionParams.httpPort = kodi::addon::GetSettingInt("http_port", 80);
  m_internalConnectionParams.httpsPort = kodi::addon::GetSettingInt("https_port", 0);
  m_internalConnectionParams.upnpPort = kodi::addon::GetSettingInt("upnp_port", 55555);
  m_internalConnectionParams.timeout = kodi::addon::GetSettingInt("connection_timeout", 3);

  m_externalConnectionParams.hostname = kodi::addon::GetSettingString("external_hostname", "");
  m_externalConnectionParams.httpPort = kodi::addon::GetSettingInt("external_http_port", 19999);
  m_externalConnectionParams.httpsPort = kodi::addon::GetSettingInt("external_https_port", 0);
  m_externalConnectionParams.upnpPort = kodi::addon::GetSettingInt("external_upnp_port", 55555);
  m_externalConnectionParams.timeout = kodi::addon::GetSettingInt("connection_timeout", 10);

  m_setChannelIdUsingOrder = kodi::addon::GetSettingEnum<vbox::ChannelOrder>("set_channelid_using_order", CH_ORDER_BY_LCN);
  m_timeshiftEnabled = kodi::addon::GetSettingBoolean("timeshift_enabled", false);
  m_timeshiftBufferPath = kodi::addon::GetSettingString("timeshift_path", "special://userdata/addon_data/pvr.vbox");
}

ADDON_STATUS InstanceSettings::SetSetting(const std::string& settingName, const kodi::addon::CSettingValue& settingValue)
{
#define UPDATE_STR(key, var) \
  if (settingName == key) \
  { \
    if (var != settingValue.GetString()) \
    { \
      kodi::Log(ADDON_LOG_INFO, "updated setting %s from '%s' to '%s'", settingName.c_str(), var.c_str(), settingValue.GetString().c_str()); \
      return ADDON_STATUS_NEED_RESTART; \
    } \
    return ADDON_STATUS_OK; \
  }

#define UPDATE_INT(key, var) \
  if (settingName == key) \
  { \
    if (var != settingValue.GetInt()) \
    { \
      kodi::Log(ADDON_LOG_INFO, "updated setting %s from '%d' to '%d'", settingName.c_str(), var, settingValue.GetInt()); \
      return ADDON_STATUS_NEED_RESTART; \
    } \
    return ADDON_STATUS_OK; \
  }

#define UPDATE_BOOL(key, var) \
  if (settingName == key) \
  { \
    if (var != settingValue.GetBoolean()) \
    { \
      kodi::Log(ADDON_LOG_INFO, "updated setting %s from '%d' to '%d'", settingName.c_str(), var, settingValue.GetBoolean()); \
      return ADDON_STATUS_NEED_RESTART; \
    } \
    return ADDON_STATUS_OK; \
  }

  UPDATE_STR("hostname", m_internalConnectionParams.hostname);
  UPDATE_INT("http_port", m_internalConnectionParams.httpPort);
  UPDATE_INT("https_port", m_internalConnectionParams.httpsPort);
  UPDATE_INT("upnp_port", m_internalConnectionParams.upnpPort);
  UPDATE_INT("connection_timeout", m_internalConnectionParams.timeout);
  UPDATE_STR("external_hostname", m_externalConnectionParams.hostname);
  UPDATE_INT("external_http_port", m_externalConnectionParams.httpPort);
  UPDATE_INT("external_https_port", m_externalConnectionParams.httpsPort);
  UPDATE_INT("external_upnp_port", m_externalConnectionParams.upnpPort);
  UPDATE_INT("external_connection_timeout", m_externalConnectionParams.timeout);
  UPDATE_INT("set_channelid_using_order", m_setChannelIdUsingOrder);
  UPDATE_BOOL("timeshift_enabled", m_timeshiftEnabled);
  UPDATE_STR("timeshift_path", m_timeshiftBufferPath);

  return ADDON_STATUS_OK;
#undef UPDATE_BOOL
#undef UPDATE_INT
#undef UPDATE_STR
}
