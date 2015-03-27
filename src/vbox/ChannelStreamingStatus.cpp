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

#include "ChannelStreamingStatus.h"
#include <sstream>
#include <stdexcept>

using namespace vbox;

std::string ChannelStreamingStatus::GetServiceName() const
{
  std::stringstream ss;
  ss << "SID " << m_sid;

  return ss.str();
}

std::string ChannelStreamingStatus::GetTunerName() const
{
  std::stringstream ss;
  ss << m_tunerType << " tuner #" << m_tunerId;

  return ss.str();
}

unsigned int ChannelStreamingStatus::GetSignalStrength() const
{
  int rfLevel = 0;

  try {
    // Convert the RF level to an integer
    rfLevel = std::stoi(m_rfLevel);

    // Normalize the value to between 0 and 1
    // TODO: This is not very scientific
    double normalized = ((double)(rfLevel - RFLEVEL_MIN)) /
      ((double)(RFLEVEL_MAX - RFLEVEL_MIN));

    return (unsigned int)(normalized * 100);
  }
  catch (std::invalid_argument)
  {
    return 0;
  }

  return rfLevel;
}

long ChannelStreamingStatus::GetBer() const
{
  try {
    // Make sure it's not detected as hexadecimal
    return std::stol(m_ber, nullptr, 10);
  }
  catch (std::invalid_argument)
  {
    return 0;
  }
}
