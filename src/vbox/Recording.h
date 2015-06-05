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

  /**
   * The possible states a recording can be in
   */
  enum RecordingState {
    SCHEDULED,
    RECORDED,
    RECORDING,
    RECORDING_ERROR, // Can't use just ERROR because of Windows
    EXTERNAL
  };

  class Recording;
  typedef std::unique_ptr<Recording> RecordingPtr;

  /**
   * Represents a recording
   */
  class Recording
  {
  public:
    Recording(const std::string &channelId, 
      const std::string &channelName, RecordingState state);
    ~Recording();

    bool operator== (const Recording &other)
    {
      return m_id == other.m_id &&
        m_channelId == other.m_channelId &&
        m_channelName == other.m_channelName &&
        m_url == other.m_url &&
        m_title == other.m_title &&
        m_description == other.m_description &&
        m_startTime == other.m_startTime &&
        m_endTime == other.m_endTime &&
        m_state == other.m_state;
    }

    bool operator!= (const Recording &other)
    {
      return !(*this == other);
    }

    /**
     * Whether this object represents a timer
     * @return true if timer
     */
    bool IsTimer() const
    {
      return m_state == RecordingState::SCHEDULED || 
        m_state == RecordingState::RECORDING;
    }

    /**
     * Whether this object represents a recording
     * @return true if recording
     */
    bool IsRecording() const
    {
      return m_state == RecordingState::RECORDED || 
        m_state == RecordingState::RECORDING ||
        m_state == RecordingState::RECORDING_ERROR ||
        m_state == RecordingState::EXTERNAL;
    }

    /**
     * Returns the state of the recording
     * @return the state
     */
    RecordingState GetState() const
    {
      return m_state;
    }

    unsigned int m_id;
    std::string m_channelId;
    std::string m_channelName;
    std::string m_url;
    std::string m_filename;
    std::string m_title;
    std::string m_description;
    std::string m_startTime;
    std::string m_endTime;

  private:
    RecordingState m_state;
  };
}
