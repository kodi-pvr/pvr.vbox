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
   * Represents the streaming status of a channel. Only the fields required by
   * Kodi are currently implemented.
   */
  class ChannelStreamingStatus
  {
  public:
    ChannelStreamingStatus() 
      : m_active(false), m_signalQuality(0), m_sid(0) {};

    ~ChannelStreamingStatus() {};

    /**
     * @return the service name (SID XXX)
     */
    std::string GetServiceName() const;

    /**
     * @return the mux name (XXX MHz (MODULATION))
     */
    std::string GetMuxName() const;

    /**
     * @return the tuner name
     */
    std::string GetTunerName() const;

    /**
     * @return the signal strength (between 0 and 100)
     */
    unsigned int GetSignalStrength() const;

    /**
     * @return the bit error rate
     */
    long GetBer() const;

    void SetServiceId(unsigned int sid)
    {
      m_sid = sid;
    }

    void SetTunerId(const std::string &tunerId)
    {
      m_tunerId = tunerId;
    }

    void SetTunerType(const std::string &tunerType)
    {
      m_tunerType = tunerType;
    }

    void SetRfLevel(const std::string &rfLevel)
    {
      m_rfLevel = rfLevel;
    }

    void SetBer(const std::string &ber)
    {
      m_ber = ber;
    }

  public:
    bool m_active;
    std::string m_lockStatus;
    std::string m_lockedMode;
    std::string m_modulation;
    std::string m_frequency;
    unsigned int m_signalQuality;

  private:
    const static int RFLEVEL_MIN;
    const static int RFLEVEL_MAX;

    unsigned int m_sid;
    std::string m_tunerId;
    std::string m_tunerType;
    std::string m_rfLevel;
    std::string m_ber;
  };
}
