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
#include "lib/tinyxml2/tinyxml2.h"

using namespace xmltv;
using namespace tinyxml2;

const std::string Programme::STRING_FORMAT_NOT_SUPPORTED = "String format is not supported";

Programme::Programme(const tinyxml2::XMLElement *xml)
  : m_year(0)
{
  // Construct a basic event
  m_startTime = xml->Attribute("start");
  m_endTime = xml->Attribute("stop");
  m_channelName = Utilities::UrlDecode(xml->Attribute("channel"));

  // Title
  const XMLElement *element = xml->FirstChildElement("title");
  if (element)
    m_title = element->GetText();

  // Subtitle
  element = xml->FirstChildElement("sub-title");
  if (element)
    m_subTitle = element->GetText();

  // Description
  element = xml->FirstChildElement("desc");
  if (element)
    m_description = element->GetText();

  // Credits
  element = xml->FirstChildElement("credits");
  if (element)
    ParseCredits(element);

  // Date
  element = xml->FirstChildElement("date");
  if (element)
    m_year = Utilities::QueryIntText(element);

  // Icon
  element = xml->FirstChildElement("icon");
  if (element)
    m_icon = element->Attribute("src");

  // Categories. Skip "movie" and "series" since most people treat categories 
  // as genres
  for (element = xml->FirstChildElement("category");
    element != NULL; element = element->NextSiblingElement("category"))
  {
    auto *category = element->GetText();
    if (!category)
      continue;

    std::string genre(category);
    if (genre == "movie" || genre == "series")
      continue;

    m_categories.push_back(genre);
  }

  // Star rating
  element = xml->FirstChildElement("star-rating");
  if (element)
  {
    element = element->FirstChildElement("value");
    if (element)
      m_starRating = element->GetText();
  }
}

void Programme::ParseCredits(const XMLElement *creditsElement)
{
  // Actors
  for (const XMLElement *element = creditsElement->FirstChildElement("actor");
    element != NULL; element = element->NextSiblingElement("actor"))
  {
    Actor actor;

    auto *name = element->GetText();
    auto *role = element->Attribute("role");

    if (name)
      actor.name = element->GetText();
    if (role)
      actor.role = role;

    m_credits.actors.push_back(actor);
  }

  // Directors
  for (const XMLElement *element = creditsElement->FirstChildElement("director");
    element != NULL; element = element->NextSiblingElement("director"))
  {
    auto *director = element->GetText();
    if (director)
      m_credits.directors.push_back(director);
  }

  // Producers
  for (const XMLElement *element = creditsElement->FirstChildElement("producer");
    element != NULL; element = element->NextSiblingElement("producer"))
  {
    auto *producer = element->GetText();
    if (producer)
      m_credits.producers.push_back(producer);
  }

  // Writers
  for (const XMLElement *element = creditsElement->FirstChildElement("writer");
    element != NULL; element = element->NextSiblingElement("writer"))
  {
    auto *writer = element->GetText();
    if (writer)
      m_credits.writers.push_back(writer);
  }
}
