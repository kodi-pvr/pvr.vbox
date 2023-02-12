/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2015 Sam Stenvall
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/AddonBase.h>

#include <memory>

#include "vbox/AddonSettings.h"

namespace vbox
{
class InstanceSettings;
}

class CVBoxInstance;

class ATTR_DLL_LOCAL CVBoxAddon : public kodi::addon::CAddonBase
{
public:
  CVBoxAddon() = default;

  ADDON_STATUS Create() override;
  ADDON_STATUS CreateInstance(const kodi::addon::IInstanceInfo& instance, KODI_ADDON_INSTANCE_HDL& hdl) override;
  ADDON_STATUS SetSetting(const std::string& settingName, const kodi::addon::CSettingValue& settingValue) override;
  void DestroyInstance(const kodi::addon::IInstanceInfo& instance, const KODI_ADDON_INSTANCE_HDL hdl) override;

private:

  CVBoxInstance* m_vbox = nullptr;
  std::shared_ptr<vbox::AddonSettings> m_settings;
};
