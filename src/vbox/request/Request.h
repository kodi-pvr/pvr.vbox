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
#include "../response/Response.h"

namespace vbox {
  namespace request {

    /**
     * Interface for requests
     */
    class Request
    {
    public:
      virtual ~Request() {};

      /**
       * @return the type of response this request leads to
       */
      virtual response::ResponseType GetResponseType() const = 0;

      /**
       * @return the request location
       */
      virtual std::string GetLocation() const = 0;

      /**
       * @return an identifier for this request (mainly for logging purposes)
       */
      virtual std::string GetIdentifier() const = 0;
    };
  }
}
