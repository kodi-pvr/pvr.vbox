#pragma once
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

#include <functional>
#include "Channel.h"
#include "Recording.h"
#include "../xmltv/Programme.h"
#include "../xmltv/Utilities.h"
#include "../compat.h"

namespace vbox {
  /**
   * Static helper class for creating unique identifiers for arbitrary objects
   */
  class ContentIdentifier
  {
  public:

    /**
     * @return a unique ID for the channel
     */
    static unsigned int GetUniqueId(const vbox::ChannelPtr &channel)
    {
      std::hash<std::string> hasher;
      int uniqueId = hasher(channel->m_uniqueId);
      return std::abs(uniqueId);
    }

    /**
     * @return a unique ID for the recording. This implementation must match
     * that of xmltv::Programme so that recordings can be linked to programmes.
     */
    static unsigned int GetUniqueId(const vbox::Recording *recording)
    {
      std::hash<std::string> hasher;
      std::string timestamp = compat::to_string(::xmltv::Utilities::XmltvToUnixTime(recording->m_endTime));
      int uniqueId = hasher(std::string(recording->m_title) + timestamp);
      return std::abs(uniqueId);
    }

    /**
     * @return a unique ID for the programme
     */
    static unsigned int GetUniqueId(const xmltv::Programme *programme)
    {
      std::hash<std::string> hasher;
      std::string timestamp = compat::to_string(::xmltv::Utilities::XmltvToUnixTime(programme->m_endTime));
      int uniqueId = hasher(std::string(programme->m_title) + timestamp);
      return std::abs(uniqueId);
    }
  };
}
