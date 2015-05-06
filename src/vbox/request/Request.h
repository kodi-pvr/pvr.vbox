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
#include <map>
#include <vector>
#include "IRequest.h"

namespace vbox {
  namespace request {

    /**
     * Represents an API request
     */
    class Request : public IRequest
    {
    public:
      Request(const std::string &method);
      virtual ~Request() {};

      /**
       * Adds a request parameter with the specified value
       *
       * @param name  The name.
       * @param value The value.
       */
      void AddParameter(const std::string &name, const std::string &value);
      void AddParameter(const std::string &name, int value);
      void AddParameter(const std::string &name, unsigned int value);

      void SetTimeout(int timeout);

      virtual vbox::response::ResponseType GetResponseType() const override;
      virtual std::string GetLocation() const override;

    private:

      /**
       * The method name
       */
      std::string m_method;

      /**
       * The request parameters (and their values)
       */
      std::map<std::string, std::string> m_parameters;

      /**
       * The timeout to use for the request. Defaults to zero which means the 
       * default underlying systems timeout is used.
       */
      int m_timeout;

      /**
       * List of methods that can take an optional "ExternalIP" parameter
       */
      static const std::vector<std::string> externalCapableMethods;

      /**
       * List of methods that return XMLTV responses
       */
      static const std::vector<std::string> xmltvMethods;
    };
  }
}
