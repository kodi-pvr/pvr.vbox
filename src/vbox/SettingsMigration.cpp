/*
 *  Copyright (C) 2005-2022 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "SettingsMigration.h"

#include "kodi/General.h"

#include <algorithm>
#include <utility>
#include <vector>

using namespace vbox;

namespace
{
// <setting name, default value> maps
const std::vector<std::pair<const char*, const char*>> stringMap = {{"hostname", ""},
                                                                    {"external_hostname", ""},
                                                                    {"timeshift_path", "special://userdata/addon_data/pvr.vbox"}};

const std::vector<std::pair<const char*, int>> intMap = {{"http_port", 80},
                                                         {"https_port", 0},
                                                         {"upnp_port", 55555},
                                                         {"connection_timeout", 3},
                                                         {"external_http_port", 19999},
                                                         {"external_https_port", 0},
                                                         {"external_upnp_port", 55555},
                                                         {"external_connection_timeout", 10},
                                                         {"set_channelid_using_order", 0}};

const std::vector<std::pair<const char*, bool>> boolMap = {{"timeshift_enabled", false}};

} // unnamed namespace

bool SettingsMigration::MigrateSettings(kodi::addon::IAddonInstance& target)
{
  std::string stringValue;
  bool boolValue{false};
  int intValue{0};

  if (target.CheckInstanceSettingString("kodi_addon_instance_name", stringValue) &&
      !stringValue.empty())
  {
    // Instance already has valid instance settings
    return false;
  }

  // Read pre-multi-instance settings from settings.xml, transfer to instance settings
  SettingsMigration mig(target);

  for (const auto& setting : stringMap)
    mig.MigrateStringSetting(setting.first, setting.second);

  for (const auto& setting : intMap)
    mig.MigrateIntSetting(setting.first, setting.second);

  for (const auto& setting : boolMap)
    mig.MigrateBoolSetting(setting.first, setting.second);

  if (mig.Changed())
  {
    // Set a title for the new instance settings
    std::string title;
    target.CheckInstanceSettingString("hostname", title);
    if (title.empty())
      title = "Migrated Add-on Config";

    target.SetInstanceSettingString("kodi_addon_instance_name", title);

    return true;
  }
  return false;
}

bool SettingsMigration::IsMigrationSetting(const std::string& key)
{
  return std::any_of(stringMap.cbegin(), stringMap.cend(),
                     [&key](const auto& entry) { return entry.first == key; }) ||
         std::any_of(intMap.cbegin(), intMap.cend(),
                     [&key](const auto& entry) { return entry.first == key; }) ||
         std::any_of(boolMap.cbegin(), boolMap.cend(),
                     [&key](const auto& entry) { return entry.first == key; });
}

void SettingsMigration::MigrateStringSetting(const char* key, const std::string& defaultValue)
{
  std::string value;
  if (kodi::addon::CheckSettingString(key, value) && value != defaultValue)
  {
    m_target.SetInstanceSettingString(key, value);
    m_changed = true;
  }
}

void SettingsMigration::MigrateIntSetting(const char* key, int defaultValue)
{
  int value;
  if (kodi::addon::CheckSettingInt(key, value) && value != defaultValue)
  {
    m_target.SetInstanceSettingInt(key, value);
    m_changed = true;
  }
}

void SettingsMigration::MigrateBoolSetting(const char* key, bool defaultValue)
{
  bool value;
  if (kodi::addon::CheckSettingBoolean(key, value) && value != defaultValue)
  {
    m_target.SetInstanceSettingBoolean(key, value);
    m_changed = true;
  }
}
