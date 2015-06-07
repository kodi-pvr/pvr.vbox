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

#include "Programme.h"
#include "Channel.h"
#include <vector>
#include <memory>

// Visual Studio can't handle type names longer than 255 characters in debug 
// mode, disable that warning since it's not important
#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif

namespace xmltv {

  class Schedule;
  typedef std::shared_ptr<Schedule> SchedulePtr;
  typedef std::vector<ProgrammePtr> Segment;

  /**
   * Represents a schedule for a channel
   */
  class Schedule
  {
  public:

    /**
     * Creates a new schedule
     * @param the channel this schedule is for
     */
    Schedule(ChannelPtr &channel);

    /**
     * Adds the specified programme to the specified channel's schedule
     * @param programme a programme
     */
    void AddProgramme(ProgrammePtr programme);

    /**
     * @param programmeUniqueId the unique ID of the programme
     * @return the programme, or nullptr if not found
     */
    const ProgrammePtr GetProgramme(int programmeUniqueId) const;

    /**
     * Returns a schedule segment containing all programmes between the 
     * specified timestamps
     * @param startTime the start time
     * @param endTime the end time
     * @return list of matching programmes
     */
    const Segment GetSegment(time_t startTime, time_t endTime) const;

    /**
     * @return the channel this schedule is for
     */
    const ChannelPtr GetChannel() const
    {
      return m_channel;
    }

    /**
     * @return the number of programmes in the schedule
     */
    size_t GetLength() const
    {
      return m_programmes.size();
    }

  private:
    Segment m_programmes;
    ChannelPtr m_channel;
  };
}
