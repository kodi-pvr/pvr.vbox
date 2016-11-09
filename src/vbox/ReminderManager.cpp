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
#include "ReminderManager.h"
#include <algorithm>
#include "lib/tinyxml2/tinyxml2.h"
#include "p8-platform/util/StringUtils.h"
#include "Utilities.h"
#include "Exceptions.h"
#include "../client.h"

using namespace vbox;
using namespace tinyxml2;

const std::string ReminderManager::REMINDERS_XML = "special://userdata/addon_data/pvr.vbox/reminders.xml";

void ReminderManager::Initialize()
{
  if (!XBMC->FileExists(REMINDERS_XML.c_str(), false))
  {
    g_vbox->Log(ADDON::LOG_INFO, "No reminders XML found");
    Save();
  }
  else
  {
    g_vbox->Log(ADDON::LOG_INFO, "Reminders XML found");
    Load();
  }
}

bool ReminderManager::AddReminder(const ChannelPtr &channel, const ::xmltv::ProgrammePtr &programme, unsigned int minsBeforePop)
{
  // Construct the object
  ReminderPtr reminder(new Reminder(channel, programme, minsBeforePop));
  // save in queue
  g_vbox->Log(ADDON::LOG_DEBUG, "Added reminder (1) for channel %s, prog %s", programme->m_channelName.c_str(), programme->m_title.c_str());
  m_reminders.push(reminder);
  Save();
  return true;
}

bool ReminderManager::AddReminder(const ChannelPtr &channel, time_t startTime, std::string &progName, unsigned int minsBeforePop)
{
  g_vbox->Log(ADDON::LOG_DEBUG, "Added reminder for %s", g_vbox->CreateTimestamp(startTime).c_str());
  // Construct the object
  ReminderPtr reminder(new Reminder(channel, startTime, progName, minsBeforePop));
  // save in queue
  g_vbox->Log(ADDON::LOG_DEBUG, "Added reminder (2) for channel %s, prog %s", channel->m_name.c_str(), progName.c_str());
  m_reminders.push(reminder);
  Save();
  return true;
}

ReminderPtr ReminderManager::GetReminderToPop(time_t currTime)
{
  if (m_reminders.empty())
    return nullptr;

  ReminderPtr reminder = m_reminders.top();
  if (reminder)
  {
    time_t popTime = reminder->GetPopTime();
    time_t startTime = reminder->GetStartTime();

    // if past pop time - handle reminder
    if (currTime > popTime)
    {
      // if we're somewhere between the pop time and the first
      // 5 first minutes of the program (addon might have not been active) - pop reminder
      if (currTime < startTime + 5 * 60)
      {
        g_vbox->Log(ADDON::LOG_DEBUG, "Reminder popped");
        return reminder;
      }
      // reminder is too old (either popped or past the first 5 minutes of program) - delete it
      else
        DeleteNextReminder();
    }
  }
  return nullptr;
}

void ReminderManager::DeleteNextReminder()
{
  g_vbox->Log(ADDON::LOG_DEBUG, "Removing reminder!");
  m_reminders.pop();
  Save();
}

bool ReminderManager::DeleteChannelReminders(const ChannelPtr &rChannel)
{
  bool fSuccess = false;
  ReminderQueue queue;

  while (!m_reminders.empty())
  {
    ReminderPtr reminder = m_reminders.top();
    m_reminders.pop();
    std::string channelId = reminder->m_channelXmltvName;
    // find matching channel
    auto &channels = g_vbox->GetChannels();
    auto it = std::find_if(channels.cbegin(), channels.cend(),
      [&channelId](const ChannelPtr &channel)
    {
      return channelId == channel->m_xmltvName;
    });

    // if channel does not match - keep reminder
    if (it != channels.end())
    {
      const ChannelPtr &selectedChannel = *it;
      if (rChannel == selectedChannel)
      {
        g_vbox->Log(ADDON::LOG_INFO, "Removing reminder, matches channel %s", selectedChannel->m_xmltvName.c_str());
        fSuccess = true;
        continue;
      }
    }
    queue.push(reminder);
  }
  m_reminders = queue;
  if (fSuccess)
    Save();
  return fSuccess;
}


bool ReminderManager::DeleteProgramReminders(unsigned int epgUid)
{
  bool fSuccess = false;
  ReminderQueue queue;

  g_vbox->Log(ADDON::LOG_INFO, "KillProgramReminders(): in");
  while (!m_reminders.empty())
  {
    ReminderPtr reminder = m_reminders.top();
    m_reminders.pop();
    std::string channelId = reminder->m_channelXmltvName;

    auto &channels = g_vbox->GetChannels();
    auto it = std::find_if(channels.cbegin(), channels.cend(),
      [&channelId](const ChannelPtr &channel)
    {
      return channelId == channel->m_xmltvName;
    });

    // if channel does not match - keep reminder & continue
    if (it != channels.end())
    {
      const ChannelPtr &selectedChannel = *it;
      const Schedule schedule = g_vbox->GetSchedule(selectedChannel);
      const xmltv::ProgrammePtr programme = (schedule.schedule) ? schedule.schedule->GetProgramme(epgUid) : nullptr;
      // skip reminder if the EPG event is found
      if (programme && programme->m_title == reminder->m_progName && xmltv::Utilities::XmltvToUnixTime(programme->m_startTime) == reminder->m_startTime)
      {
        fSuccess = true;
        continue;
      }
    }
    queue.push(reminder);
  }
  m_reminders = queue;
  if (fSuccess)
    Save();
  return fSuccess;
}


void ReminderManager::Load()
{
  g_vbox->Log(ADDON::LOG_INFO, "Found reminders XML file, attempting to load it");
  void *fileHandle = XBMC->OpenFile(REMINDERS_XML.c_str(), 0x08 /* READ_NO_CACHE */);

  if (!fileHandle)
  {
    g_vbox->Log(ADDON::LOG_ERROR, "Could not open reminders XML, throwing exception");
    throw vbox::InvalidXMLException("XML could not be opened" );
  }

  m_reminders = ReminderQueue();

  g_vbox->Log(ADDON::LOG_INFO, "Reading XML");
  // Read the XML
  tinyxml2::XMLDocument document;
  std::unique_ptr<std::string> contents = utilities::ReadFileContents(fileHandle);

  // Try to parse the document
  if (document.Parse(contents->c_str(), contents->size()) != XML_NO_ERROR)
    throw vbox::InvalidXMLException("XML parsing failed: " + std::string(document.ErrorName()));

  unsigned int minsBeforePop = g_vbox->GetSettings().m_remindMinsBeforeProg;
  // Create mappings
  for (const XMLElement *element = document.RootElement()->FirstChildElement("reminder");
    element != nullptr; element = element->NextSiblingElement("reminder"))
  {
    g_vbox->Log(ADDON::LOG_INFO, "Found reminder");
    // get channel, program name and start time
    const char *pXmltvId = element->Attribute("channel");
    const char *pStartTime = element->Attribute("start-time");
    const char *pProgName = element->GetText();
    g_vbox->Log(ADDON::LOG_INFO, "Reminder  1 is for ch %s, startTime %s", pXmltvId, pStartTime);
    std::string progTitle(pProgName? pProgName : "");
    g_vbox->Log(ADDON::LOG_INFO, "Reminder 2 is for ch %s, startTime %s, progTitle=%s", pXmltvId, pStartTime, progTitle.c_str());
    if (!pXmltvId || !pStartTime)
      continue;

    g_vbox->Log(ADDON::LOG_INFO, "Reminder 3 is for ch %s, startTime %s", pXmltvId, pStartTime);
    std::string encodedChId(pXmltvId);
    std::string xmltvStartTime(pStartTime);
    time_t startTime(xmltv::Utilities::XmltvToUnixTime(xmltvStartTime));
    g_vbox->Log(ADDON::LOG_INFO, "Reminder is for encodedChId %s, looking for it", encodedChId.c_str());
    // Find the channel the reminder is for
    auto &channels = g_vbox->GetChannels();
    auto it = std::find_if(channels.cbegin(), channels.cend(),
      [&encodedChId](const ChannelPtr &channel)
    {
      g_vbox->Log(ADDON::LOG_INFO, "Channel is %s when looking for %s", channel->m_xmltvName.c_str(), encodedChId.c_str());
      return encodedChId == channel->m_xmltvName;
    });

    if (it == channels.end())
    {
      g_vbox->Log(ADDON::LOG_INFO, "Channel of reminder not found, continuing");
      continue;
    }
    const ChannelPtr channel = *it;

    g_vbox->Log(ADDON::LOG_INFO, "Channel found, it's %s, adding reminder to queue", channel->m_xmltvName.c_str());
    if (!AddReminder(channel, startTime, progTitle, minsBeforePop))
      g_vbox->Log(ADDON::LOG_ERROR, "Could not load reminder");
    else
      g_vbox->Log(ADDON::LOG_INFO, "Channel found, it's %s, added reminder to queue", channel->m_xmltvName.c_str());
  }
  XBMC->CloseFile(fileHandle);
}

void ReminderManager::Save()
{
  ReminderQueue queue;

  // Create the document
  tinyxml2::XMLDocument document;
  XMLDeclaration *declaration = document.NewDeclaration();
  document.InsertEndChild(declaration);

  // Create the root node (<reminders>)
  XMLElement *rootElement = document.NewElement("reminders");
  document.InsertEndChild(rootElement);
  g_vbox->Log(ADDON::LOG_INFO, "Save(1): %u reminders", m_reminders.size());

  // Create one <reminder> for every reminder
  while (!m_reminders.empty())
  {
    ReminderPtr reminder = m_reminders.top();
    g_vbox->Log(ADDON::LOG_INFO, "Save(2): got reminder", m_reminders.size());
    XMLElement *reminderElement = document.NewElement("reminder");
    reminderElement->SetText(reminder->m_progName.c_str());
    reminderElement->SetAttribute("channel", reminder->m_channelXmltvName.c_str());
    reminderElement->SetAttribute("start-time", g_vbox->CreateTimestamp(reminder->m_startTime).c_str());
    rootElement->InsertFirstChild(reminderElement);
    m_reminders.pop();
    g_vbox->Log(ADDON::LOG_INFO, "Save(3): popped. Now pushing to queue", m_reminders.size());
    queue.push(reminder);
  }
  g_vbox->Log(ADDON::LOG_INFO, "Save(4): queue size %d, m_reminders size %d", queue.size(), m_reminders.size());
  m_reminders = queue;
  g_vbox->Log(ADDON::LOG_INFO, "Save(5): queue size %d, m_reminders size %d", queue.size(), m_reminders.size());
  
  XBMC->DeleteFile(REMINDERS_XML.c_str());
  // Save the file
  void *fileHandle = XBMC->OpenFileForWrite(REMINDERS_XML.c_str(), false);

  if (fileHandle)
  {
    XMLPrinter printer;
    document.Accept(&printer);

    //XBMC->TruncateFile()
    std::string xml = printer.CStr();
    XBMC->WriteFile(fileHandle, xml.c_str(), xml.length());

    XBMC->CloseFile(fileHandle);
  }
}
