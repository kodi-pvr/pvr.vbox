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
#include <vector>
#include <memory>
#include <functional>
#include <cstdlib>

// Forward declarations
namespace tinyxml2
{
  class XMLElement;
}

namespace xmltv {
    
  class Programme;
  typedef std::unique_ptr<Programme> ProgrammePtr;

  /**
   * Represents an actor
   */
  struct Actor
  {
    std::string role;
    std::string name;
  };

  /**
   * Represents the credits for a programme (actors, director etc.)
   */
  struct Credits
  {
    std::vector<std::string> directors;
    std::vector<Actor> actors;
    std::vector<std::string> producers;
    std::vector<std::string> writers;
  };

  /**
   * Represents a single programme/event
   */
  class Programme
  {
  public:

    /**
     * Creates a programme from the specified <programme> element
     */
    Programme(const tinyxml2::XMLElement *xml);
    virtual ~Programme() {}

    /**
    * @return a unique ID of this object based on the hash content
    */
    unsigned int GetUniqueId() const
    {
      std::hash<std::string> hasher;
      int uniqueId = hasher(m_channelName + m_startTime + m_endTime);
      return std::abs(uniqueId);
    }

    const std::vector<std::string>& GetDirectors() const
    {
      return m_credits.directors;
    }

    const std::vector<Actor>& GetActors() const
    {
      return m_credits.actors;
    }

    const std::vector<std::string>& GetProducers() const
    {
      return m_credits.producers;
    }

    const std::vector<std::string>& GetWriters() const
    {
      return m_credits.writers;
    }

    const std::vector<std::string>& GetCategories() const
    {
      return m_categories;
    }

    std::string m_startTime;
    std::string m_endTime;
    std::string m_channelName;
    std::string m_title;
    std::string m_description;
    std::string m_subTitle;
    int m_year;
    std::string m_starRating;

  private:

    /**
     * Parses the credits from the specified <credits> element
     */
    void ParseCredits(const tinyxml2::XMLElement *creditsElement);

    Credits m_credits;
    std::vector<std::string> m_categories;
  };
}
