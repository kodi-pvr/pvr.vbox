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

  typedef std::priority_queue<ReminderPtr, std::vector<ReminderPtr>, ReminderComparison> ReminderQueue;
  typedef std::unique_ptr<ReminderQueue> ReminderQueuePtr;

  class ReminderManager
  {
  public:
    ReminderManager() = default;
    ~ReminderManager() = default;
    void Initialize();
    bool AddReminder(const ChannelPtr &channel, const ::xmltv::ProgrammePtr &programme, unsigned int minsBeforePop);
    bool AddReminder(const ChannelPtr &channel, time_t startTime, std::string &progName, unsigned int minsBeforePop);
    ReminderPtr GetReminderToPop(time_t currTime);
    void KillNextReminder();
    bool KillChannelReminders(const ChannelPtr &channel);
    bool KillProgramReminders(unsigned int epgUid);
    void Load();
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
