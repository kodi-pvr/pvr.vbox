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

#include <algorithm>
#include <memory>
#include "../client.h"

namespace utilities {

  /**
   * Compares two containers for equality based on the equality of their
   * dereferenced contents (i.e. the containers should contain some kind of
   * pointers).
   */
  template<class Container>
  bool deref_equals(const Container& left, const Container &right)
  {
    return !(left.size() != right.size() || !std::equal(
      left.begin(),
      left.end(),
      right.begin(),
      [](const typename Container::value_type &leftItem, const typename Container::value_type &rightItem)
    {
      return *leftItem == *rightItem;
    }));
  }

  /**
   * Reads the contents of the file pointed to by the handle and returns it.
   * The file handle must be opened before calling this method.
   * @param fileHandle the file handle
   * @return the contents (unique pointer)
   */
  inline std::unique_ptr<std::string> ReadFileContents(void *fileHandle)
  {
    std::unique_ptr<std::string> content(new std::string());

    char buffer[1024];
    int bytesRead = 0;

    // Read until EOF or explicit error
    while ((bytesRead = XBMC->ReadFile(fileHandle, buffer, sizeof(buffer) - 1)) > 0)
      content->append(buffer, bytesRead);

    return content;
  }
}
