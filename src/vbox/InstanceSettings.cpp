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
  if (!m_instance.CheckInstanceSettingString("hostname", m_internalConnectionParams.hostname))
    m_internalConnectionParams.hostname = "";
  if (!m_instance.CheckInstanceSettingInt("http_port", m_internalConnectionParams.httpPort))
    m_internalConnectionParams.httpPort = 80;
  if (!m_instance.CheckInstanceSettingInt("https_port", m_internalConnectionParams.httpsPort))
    m_internalConnectionParams.httpsPort = 0;
  if (!m_instance.CheckInstanceSettingInt("upnp_port", m_internalConnectionParams.upnpPort))
    m_internalConnectionParams.upnpPort = 55555;
  if (!m_instance.CheckInstanceSettingInt("connection_timeout", m_internalConnectionParams.timeout))
    m_internalConnectionParams.timeout = 3;

  if (!m_instance.CheckInstanceSettingString("external_hostname", m_externalConnectionParams.hostname))
    m_externalConnectionParams.hostname = "";
  if (!m_instance.CheckInstanceSettingInt("external_http_port", m_externalConnectionParams.httpPort))
    m_externalConnectionParams.httpPort = 19999;
  if (!m_instance.CheckInstanceSettingInt("external_https_port", m_externalConnectionParams.httpsPort))
    m_externalConnectionParams.httpsPort = 0;
  if (!m_instance.CheckInstanceSettingInt("external_upnp_port", m_externalConnectionParams.upnpPort))
    m_externalConnectionParams.upnpPort = 55555;
  if (!m_instance.CheckInstanceSettingInt("external_connection_timeout", m_externalConnectionParams.timeout))
    m_externalConnectionParams.timeout = 10;

  if (!m_instance.CheckInstanceSettingEnum<vbox::ChannelOrder>("set_channelid_using_order", m_setChannelIdUsingOrder))
    m_setChannelIdUsingOrder = CH_ORDER_BY_LCN;
  if (!m_instance.CheckInstanceSettingBoolean("timeshift_enabled", m_timeshiftEnabled))
    m_timeshiftEnabled = false;
  if (!m_instance.CheckInstanceSettingString("timeshift_path", m_timeshiftBufferPath))
    m_timeshiftBufferPath = "special://userdata/addon_data/pvr.vbox";
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
