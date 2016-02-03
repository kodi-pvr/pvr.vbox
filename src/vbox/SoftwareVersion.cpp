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

#include "SoftwareVersion.h"
#include "../compat.h"

using namespace vbox;

std::string SoftwareVersion::GetString() const
{
  return compat::to_string(m_major) + "." +
    compat::to_string(m_minor) + "." +
    compat::to_string(m_revision);
}

SoftwareVersion SoftwareVersion::ParseString(const std::string &string)
{
  SoftwareVersion version;
  std::string format = "%d.%d.%d";
  
  if (string.substr(0, 1) == "V")
    format = string.substr(0, 2) + ".%d.%d.%d";

  sscanf(string.c_str(), format.c_str(), &version.m_major, &version.m_minor,
    &version.m_revision);

  return version;
}
