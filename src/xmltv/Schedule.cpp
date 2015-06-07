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

#include "Schedule.h"
#include <algorithm>
#include <iterator>
#include "../vbox/ContentIdentifier.h"

using namespace xmltv;

Schedule::Schedule(ChannelPtr& channel)
  : m_channel(channel)
{
  
}

void Schedule::AddProgramme(ProgrammePtr programme)
{
  m_programmes.push_back(programme);
}

const ProgrammePtr Schedule::GetProgramme(int programmeUniqueId) const
{
  auto it = std::find_if(
    m_programmes.cbegin(),
    m_programmes.cend(),
    [programmeUniqueId](const ProgrammePtr &programme)
  {
    return programmeUniqueId == vbox::ContentIdentifier::GetUniqueId(programme.get());
  });

  if (it != m_programmes.cend())
    return *it;

  return nullptr;
}

const Segment Schedule::GetSegment(time_t startTime, time_t endTime) const
{
  Segment segment;

  // Copy matching programmes to the segment
  std::copy_if(
    m_programmes.cbegin(),
    m_programmes.cend(),
    std::back_inserter(segment),
    [startTime, endTime](const ProgrammePtr &programme)
  {
    time_t programmeStartTime = Utilities::XmltvToUnixTime(programme->m_startTime);
    time_t programmeEndTime = Utilities::XmltvToUnixTime(programme->m_endTime);

    return programmeStartTime >= startTime && programmeEndTime <= endTime;
  });

  return segment;
}
