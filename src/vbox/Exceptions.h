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

#include <stdexcept>

namespace vbox {
  // Base exception class
  class VBoxException : public std::runtime_error
  {
  public:
    VBoxException(const std::string &message) : std::runtime_error(message) {}
  };

  // Domain-specific exceptions
  class InvalidXMLException : public VBoxException
  {
  public:
    InvalidXMLException(const std::string &message) : VBoxException(message) {};
  };

  class InvalidResponseException : public VBoxException
  {
  public:
    InvalidResponseException(const std::string &message) : VBoxException(message) {};
  };

  class RequestFailedException : public VBoxException
  {
  public:
    RequestFailedException(const std::string &message) : VBoxException(message) {};
  };

  class FirmwareVersionException : public VBoxException
  {
  public:
    FirmwareVersionException(const std::string &message) : VBoxException(message) {};
  };
}
