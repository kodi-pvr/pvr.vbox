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
#include <functional>

namespace vbox {
  class Channel
  {
  public:
    Channel(std::string id, std::string name, std::string url)
      : m_id(id), m_name(name), m_url(url), m_radio(false), m_encrypted(false) {}
    ~Channel() {};

    std::string GetInternalId() const
    {
      return m_id;
    }

    unsigned int GetUniqueId() const
    {
      std::hash<std::string> hasher;
      return hasher(m_id);
    }

    std::string GetName() const
    {
      return m_name;
    }

    unsigned int GetNumber() const
    {
      return m_number;
    }

    void SetNumber(unsigned int number)
    {
      m_number = number;
    }

    std::string GetIconUrl() const
    {
      return m_iconUrl;
    }

    /**
     * Sets the icon URL
     * @param url the icon URL
     */
    void SetIconUrl(const std::string &url);

    bool IsRadio() const
    {
      return m_radio;
    }

    /**
     * Indicate whether this is a radio channel or not
     * @param radio true for radio
     */
    void SetRadio(bool radio)
    {
      m_radio = radio;
    }

    bool IsEncrypted() const
    {
      return m_encrypted;
    }

    /**
     * Indicate whether this channel is encrypted or not
     * @param encrypted true if encrypted
     */
    void SetEncrypted(bool encrypted)
    {
      m_encrypted = encrypted;
    }

  private:
    /**
    * The XMLTV channel ID
    */
    std::string m_id;

    /**
    * The channel name
    */
    std::string m_name;

    /**
     * The channel number
     */
    unsigned int m_number;

    /**
    * The icon URL
    */
    std::string m_iconUrl;

    /**
    * The channel URL
    */
    std::string m_url;

    /**
     * Radio or not
     */
    bool m_radio;

    /**
     * Encrypted or not
     * TODO: The VBox SDK does not expose functionality for retrieving the CAID
     */
    bool m_encrypted;
  };
}
