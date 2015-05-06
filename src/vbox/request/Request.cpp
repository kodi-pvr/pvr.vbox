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

#include "Request.h"
#include "../../client.h"
#include "../../xmltv/Utilities.h"
#include "../../compat.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace vbox::request;
using vbox::response::ResponseType;

const std::vector<std::string> Request::externalCapableMethods = {
  "GetXmltvEntireFile",
  "GetXmltvSection",
  "GetXmltvChannelsList",
  "GetXmltvProgramsList",
  "GetRecordsList"
};

const std::vector<std::string> Request::xmltvMethods = {
  "GetXmltvEntireFile",
  "GetXmltvSection",
  "GetXmltvChannelsList",
  "GetXmltvProgramsList",
};

Request::Request(const std::string &method)
  : m_method(method)
{
  AddParameter("Method", method);

  // Add external IP and port options to the methods that support it
  if (std::find(
    externalCapableMethods.cbegin(), 
    externalCapableMethods.cend(), method) != externalCapableMethods.cend())
  {
    AddParameter("ExternalIP", g_vbox->GetConnectionParams().hostname);
    AddParameter("Port", g_vbox->GetConnectionParams().upnpPort);
  }
}

ResponseType Request::GetResponseType() const
{
  // Determine the response type based on the method name
  if (std::find(xmltvMethods.cbegin(), xmltvMethods.cend(), m_method) != xmltvMethods.cend())
    return ResponseType::XMLTV;
  else if (m_method == "GetRecordsList")
    return ResponseType::RECORDS;

  return ResponseType::GENERIC;
}

std::string Request::GetLocation() const
{
  std::string url = g_vbox->GetApiBaseUrl();

  // Append parameters (including method)
  if (m_parameters.size() > 0)
    for (auto const &parameter : m_parameters)
      url += "&" + parameter.first + "=" + ::xmltv::Utilities::UrlEncode(parameter.second);

  return url;
}

void Request::AddParameter(const std::string &name, const std::string &value)
{
  m_parameters[name] = value;
}

void Request::AddParameter(const std::string &name, int value)
{
  m_parameters[name] = compat::to_string(value);
}

void Request::AddParameter(const std::string &name, unsigned int value)
{
  m_parameters[name] = compat::to_string(value);
}
