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

#include "Recording.h"

#include "../xmltv/Utilities.h"

using namespace vbox;

Recording::Recording(const std::string& channelId, const std::string& channelName, RecordingState state)
  : m_id(0), m_seriesId(0), m_channelId(channelId), m_channelName(channelName), m_state(state)
{
}

Recording::~Recording()
{
}

bool Recording::IsRunning(const std::time_t now, const std::string& channelName, std::time_t startTime) const
{
  time_t timerStartTime = xmltv::Utilities::XmltvToUnixTime(m_startTime);
  time_t timerEndTime = xmltv::Utilities::XmltvToUnixTime(m_endTime);
  if (!(timerStartTime <= now && now <= timerEndTime))
    return false;
  if (!channelName.empty() && m_channelName != channelName)
    return false;
  if (timerStartTime != startTime)
    return false;
  return true;
}