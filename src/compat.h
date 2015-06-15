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
#include <sstream>
#include <ctime>
#include <cstdlib>

namespace compat
{
  /**
   * Android doesn't fully support C++11 so std::to_string() is missing
   */
  template<typename T> std::string to_string(const T& value)
  {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }

  /**
   * Android doesn't fully support C++11 so std::stoi() is missing
   */
  template<typename T> int stoi(const T& value)
  {
    std::istringstream iss(value);
    
    int result;
    iss >> result;
    return result;
  }

  /**
   * Android doesn't fully support C++11 so std::stol() is missing, we provide 
   * a limited version which does the job
   */
  template<typename T> long stol(const T& value)
  {
    std::istringstream iss(value);

    long result;
    iss >> result;
    return result;
  }

  /**
   * Converts the specified value to an unsigned int
   */
  template<typename T> unsigned int stoui(const T& value)
  {
    std::istringstream iss(value);

    unsigned int result;
    iss >> result;
    return result;
  }

  /**
   * Android doesn't have timegm() and Windows calls it _mkgmtime()
   * Source: http://linux.die.net/man/3/timegm
   */
  inline time_t timegm(struct tm *tm)
  {
#ifdef _WIN32
    return _mkgmtime(tm);
#else
    time_t ret;
    char *tz;

    tz = getenv("TZ");
    setenv("TZ", "", 1);
    tzset();
    ret = mktime(tm);
    if (tz)
      setenv("TZ", tz, 1);
    else
      unsetenv("TZ");
    tzset();

    return ret;
#endif
  }
}
