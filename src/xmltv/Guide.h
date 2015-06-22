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
#include <vector>
#include "Schedule.h"
#include "Programme.h"

// Forward declarations
namespace tinyxml2
{
  class XMLElement;
}

namespace xmltv {

  typedef std::map<std::string, xmltv::SchedulePtr> Schedules;

  /**
    * Represents a set of guide data. A guide has many schedules (one per
    * channel) and each schedule has many programmes
    */
  class Guide
  {
  public:

    Guide() = default;
    ~Guide() = default;

    /**
      * Creates a guide from the specified XMLTV contents
      */
    explicit Guide(const tinyxml2::XMLElement *m_content);

    /**
      * For combining the other guide into this one
      */
    Guide& operator+= (Guide &other)
    {
      // Add all schedules from the other object
      for (auto &entry : other.m_schedules)
        AddSchedule(entry.first, entry.second);

      // Merge the display name mappings
      m_displayNameMappings.insert(
        other.m_displayNameMappings.begin(), 
        other.m_displayNameMappings.end());

      return *this;
    }

    /**
     * Adds the specified schedule on the specified channel
     * @param channelId the channel name
     * @param schedule the schedule (ownership is taken)
     */
    void AddSchedule(const std::string &channelId, SchedulePtr schedule)
    {
      m_schedules[channelId] = schedule;
    }

    /**
      * @param channelId the channel ID
      * @return the schedule for the specified channel, or nullptr if the
      * channel has no schedule
      */
    const SchedulePtr GetSchedule(const std::string &channelId) const;

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
     * Maps a channel display name to its XMLTV name (case-insensitive)
     * @param displayName the display name
     * @return the corresponding channel ID for specified display name
     */
    std::string GetChannelId(const std::string &displayName) const;

    /**
     * @return all the channel names in the guide
     */
    std::vector<std::string> GetChannelNames() const
    {
      std::vector<std::string> channelNames;

      for (const auto &mapping : m_displayNameMappings)
        channelNames.push_back(mapping.first);

      return channelNames;
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
