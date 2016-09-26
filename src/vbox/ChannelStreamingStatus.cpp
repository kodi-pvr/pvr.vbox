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

#include "ChannelStreamingStatus.h"
#include "../compat.h"
#include <sstream>
#include <stdexcept>

using namespace vbox;

const int ChannelStreamingStatus::RFLEVEL_MIN = -96;
const int ChannelStreamingStatus::RFLEVEL_MAX = -60;

std::string ChannelStreamingStatus::GetServiceName() const
{
  if (!m_active)
    return "";

  std::stringstream ss;
  ss << "SID " << m_sid;

  return ss.str();
}

std::string ChannelStreamingStatus::GetMuxName() const
{
  if (!m_active)
    return "";

  std::stringstream ss;
  ss << m_lockedMode << " @ " << m_frequency << " (" << m_modulation << ")";

  return ss.str();
}

std::string ChannelStreamingStatus::GetTunerName() const
{
  if (!m_active)
    return "";

  std::stringstream ss;
  ss << m_tunerType << " tuner #" << m_tunerId;

  return ss.str();
}

unsigned int ChannelStreamingStatus::GetSignalStrength() const
{
  if (!m_active)
    return 0;

  unsigned int rfLevel = 0;

  try {
    // Convert the RF level to an integer
    rfLevel = compat::stoui(m_rfLevel);

    // If the level is above the maximum we consider it to be perfect
    if (rfLevel > RFLEVEL_MAX)
      return 100;

    // Normalize the value to between 0 and 1
    // TODO: This is not very scientific
    double normalized = static_cast<double>(rfLevel - RFLEVEL_MIN) /
                        static_cast<double>(RFLEVEL_MAX - RFLEVEL_MIN);

    return static_cast<unsigned int>(normalized * 100);
  }
  catch (std::invalid_argument)
  {
    
  }

  return rfLevel;
}

long ChannelStreamingStatus::GetBer() const
{
  if (!m_active)
    return 0;

  try {
    // Make sure it's not detected as hexadecimal
    return compat::stol(m_ber);
  }
  catch (std::invalid_argument)
  {
    return 0;
  }
}
