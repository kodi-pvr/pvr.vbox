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

namespace vbox
{
  /**
  * Represents a software version
  */
  class SoftwareVersion
  {
  public:

    bool operator== (const SoftwareVersion &other) const
    {
      return m_major == other.m_major &&
        m_minor == other.m_minor &&
        m_revision == other.m_revision;
    }

    bool operator> (const SoftwareVersion &other) const
    {
      return m_major > other.m_major || 
        m_minor > other.m_minor ||
        m_revision > other.m_revision;
    }

    bool operator< (const SoftwareVersion &other) const
    {
      return !(*this > other) && !(*this == other);
    }

    bool operator>= (const SoftwareVersion &other) const
    {
      return !(*this < other);
    }

    bool operator<= (const SoftwareVersion &other) const
    {
      return !(*this > other);
    }
    
    /**
     * @return the software version as a string
     */
    std::string GetString() const;

    /**
     * @return a SoftwareVersion object representing the specified version
     * string. The version string should either be e.g. "2.46.20" or "VB.2.46.20"
     */
    static SoftwareVersion ParseString(const std::string &string);

  private:
    unsigned int m_major = 0;
    unsigned int m_minor = 0;
    unsigned int m_revision = 0;
  };
}
