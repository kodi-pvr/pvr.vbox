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
    Recording(unsigned int id, const std::string &channelId, 
      const std::string &channelName, RecordingState state);
    ~Recording();

    /**
     * Whether this object represents a timer
     * @return true if timer
     */
    bool IsTimer() const
    {
      return m_state == RecordingState::SCHEDULED;
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
    std::string m_title;
    time_t m_start;
    time_t m_end;

  private:
    RecordingState m_state;
  };
}
