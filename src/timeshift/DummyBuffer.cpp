/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2015 Sam Stenvall
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "DummyBuffer.h"

using namespace timeshift;

int DummyBuffer::Read(byte* buffer, size_t length)
{
  return m_inputHandle.Read(buffer, length);
}

int64_t DummyBuffer::Seek(int64_t position, int whence)
{
  return m_inputHandle.Seek(position, whence);
}
