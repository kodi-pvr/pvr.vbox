/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2015 Sam Stenvall
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "Buffer.h"

#include <sstream>

using namespace timeshift;

const int Buffer::DEFAULT_READ_TIMEOUT = 10;

bool Buffer::Open(const std::string inputUrl)
{
  // Append the read timeout parameter
  std::stringstream ss;
  ss << inputUrl << "|connection-timeout=" << m_readTimeout;

  // Remember the start time and open the input
  m_startTime = time(nullptr);

  return m_inputHandle.OpenFile(ss.str(), ADDON_READ_NO_CACHE);
}

Buffer::~Buffer()
{
  Buffer::Close();
}

void Buffer::Close()
{
  CloseHandle(m_inputHandle);
}

void Buffer::CloseHandle(kodi::vfs::CFile& handle)
{
  if (handle.IsOpen())
  {
    handle.Close();
  }
}
