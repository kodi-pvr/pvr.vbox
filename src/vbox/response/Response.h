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

#include <memory>
#include <string>
#include <vector>
#include <tinyxml2/tinyxml2.h>

namespace vbox {
  namespace response {

    class Response;
    typedef std::unique_ptr<Response> ResponsePtr;

    /**
     * The possible error codes a response can have
     */
    enum ErrorCode {
      SUCCESS = 0,
      UNKNOWN_METHOD,
      GENERAL_ERROR,
      MISSING_PARAMETER,
      ILLEGAL_PARAMETER,
      REQUEST_REJECTED,
      MISSING_METHOD,
      REQUEST_TIMEOUT,
      REQUEST_ABORTED,
    };

    /**
     * Represents an error
     */
    struct Error {
      ErrorCode code;
      std::string description;
    };

    /**
     * Represents an API response
     */
    class Response
    {
    public:
      Response();
      virtual ~Response();

      /**
       * Prevent copying and assignment
       */
      Response(Response const &) = delete;
      Response &operator=(Response const &) = delete;

      /**
       * Move constructor. It transfers the ownership of
       * the underlying XML document.
       */
      Response(Response &&other)
      {
        if (this != &other)
          m_document = std::move(other.m_document);
      }

      /**
       * Parses the raw XML response
       * @param rawResponse The raw response.
       */
      void ParseRawResponse(const std::string &rawResponse);

      /**
       * @return whether the response was successful
       */
      bool IsSuccessful() const
      {
        return GetErrorCode() == ErrorCode::SUCCESS;
      }

      /**
       * @return the error code
       */
      ErrorCode GetErrorCode() const
      {
        return m_error.code;
      }

      /**
       * @return the error description
       */
      std::string GetErrorDescription() const
      {
        return m_error.description;
      }

      /**
       * Returns the portion of the XML response that represents the actual
       * response content
       * @return pointer to the reply element
       */
      virtual tinyxml2::XMLElement* GetReplyElement() const;

    protected:

      /**
       * Returns the name of the element that represents the request status. The
       * element name varies slightly between different response types so it
       * may be overriden
       * @return the element name
       */
      virtual std::string GetStatusElementName() const
      {
        return "Status";
      }

      /**
      * The underlying XML response document
      */
      std::unique_ptr<tinyxml2::XMLDocument> m_document;

    private:

      /**
       * Parses the response status for possible errors
       */
      void ParseStatus();

      /**
       * The response error
       */
      Error m_error;
    };

    /**
     * Represents a response that returns XMLTV data
     */
    class XMLTVResponse : public Response
    {
    public:
      virtual tinyxml2::XMLElement* GetReplyElement() const override;

    protected:
      virtual std::string GetStatusElementName() const override
      {
        return "Error";
      }
    };

    /**
    * Represents a response that returns data about recordings and timers
    */
    class RecordingResponse : public Response
    {
    public:
      virtual tinyxml2::XMLElement* GetReplyElement() const override;

    protected:
      virtual std::string GetStatusElementName() const override
      {
        return "Error";
      }
    };
  }
}
