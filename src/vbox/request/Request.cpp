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

using namespace vbox::request;

Request::Request(const std::string &method)
  : m_method(method)
{
  // Add the method parameter
  AddParameter("Method", method);
}

std::string Request::GetUrl() const
{
  std::string url = g_vbox->GetApiBaseUrl();

  // Append parameters (including method)
  if (m_parameters.size() > 0)
    for (auto const &parameter : m_parameters)
      url += "&" + parameter.first + "=" + parameter.second;

  return url;
}

void Request::AddParameter(const std::string &name, const std::string &value)
{
  m_parameters[name] = value;
}

void Request::AddParameter(const std::string &name, int value)
{
  m_parameters[name] = std::to_string(value);
}

void Request::AddParameter(const std::string &name, unsigned int value)
{
  m_parameters[name] = std::to_string(value);
}
