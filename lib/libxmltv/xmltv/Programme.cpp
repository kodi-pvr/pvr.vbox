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

#include "Programme.h"
#include "Utilities.h"
#include "tinyxml2.h"

using namespace xmltv;
using namespace tinyxml2;

Programme::Programme(const tinyxml2::XMLElement *xml)
{
  // Construct a basic event
  m_startTime = xml->Attribute("start");
  m_endTime = xml->Attribute("stop");
  m_channelName = Utilities::UrlDecode(xml->Attribute("channel"));

  // Add title and description, if present
  const XMLElement *title = xml->FirstChildElement("title");
  if (title)
    m_title = title->GetText();

  const XMLElement *description = xml->FirstChildElement("desc");
  if (description)
    m_description = description->GetText();
}
