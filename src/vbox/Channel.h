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

    unsigned int GetUniqueId() const;

    std::string m_name;
    unsigned int m_number;
    std::string m_iconUrl;
    bool m_radio;
    std::string m_url;
    bool m_encrypted;

  private:
    /**
    * The XMLTV channel ID
    */
    std::string m_id;
  };
}
