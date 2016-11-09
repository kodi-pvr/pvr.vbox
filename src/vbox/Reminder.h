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
#include "Channel.h"
#include "../xmltv/Programme.h"
#include "xbmc_pvr_types.h"

namespace vbox {

  class ReminderManager;

  /**
  * Represents a single-program reminder. 
  * Contains a message reminding the user of the program
  */
  class Reminder
  {
  public:
    
    /**
    * Creates a reminder from a channel and a specific program
    * @param channel the channel containing the program to remind
    * @param programme the program to remind
    * @param minsInAdvance minutes before the program's start time to pop the reminder
    */
    Reminder(const ChannelPtr &channel, const ::xmltv::ProgrammePtr &programme, unsigned int minsInAdvance);
    
    /**
    * Creates a reminder according to a manually given program name and its' start time
    * @param channel the channel containing the program to remind
    * @param startTime the program's original start time
    * @param minsInAdvance minutes before the program's start time to pop the reminder
    */
    Reminder(const ChannelPtr &channel, time_t startTime, std::string &progName, unsigned int minsInAdvance);
    
    /**
    * For comparing reminders' pop times
    */
    bool operator< (const Reminder &other) const
    {
      return !(m_popTime < other.m_popTime);
    }

    /**
    * Composes & returns a message for a certain moment in time, showing details 
    * of the program and the time left for it to start
    * @param currTime the current time of showing the reminder pop-up
    * @return the reminder's pop-up message
    */
    std::string GetReminderText();

    /**
    * @return the reminder's pop time
    */
    time_t GetPopTime() const;

    /**
    * @return the program's original start time
    */
    time_t GetStartTime() const;

  private:
    friend ReminderManager;

    /**
    * Finds a channel's display number in the addon (LCN / index - varies by setting)
    * @param channel the requested channel
    * @return the channel's number
    */
    static unsigned int FindChannelNumber(const ChannelPtr &channel);

    /**
    * Composes the reminder's message
    * @param currTime the time of showing the popup (pop time)
    */
    void ComposeMessage(time_t currTime);

    unsigned int m_minsInAdvance;
    time_t m_startTime;
    time_t m_popTime;
    std::string m_channelXmltvName;
    unsigned int m_channelNum;
    std::string m_channelName;
    std::string m_progName;
    std::string m_msgTitle;
    std::string m_msgText;
  };

  typedef std::shared_ptr<Reminder> ReminderPtr;
}
