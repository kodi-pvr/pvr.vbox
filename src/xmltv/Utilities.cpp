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

#include "Utilities.h"
#include "../compat.h"
#include "lib/tinyxml2/tinyxml2.h"
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <iomanip>

using namespace xmltv;

const char* Utilities::XMLTV_DATETIME_FORMAT = "%Y%m%d%H%M%S";
const char* Utilities::XMLTV_TIMEZONE_OFFSET_FORMAT = "%03d%02d";

std::string Utilities::GetTimezoneOffset(const std::string timestamp)
{
  std::string xmltvTime = timestamp;
  std::string tzOffset = "";

  // Make sure the timestamp doesn't contain a space before the timezone offset
  xmltvTime.erase(std::remove_if(xmltvTime.begin(), xmltvTime.end(), isspace), xmltvTime.end());

  if (xmltvTime.length() > 14)
    tzOffset = xmltvTime.substr(14);

  return tzOffset;
}

int Utilities::GetTimezoneAdjustment(const std::string tzOffset)
{
  // Sanity check
  if (tzOffset.length() != 5)
    return 0;

  int hours = 0;
  int minutes = 0;

  sscanf(tzOffset.c_str(), XMLTV_TIMEZONE_OFFSET_FORMAT, &hours, &minutes);

  // Make minutes negative if hours is
  if (hours < 0)
    minutes = -minutes;

  return (hours * 3600) + (minutes * 60);
}

time_t Utilities::XmltvToUnixTime(const std::string &time)
{
  std::tm timeinfo;

  // Convert the timestamp, disregarding the timezone offset
  sscanf(time.c_str(), "%04d%02d%02d%02d%02d%02d",
    &timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday,
    &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec);

  timeinfo.tm_year -= 1900;
  timeinfo.tm_mon -= 1;
  timeinfo.tm_isdst = -1;

  time_t unixTime = compat::timegm(&timeinfo);

  // Adjust for eventual timezone offset
  std::string tzOffset = GetTimezoneOffset(time);

  if (!tzOffset.empty())
    unixTime -= GetTimezoneAdjustment(tzOffset);

  return unixTime;
}

std::string Utilities::UnixTimeToXmltv(const time_t timestamp,
  const std::string tzOffset /* = ""*/)
{
  // Adjust the timestamp according to the timezone
  time_t adjustedTimestamp = timestamp + GetTimezoneAdjustment(tzOffset);

  // Format the timestamp
  std::tm tm = *std::gmtime(&adjustedTimestamp);

  char buffer[20];
  strftime(buffer, sizeof(buffer), XMLTV_DATETIME_FORMAT, &tm);

  std::string xmltvTime(buffer);

  // Append the timezone offset
  if (tzOffset.empty())
    xmltvTime += "+0000";
  else
    xmltvTime += tzOffset;

  return xmltvTime;
}

// Borrowed from https://github.com/xbmc/xbmc/blob/master/xbmc/URL.cpp
std::string Utilities::UrlDecode(const std::string& strURLData)
{
  std::string strResult;
  /* result will always be less than source */
  strResult.reserve(strURLData.length());
  for (unsigned int i = 0; i < strURLData.size(); ++i)
  {
    int kar = static_cast<unsigned char>(strURLData[i]);
    if (kar == '+') strResult += ' ';
    else if (kar == '%')
    {
      if (i < strURLData.size() - 2)
      {
        std::string strTmp;
        strTmp.assign(strURLData.substr(i + 1, 2));
        int dec_num = -1;
        sscanf(strTmp.c_str(), "%x", reinterpret_cast<unsigned int *>(&dec_num));
        if (dec_num < 0 || dec_num>255)
          strResult += kar;
        else
        {
          strResult += static_cast<char>(dec_num);
          i += 2;
        }
      }
      else
        strResult += kar;
    }
    else strResult += kar;
  }
  return strResult;
}

int Utilities::QueryIntText(const tinyxml2::XMLElement *element)
{
  int value = 0;

  if (element->GetText())
  {
    try {
      std::string content = element->GetText();
      value = compat::stoi(content);
    }
    catch (std::invalid_argument) {

    }
  }

  return value;
}

unsigned int Utilities::QueryUnsignedText(const tinyxml2::XMLElement *element)
{
  unsigned int value = 0;

  if (element->GetText())
  {
    try {
      std::string content = element->GetText();
      value = compat::stoui(content);
    }
    catch (std::invalid_argument) {

    }
  }

  return value;
}

// Adapted from http://stackoverflow.com/a/17708801
std::string Utilities::UrlEncode(const std::string &value)
{
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (auto i = value.cbegin(), n = value.cend(); i != n; ++i)
  {
    std::string::value_type c = (*i);

    // Keep alphanumeric and other accepted characters intact
    if (c < 0 || isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
    {
      escaped << c;
      continue;
    }

    // Any other characters are percent-encoded
    escaped << '%' << std::setw(2) << int(static_cast<unsigned char>(c));
  }

  return escaped.str();
}

// Borrowed from http://stackoverflow.com/a/8581865
std::string Utilities::ConcatenateStringList(const std::vector<std::string> &vector,
  const std::string &separator /* = ", "*/)
{
  std::ostringstream oss;

  if (!vector.empty())
  {
    std::copy(vector.begin(), vector.end() - 1,
      std::ostream_iterator<std::string>(oss, separator.c_str()));

    oss << vector.back();
  }

  return oss.str();
}
