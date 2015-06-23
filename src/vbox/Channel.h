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

#include <string>
#include <memory>

namespace vbox {

  class Channel;
  typedef std::shared_ptr<Channel> ChannelPtr;

  /**
  * Represents a channel
  */
  class Channel
  {
  public:
    Channel(const std::string &uniqueId, const std::string &xmltvName,
      const std::string &name, const std::string &url)
      : m_uniqueId(uniqueId), m_index(0), m_xmltvName(xmltvName), m_name(name), m_number(0),
      m_radio(false), m_url(url), m_encrypted(false) {}
    ~Channel() {}

    bool operator== (const Channel &other)
    {
      return m_index == other.m_index &&
        m_xmltvName == other.m_xmltvName &&
        m_name == other.m_name &&
        m_number == other.m_number &&
        m_iconUrl == other.m_iconUrl &&
        m_radio == other.m_radio &&
        m_url == other.m_url &&
        m_encrypted == other.m_encrypted &&
        m_uniqueId == other.m_uniqueId;
    }

    bool operator!= (const Channel &other)
    {
      return !(*this == other);
    }

    /**
     * The internal name used by VBox
     */
    std::string m_uniqueId;

    /**
    * The index of the channel, as it appears in the API results. Needed for
    * some API requests.
    */
    unsigned int m_index;

    /**
    * The XMLTV channel ID
    */
    std::string m_xmltvName;

    std::string m_name;
    unsigned int m_number;
    std::string m_iconUrl;
    bool m_radio;
    std::string m_url;
    bool m_encrypted;
  };
}
