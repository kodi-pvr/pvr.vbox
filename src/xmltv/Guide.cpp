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
#include <algorithm>
#include "Utilities.h"
#include "tinyxml2.h"
#include "../vbox/ContentIdentifier.h"

using namespace xmltv;
using namespace tinyxml2;

Guide::Guide(const XMLElement *m_content)
{
  // Populate the lookup table which maps XMLTV IDs to display names
  for (const XMLElement *element = m_content->FirstChildElement("channel");
    element != NULL; element = element->NextSiblingElement("channel"))
  {
    std::string id = Utilities::UrlDecode(element->Attribute("id"));
    std::string displayName = element->FirstChildElement("display-name")->GetText();

    AddDisplayNameMapping(displayName, id);
  }

  for (const XMLElement *element = m_content->FirstChildElement("programme");
    element != NULL; element = element->NextSiblingElement("programme"))
  {
    // Extract the channel name and the programme
    std::string channelName = Utilities::UrlDecode(element->Attribute("channel"));
    xmltv::ProgrammePtr programme(new Programme(element));

    // Add the programme to the guide
    AddProgramme(channelName, std::move(programme));
  }
}

const Schedule* Guide::GetSchedule(const std::string &channelId) const
{
  auto it = m_schedules.find(channelId);

  if (it != m_schedules.cend())
    return it->second.get();

  return nullptr;
}

const Programme* Guide::GetProgramme(int programmeUniqueId) const
{
  for (const auto &entry : m_schedules)
  {
    const ::xmltv::SchedulePtr &schedule = entry.second;

    auto it = std::find_if(
      schedule->cbegin(),
      schedule->cend(),
      [programmeUniqueId](const ProgrammePtr &programme)
    {
      return programmeUniqueId == vbox::ContentIdentifier::GetUniqueId(programme.get());
    });

    if (it != schedule->cend())
      return it->get();
  }

  return nullptr;
}

void Guide::AddProgramme(const std::string &channelId, ProgrammePtr programme)
{
  if (m_schedules.find(channelId) == m_schedules.end())
    AddSchedule(channelId, SchedulePtr(new Schedule);

  m_schedules[channelId]->push_back(std::move(programme));
}
