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
#include <memory>
#include <map>
#include "../xmltv/Guide.h"

namespace vbox {

  class GuideChannelMapper;
  typedef std::unique_ptr<GuideChannelMapper> GuideChannelMapperPtr;
  typedef std::map<std::string, std::string> ChannelMappings;

  /**
   * Provides functionality for mapping VBox channels into the channel names
   * used by external XMLTV guide data
   */
  class GuideChannelMapper
  {
  public:

    GuideChannelMapper(const ::xmltv::Guide &vboxGuide, const ::xmltv::Guide &externalGuide);
    ~GuideChannelMapper() = default;

    /**
     * Initializes the mapper by loading the mappings from disk. If no existing
     * mappings exist, a basic map is created.
     */
    void Initialize();

    /**
    * @param vboxName a VBox channel name
    * @return the corresponding channel name from the external guide
    */
    std::string GetExternalChannelName(const std::string &vboxName) const;

  private:

    /**
     * Creates a default mapping between the two guides using exact name matching
     * @eturn the mappings
     */
    ChannelMappings CreateDefaultMappings();

    /**
     * Loads the mappings from disk
     */
    void Load();

    /**
     * Saves the current mappings to disk
     */
    void Save();

    /**
     * The path to the mapping file
     */
    const static std::string MAPPING_FILE_PATH;

    /**
     * The internal guide data
     */
    const ::xmltv::Guide &m_vboxGuide;

    /**
     * The external guide data
     */
    const ::xmltv::Guide &m_externalGuide;

    /**
     * The channel name mappings
     */
    ChannelMappings m_channelMappings;
  };
}
