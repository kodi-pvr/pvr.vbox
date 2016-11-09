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
#include <vector>
#include <queue>
#include <functional>
#include "Reminder.h"
#include "../xmltv/Programme.h"
#include "xbmc_pvr_types.h"

namespace vbox {

  typedef std::priority_queue<ReminderPtr, std::vector<ReminderPtr> > ReminderQueue;
  typedef std::unique_ptr<ReminderQueue> ReminderQueuePtr;

  /**
  * Represents a reminder manager, which creates, stores and manages reminders
  * according to the time they're supposed to pop. It stores the reminders to disk
  */
  class ReminderManager
  {
  public:
    ReminderManager() = default;
    ~ReminderManager() = default;

    /**
    * Initializes the manager by loading previously set reminders from disk
    * if no XML exists, saves the current reminders to disk
    */
    void Initialize();

    /**
    * Creates and stores a reminder with a given channel and program
    * @param channel the channel containing the program to remind
    * @param programme the program to remind
    * @param minsInAdvance minutes before the program's start time to pop the reminder
    * @return the success of adding the newly created reminder
    */
    bool AddReminder(const ChannelPtr &channel, const ::xmltv::ProgrammePtr &programme, unsigned int minsBeforePop);
    
    /**
    * Creates and stores a reminder with a given channel, and a manually given program name and its' start time
    * @param channel the channel containing the program to remind
    * @param programme the program to remind
    * @param minsInAdvance minutes before the program's start time to pop the reminder
    * @return the success of adding the newly created reminder
    */
    bool AddReminder(const ChannelPtr &channel, time_t startTime, std::string &progName, unsigned int minsBeforePop);

    /**
    * @param currTime the current time
    * @return the reminder at the top of the queue
    */
    ReminderPtr GetReminderToPop(time_t currTime);

    /**
    * Removes the reminder at the top of the queue
    */
    void DeleteNextReminder();

    /**
    * Removes all reminder set for a channel (if exist)
    */
    bool DeleteChannelReminders(const ChannelPtr &channel);

    /**
    * Removes a program's reminder (if exists)
    */
    bool DeleteProgramReminders(unsigned int epgUid);

    /**
    * Loads reminders from disk
    */
    void Load();

    /**
    * Saves reminders to disk
    */
    void Save();

  private:

    /**
    * The path to the reminders XML file
    */
    const static std::string REMINDERS_XML;

    /**
    * The queue of reminders (prioritized by earliest)
    */
    ReminderQueue m_reminders;
  };

  typedef std::unique_ptr<ReminderManager> ReminderManagerPtr;
}
