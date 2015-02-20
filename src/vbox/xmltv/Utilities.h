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

#include <ctime>
#include <string>

namespace vbox {
  namespace xmltv {
    class Utilities
    {
    public:

      /**
       * Converts an XMLTV datetime string to time_t
       * @param time e.g. "20120228001500 +0200"
       * @return a UNIX timestamp
       */
      static time_t ConvertXMLTVTime(const std::string &time)
      {
        struct tm timeinfo;
        int timezoneHours, timezoneMinutes;

        sscanf(time.c_str(), "%04d%02d%02d%02d%02d%02d %02d%02d",
          &timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday, 
          &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec, 
          &timezoneHours, &timezoneMinutes);

        timeinfo.tm_hour -= 1;
        timeinfo.tm_year -= 1900;
        time_t timestamp = mktime(&timeinfo);
        
        // Correct for timezone
        if (timezoneHours < 0)
          timezoneMinutes *= -1;

        timestamp += timezoneHours * 3600;
        timestamp += timezoneMinutes * 60;

        return timestamp;
      }
    };
  }
}
