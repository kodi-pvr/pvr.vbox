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
#include "Request.h"

namespace vbox {
  namespace request {

    /**
     * Represents a local file request
     */
    class FileRequest : public Request
    {
    public:
      FileRequest(const std::string &path) 
        : m_path(path) {}
      virtual ~FileRequest() {}
      
      virtual response::ResponseType GetResponseType() const override
      {
        // Currently we always expect local files to contain XMLTV
        return response::ResponseType::XMLTV;
      }

      virtual std::string GetLocation() const override
      {
        return m_path;
      }

      virtual std::string GetIdentifier() const override
      {
        return "FileRequest for \"" + m_path + "\"";
      }

    private:
      std::string m_path;
    };
  }
}