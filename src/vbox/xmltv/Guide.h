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
#include <map>
#include "Schedule.h"

namespace vbox {
  namespace xmltv {

    typedef std::map<std::string, xmltv::SchedulePtr> Schedules;

    /**
     * Represents a set of guide data. A guide has many schedules (one per
     * channel) and each schedule has many programmes
     */
    class Guide
    {
    public:

      Guide() {}
      ~Guide() {}

      /**
       * Move constructor, needed since we have members containing unique_ptr's
       */
      Guide(Guide &&other)
      {
        if (this != &other)
        {
          m_schedules = std::move(other.m_schedules);
          m_displayNameMappings = other.m_displayNameMappings;
        }
      }

      /**
       * For combining the other guide into this one
       */
      Guide& operator+= (Guide &other)
      {
        // Add all schedules from the other object
        for (auto &entry : other.GetSchedules())
          AddSchedule(entry.first, std::move(entry.second));

        // Merge the display name mappings
        m_displayNameMappings.insert(
          other.m_displayNameMappings.begin(), 
          other.m_displayNameMappings.end());

        return *this;
      }

      /**
       * @param channelId the channel ID
       * @return the schedule for the specified channel, or nullptr if the
       * channel has no schedule
       */
      const Schedule* GetSchedule(const std::string &channelId) const;

      /**
       * Adds the specified schedule on the specified channel
       * @param channelId the channel name
       * @param schedule the schedule (ownership is taken)
       */
      void AddSchedule(const std::string &channelId, SchedulePtr schedule)
      {
        m_schedules[channelId] = std::move(schedule);
      }

      /**
       * Adds the specified programme to the specified channel's schedule
       * @param channelId the channel name
       * @param programme the program (ownership is taken)
       */
      void AddProgramme(const std::string &channelId, ProgrammePtr programme);

      /**
       * Adds a new mapping between the display name and the channel ID
       * @param displayName the display name
       * @param channelId the channel ID
       */
      void AddDisplayNameMapping(const std::string &displayName, const std::string &channelId)
      {
        m_displayNameMappings[displayName] = channelId;
      }

      /**
       * @param displayName the display name
       * @return the corresponding channel ID for specified display name
       */
      std::string GetChannelId(const std::string &displayName) const
      {
        auto it = m_displayNameMappings.find(displayName);

        if (m_displayNameMappings.find(displayName) == m_displayNameMappings.cend())
          return "";

        return it->first;
      }

      /**
       * @return the schedules
       */
      Schedules& GetSchedules()
      {
        return m_schedules;
      }

      /**
       * @return the schedules
       */
      const Schedules& GetSchedules() const
      {
        return m_schedules;
      }

    private:

      /**
       * The schedules
       */
      Schedules m_schedules;

      /**
       * Maps a display name to an XMLTV channel ID
       */
      std::map<std::string, std::string> m_displayNameMappings;

    };
  }
}

