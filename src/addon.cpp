/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2015 Sam Stenvall
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "addon.h"
#include "VBoxInstance.h"
#include "vbox/SettingsMigration.h"

using namespace vbox;

ADDON_STATUS CVBoxAddon::Create()
{
  /* Init settings */
  m_settings.reset(new AddonSettings());

  kodi::Log(ADDON_LOG_DEBUG,  "%s starting PVR client...", __func__);

  return ADDON_STATUS_OK;
}

ADDON_STATUS CVBoxAddon::CreateInstance(const kodi::addon::IInstanceInfo& instance, KODI_ADDON_INSTANCE_HDL& hdl)
{
  if (instance.IsType(ADDON_INSTANCE_PVR))
  {
    kodi::Log(ADDON_LOG_DEBUG, "creating VBox Gateway PVR addon");

    m_vbox = new CVBoxInstance(instance);
    ADDON_STATUS status = m_vbox->Initialize();

    // Try to migrate settings from a pre-multi-instance setup
    if (SettingsMigration::MigrateSettings(*m_vbox))
    {
      // Initial client operated on old/incomplete settings
      delete m_vbox;
      m_vbox = new CVBoxInstance(instance);
    }

    hdl = m_vbox;

    return status;
  }

  return ADDON_STATUS_UNKNOWN;
}

ADDON_STATUS CVBoxAddon::SetSetting(const std::string& settingName, const kodi::addon::CSettingValue& settingValue)
{
  return m_settings->SetSetting(settingName, settingValue);
}

void CVBoxAddon::DestroyInstance(const kodi::addon::IInstanceInfo& instance, const KODI_ADDON_INSTANCE_HDL hdl)
{
  if (instance.IsType(ADDON_INSTANCE_PVR))
  {
    m_vbox = nullptr;
  }
}

ADDONCREATOR(CVBoxAddon)
