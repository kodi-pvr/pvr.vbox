/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2015 Sam Stenvall
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <memory>
#include <string>

namespace xmltv
{

  class Channel;
  typedef std::shared_ptr<Channel> ChannelPtr;

  /**
   * Represents a channel
   */
  class Channel
  {
  public:
    Channel(const std::string& id, const std::string& displayName);
    ~Channel() = default;

    std::string m_id;
    std::string m_displayName;
    std::string m_icon;
  };
} // namespace xmltv
