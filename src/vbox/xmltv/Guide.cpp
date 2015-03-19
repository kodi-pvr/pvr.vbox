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

#include "Guide.h"

using namespace vbox::xmltv;

const Schedule* Guide::GetSchedule(const std::string &channelId) const
{
  auto it = m_schedules.find(channelId);

  if (it != m_schedules.cend())
    return it->second.get();

  return nullptr;
}

void Guide::AddProgramme(const std::string &channelId, ProgrammePtr programme)
{
  if (m_schedules.find(channelId) == m_schedules.end())
    AddSchedule(channelId, SchedulePtr(new Schedule));

  m_schedules[channelId]->push_back(std::move(programme));
}
