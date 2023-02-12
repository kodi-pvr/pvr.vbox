/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2015 Sam Stenvall
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <sstream>
#include <string>

#include <kodi/AddonBase.h>

namespace vbox
{

  enum ChannelOrder
  {
    CH_ORDER_BY_LCN = 0,
    CH_ORDER_BY_INDEX
  };

  /**
   * Represents a set of parameters required to make a connection
   */
  class ATTR_DLL_LOCAL ConnectionParameters
  {
  public:
    std::string hostname;
    int httpPort;
    int httpsPort;
    int upnpPort;
    int timeout;

    /**
     * @return whether the connection parameters appear valid
     */
    bool AreValid() const
    {
      return !hostname.empty() && httpPort > 0 && upnpPort > 0 && timeout > 0;
    }

    /**
     * @return the URI scheme to use
     */
    std::string GetUriScheme() const
    {
      return UseHttps() ? "https" : "http";
    }

    /**
     * @return the URI authority to use
     */
    std::string GetUriAuthority() const
    {
      std::stringstream ss;
      int port = UseHttps() ? httpsPort : httpPort;
      ss << hostname << ":" << port;

      return ss.str();
    }

    /**
     * @return whether HTTPS should be used or not
     */
    bool UseHttps() const { return httpsPort > 0; }
  };

  /**
   * Represents the settings for this addon
   */
  class ATTR_DLL_LOCAL InstanceSettings
  {
  public:
    explicit InstanceSettings(kodi::addon::IAddonInstance& instance);
    ADDON_STATUS SetSetting(const std::string& settingName, const kodi::addon::CSettingValue& settingValue);

    ConnectionParameters m_internalConnectionParams;
    ConnectionParameters m_externalConnectionParams;
    ChannelOrder m_setChannelIdUsingOrder;
    bool m_timeshiftEnabled;
    std::string m_timeshiftBufferPath;

  private:
    InstanceSettings(const InstanceSettings&) = delete;
    void operator=(const InstanceSettings&) = delete;

    void ReadSettings();

    kodi::addon::IAddonInstance& m_instance;
  };
} // namespace vbox
