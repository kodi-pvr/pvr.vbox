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
#include <vector>

namespace tinyxml2 {
  class XMLElement;
}

namespace xmltv {
  class Utilities
  {
  public:

    /**
      * The XMLTV datetime format string
      */
    static const char* XMLTV_DATETIME_FORMAT;

    /**
     * The XMLTV datetime timezone offset format
     */
    static const char* XMLTV_TIMEZONE_OFFSET_FORMAT;

    /**
     * Returns the timezone offset part of the specified XMLTV timestamp
     * @param timestamp an XMLTV timestamp
     * @return the timezone offset, e.g. "+0200", or an empty string if no 
     * timezone offset could be parsed.
     */
    static std::string GetTimezoneOffset(const std::string timestamp);

    /**
     * Converts the specified timezone offset into the number of seconds it 
     * differs from UTC
     * @param tzOffset the timezone offset, e.g. "+0200"
     * @return the number of seconds to adjust
     */
    static int GetTimezoneAdjustment(const std::string tzOffset);

    /**
      * Converts an XMLTV datetime string into a UTC UNIX timestamp
      * @param time e.g. "20120228001500+0200"
      * @return a UTC UNIX timestamp
      */
    static time_t XmltvToUnixTime(const std::string &time);

    /**
      * Converts a UTC time_t to an XMLTV datetime string, optionally adjusted 
      * for the specified timezone offset
      * @param time a UNIX timestamp
      * @param tzOffset the timezone offset, e.g. "+0200"
      * @return e.g. "20120228001500+0200"
      */
    static std::string UnixTimeToXmltv(const time_t timestamp, const std::string tzOffset = "");

    /**
     * Parses the contents of the specified element into an integer. We need 
     * this for backward-compatibility with older versions of tinyxml2.
     */
    static int QueryIntText(const tinyxml2::XMLElement *element);

    /**
     * Parses the contents of the specified element into an unsigned integer. 
     * We need this for backward-compatibility with older versions of tinyxml2.
     */
    static unsigned int QueryUnsignedText(const tinyxml2::XMLElement *element);

    /**
     * URL-encodes the specified string
     *
     * @param name the string to encode
     * @return the encoded string
     */
    static std::string UrlEncode(const std::string &string);

    /**
      * Decodes the specified URL
      *
      * @param name the string to decode
      * @return the decoded string
      */
    static std::string UrlDecode(const std::string &string);

    /**
     * Concatenates the contents of vector with the specified separator
     * @param vector the vector
     * @param separator the separator
     * @return a concatenated string
     */
    static std::string ConcatenateStringList(const std::vector<std::string> &vector, 
      const std::string &separator = ", ");
  };
}
