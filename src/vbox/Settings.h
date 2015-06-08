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

namespace vbox {

  /**
   * Represents a set of parameters required to make a connection
   */
  class ConnectionParameters
  {
  public:
    std::string hostname;
    int httpPort;
    int upnpPort;
    int timeout;

    /**
     * @return whether the connection parameters appear valid
     */
    bool AreValid() const
    {
      return !hostname.empty() && httpPort > 0 && upnpPort > 0 && timeout > 0;
    }
  };

  /**
   * Represents the settings for this addon
   */
  class Settings {
  public:
    ConnectionParameters m_internalConnectionParams;
    ConnectionParameters m_externalConnectionParams;
    bool m_useExternalXmltv;
    std::string m_externalXmltvPath;
    bool m_preferExternalXmltv;
    bool m_useExternalXmltvIcons;
    bool m_timeshiftEnabled;
    std::string m_timeshiftBufferPath;
  };
}