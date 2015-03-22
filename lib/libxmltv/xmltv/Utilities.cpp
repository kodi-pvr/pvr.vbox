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
#include <algorithm>
#include <iterator>

using namespace xmltv;

const char* Utilities::XMLTV_DATETIME_FORMAT = "%Y%m%d%H%M%S";

// Borrowed from https://github.com/xbmc/xbmc/blob/master/xbmc/URL.cpp
std::string Utilities::UrlDecode(const std::string& strURLData)
{
  std::string strResult;
  /* result will always be less than source */
  strResult.reserve(strURLData.length());
  for (unsigned int i = 0; i < strURLData.size(); ++i)
  {
    int kar = (unsigned char)strURLData[i];
    if (kar == '+') strResult += ' ';
    else if (kar == '%')
    {
      if (i < strURLData.size() - 2)
      {
        std::string strTmp;
        strTmp.assign(strURLData.substr(i + 1, 2));
        int dec_num = -1;
        sscanf(strTmp.c_str(), "%x", (unsigned int *)&dec_num);
        if (dec_num < 0 || dec_num>255)
          strResult += kar;
        else
        {
          strResult += (char)dec_num;
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
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
    {
      escaped << c;
      continue;
    }

    // Any other characters are percent-encoded
    escaped << '%' << std::setw(2) << int((unsigned char)c);
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
