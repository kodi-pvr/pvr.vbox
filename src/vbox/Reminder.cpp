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
#include "ContentIdentifier.h"
#include "Reminder.h"
#include <algorithm>
#include "lib/tinyxml2/tinyxml2.h"
#include "p8-platform/util/StringUtils.h"
#include "Utilities.h"
#include "Exceptions.h"
#include "../client.h"

using namespace vbox;
using namespace tinyxml2;

unsigned int Reminder::FindChannelNumber(const ChannelPtr &channel)
{
  if (g_vbox->GetSettings().m_setChannelIdUsingOrder == CH_ORDER_BY_LCN)
  {
     return channel->m_number;
  }
  else
  {
    auto &channels = g_vbox->GetChannels();
    unsigned int i = 0;

    for (const auto &item : channels)
    {
      ++i;
      if (item == channel)
        break;
    }
    return i;
  }
}

Reminder::Reminder(const ChannelPtr &channel, const ::xmltv::ProgrammePtr &programme, unsigned int minsInAdvance) :
  m_minsInAdvance(minsInAdvance), m_startTime(xmltv::Utilities::XmltvToUnixTime(programme->m_startTime)),
  m_popTime(xmltv::Utilities::XmltvToUnixTime(programme->m_startTime) - (60 * m_minsInAdvance)), m_progName(programme->m_title), 
  m_channelName(channel->m_name), m_channelXmltvName(channel->m_xmltvName)
{
  m_channelNum = FindChannelNumber(channel);
}

Reminder::Reminder(const ChannelPtr &channel, time_t startTime, std::string &progName, unsigned int minsInAdvance) :
  m_minsInAdvance(minsInAdvance), m_startTime(startTime), 
  m_popTime(startTime - (60 * m_minsInAdvance)), m_progName(progName),
  m_channelName(channel->m_name), m_channelXmltvName(channel->m_xmltvName)
{
  m_channelNum = FindChannelNumber(channel);
}

void Reminder::ComposeMessage(time_t currTime)
{
  char buf[32], minBuf[32];

  memset(minBuf, 0, sizeof(buf));
  
  sprintf(buf, "[%u] ", m_channelNum);

  m_msgTitle = "Program reminder:";
  m_msgText = "Program: " + std::string("    ") + m_progName + '\n';
  m_msgText += "Channel: " + std::string("    ") +  std::string(buf) + m_channelName + '\n';
  unsigned int minutes = (m_startTime - currTime) / 60;

  m_msgText += "Starts ";

  if (currTime < m_startTime && minutes > 0)
  {
    sprintf(minBuf, "%li", (m_startTime - currTime) / 60);
    m_msgText += "in:     " + std::string(minBuf) + " minutes";
  }
  else
  {
    m_msgText += ":        Now";
  }
}

std::string Reminder::GetReminderText()
{
  ComposeMessage(time(nullptr));
  return m_msgText;
}

time_t Reminder::GetPopTime() const
{
  return m_popTime;
}

time_t Reminder::GetStartTime() const
{
  return m_startTime;
}