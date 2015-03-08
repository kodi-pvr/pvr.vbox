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

namespace vbox {
  namespace request {

    /**
     * Represents an API request
     */
    class Request
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

      /**
       * @return the request method
       */
      std::string GetMethod() const
      {
        return m_method;
      }

      /**
       * @return the complete URL for the request
       */
      std::string GetUrl() const;

    private:

      /**
       * URL-encodes the specified string
       *
       * @param name the string to encode
       * @return the encoded string
       */
      std::string EncodeUrl(const std::string &string) const;

      /**
       * The method name
       */
      std::string m_method;

      /**
       * The request parameters (and their values)
       */
      std::map<std::string, std::string> m_parameters;
    };
  }
}
