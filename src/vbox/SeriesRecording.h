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
#include <memory>

namespace vbox {

  class SeriesRecording;
  typedef std::unique_ptr<SeriesRecording> SeriesRecordingPtr;

  /**
  * Represents a series
  */
  class SeriesRecording
  {
  public:
    SeriesRecording(const std::string &channelId);
    ~SeriesRecording() = default;

    bool operator== (const SeriesRecording &other)
    {
      return m_id == other.m_id &&
        m_scheduledId == other.m_scheduledId &&
        m_channelId == other.m_channelId &&
        m_title == other.m_title &&
        m_description == other.m_description &&
        m_startTime == other.m_startTime &&
        m_endTime == other.m_endTime;
    }

    bool operator!= (const SeriesRecording &other)
    {
      return !(*this == other);
    }

    unsigned int m_id;
    unsigned int m_scheduledId;
    std::string m_channelId;
    std::string m_title;
    std::string m_description;
    bool m_fIsAuto;
    std::string m_startTime;
    std::string m_endTime;
    unsigned int m_weekdays;
  };

}