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

  class Reminder
  {
  public:
    Reminder(const ChannelPtr &channel, const ::xmltv::ProgrammePtr &programme, unsigned int minsInAdvance);
    Reminder(const ChannelPtr &channel, time_t startTime, std::string &progName, unsigned int minsInAdvance);
    ~Reminder() = default;
    std::string GetReminderText(time_t currTime);
    time_t GetPopTime() const;
    time_t GetStartTime() const;

  private:
    friend ReminderManager;

    static unsigned int FindChannelNumber(const ChannelPtr &channel);
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

  class ReminderComparison
  {
    bool reverse;
  public:
    ReminderComparison(const bool& revparam = true)
    {
      reverse = revparam;
    }
    bool operator() (ReminderPtr &lhs, ReminderPtr &rhs) const
    {
      if (reverse) 
        return (lhs->GetPopTime() > rhs->GetPopTime());
      else 
        return (rhs->GetPopTime() > lhs->GetPopTime());
    }
  };
}
