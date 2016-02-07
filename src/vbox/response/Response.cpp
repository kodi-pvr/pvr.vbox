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
#include "../../xmltv/Utilities.h"

using namespace tinyxml2;
using namespace vbox::response;

Response::Response()
{
  // Some XMLTV files have weird line endings, try to account for that
  m_document = std::unique_ptr<XMLDocument>(
    new XMLDocument(/*processEntities = */true, tinyxml2::PRESERVE_WHITESPACE));

  m_error.code = ErrorCode::SUCCESS;
  m_error.description = "";
}

Response::~Response()
{
}

void Response::ParseRawResponse(const std::string &rawResponse)
{
  // Try to parse the response as XML
  if (m_document->Parse(rawResponse.c_str(), rawResponse.size()) != XML_NO_ERROR)
    throw vbox::InvalidXMLException("XML parsing failed: " + std::string(m_document->ErrorName()));

  // Parse the response status
  ParseStatus();
}

void Response::ParseStatus()
{
  int errorCode;
  std::string errorDescription;

  XMLNode *rootElement = m_document->RootElement();
  XMLElement *statusElement = rootElement->FirstChildElement(GetStatusElementName().c_str());

  // Not all response types always return the status element
  if (statusElement)
  {
    errorCode = xmltv::Utilities::QueryIntText(statusElement->FirstChildElement("ErrorCode"));
    errorDescription = statusElement->FirstChildElement("ErrorDescription")->GetText();

    m_error.code = static_cast<ErrorCode>(errorCode);
    m_error.description = errorDescription;
  }
}

XMLElement* Response::GetReplyElement() const
{
  XMLNode *rootElement = m_document->RootElement();
  return rootElement->FirstChildElement("Reply");
}

XMLElement* XMLTVResponse::GetReplyElement() const
{
  return m_document->RootElement();
}

XMLElement* RecordingResponse::GetReplyElement() const
{
  return m_document->RootElement();
}
