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

#include "Response.h"
#include "../Exceptions.h"
#include "../VBox.h"

using namespace tinyxml2;
using namespace vbox::response;

Response::Response()
{
  m_document = std::unique_ptr<XMLDocument>(new XMLDocument());
}

Response::~Response()
{
}

void Response::ParseRawResponse(const std::string &rawResponse)
{
  // Try to parse the response as XML
  if (m_document->Parse(rawResponse.c_str()) != XML_NO_ERROR)
  {
    throw vbox::InvalidXMLException("XML parsing failed: " +
      std::string(m_document->GetErrorStr1()) +
      std::string(m_document->GetErrorStr2()));
  }

  // Parse the response status
  ParseStatus();
}

void Response::ParseStatus()
{
  int errorCode;
  std::string errorDescription;

  XMLNode *rootElement = m_document->FirstChild();
  XMLElement *statusElement = rootElement->FirstChildElement(GetStatusElementName().c_str());

  statusElement->FirstChildElement("ErrorCode")->QueryIntText(&errorCode);
  errorDescription = statusElement->FirstChildElement("ErrorDescription")->GetText();

  m_error.code = static_cast<ErrorCode>(errorCode);
  m_error.description = errorDescription;
}

XMLElement* Response::GetReplyElement() const
{
  XMLNode *rootElement = m_document->FirstChild();
  return rootElement->FirstChildElement("Reply");
}

XMLElement* XMLTVResponse::GetReplyElement() const
{
  XMLNode *rootElement = m_document->FirstChild();
  return rootElement->FirstChildElement("tv");
}

XMLElement* RecordingResponse::GetReplyElement() const
{
  XMLNode *rootElement = m_document->FirstChild();
  return rootElement->FirstChildElement("records-list");
}
