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

#include "GuideChannelMapper.h"
#include <memory>
#include "lib/tinyxml2/tinyxml2.h"
#include "Exceptions.h"
#include "Utilities.h"
#include "../client.h"

using namespace vbox;
using namespace tinyxml2;

const std::string GuideChannelMapper::MAPPING_FILE_PATH = "special://userdata/addon_data/pvr.vbox/channel_mappings.xml";

GuideChannelMapper::GuideChannelMapper(
  const ::xmltv::Guide &vboxGuide,
  const ::xmltv::Guide &externalGuide) :
  m_vboxGuide(vboxGuide),
  m_externalGuide(externalGuide)
{
}

void GuideChannelMapper::Initialize()
{
  g_vbox->Log(ADDON::LOG_INFO, "Initializing channel mapper with default mappings");

  // Generate default mappings
  m_channelMappings = CreateDefaultMappings();

  // Create a default mapping file if none exists, otherwise load it
  if (!XBMC->FileExists(MAPPING_FILE_PATH.c_str(), false))
  {
    g_vbox->Log(ADDON::LOG_INFO, "No external XMLTV channel mapping file found, saving default mappings");
    Save();
  }
  else
  {
    g_vbox->Log(ADDON::LOG_INFO, "Found channel mapping file, attempting to load it");
    Load();
  }
}

ChannelMappings GuideChannelMapper::CreateDefaultMappings()
{
  ChannelMappings mappings;
  std::vector<std::string> channelNames = m_vboxGuide.GetChannelNames();

  // Add a mapping for every channel which display names matches, otherwise 
  // leave it empty
  for (const std::string &channelName : channelNames)
  {
    if (!m_externalGuide.GetChannelId(channelName).empty())
      mappings[channelName] = channelName;
    else
      mappings[channelName] = "";
  }

  return mappings;
}

void GuideChannelMapper::Load()
{
  void *fileHandle = XBMC->OpenFile(MAPPING_FILE_PATH.c_str(), 0x08 /* READ_NO_CACHE */);

  if (fileHandle)
  {
    // Read the XML
    tinyxml2::XMLDocument document;
    std::unique_ptr<std::string> contents = utilities::ReadFileContents(fileHandle);

    // Try to parse the document
    if (document.Parse(contents->c_str(), contents->size()) != XML_NO_ERROR)
      throw vbox::InvalidXMLException("XML parsing failed: " + std::string(document.ErrorName()));

    // Create mappings
    for (const XMLElement *element = document.RootElement()->FirstChildElement("mapping");
      element != nullptr; element = element->NextSiblingElement("mapping"))
    {
      const std::string vboxName = element->Attribute("vbox-name");
      const std::string xmltvName = element->Attribute("xmltv-name");

      m_channelMappings[vboxName] = xmltvName;
    }

    XBMC->CloseFile(fileHandle);
  }
}

void GuideChannelMapper::Save()
{
  // Create the document
  tinyxml2::XMLDocument document;
  XMLDeclaration *declaration = document.NewDeclaration();
  document.InsertEndChild(declaration);

  // Create the root node (<xmltvmap>)
  XMLElement *rootElement = document.NewElement("xmltvmap");
  document.InsertEndChild(rootElement);

  // Create one <mapping> for every channel
  for (const auto &mapping : m_channelMappings)
  {
    XMLElement *mappingElement = document.NewElement("mapping");
    mappingElement->SetAttribute("vbox-name", mapping.first.c_str());
    mappingElement->SetAttribute("xmltv-name", mapping.second.c_str());

    rootElement->InsertEndChild(mappingElement);
  }

  // Save the file
  void *fileHandle = XBMC->OpenFileForWrite(MAPPING_FILE_PATH.c_str(), false);

  if (fileHandle)
  {
    XMLPrinter printer;
    document.Accept(&printer);

    std::string xml = printer.CStr();
    XBMC->WriteFile(fileHandle, xml.c_str(), xml.length());

    XBMC->CloseFile(fileHandle);
  }
}

std::string GuideChannelMapper::GetExternalChannelName(const std::string &vboxName) const
{
  auto it = m_channelMappings.find(vboxName);

  return it != m_channelMappings.cend() ? it->second : "";
}
