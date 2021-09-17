/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2015 Sam Stenvall
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "FilesystemBuffer.h"

#include <cstring>

using namespace timeshift;

const int FilesystemBuffer::INPUT_READ_LENGTH = 32768;

FilesystemBuffer::FilesystemBuffer(const std::string& bufferPath)
  : Buffer(), m_readPosition(0), m_writePosition(0)
{
  m_bufferPath = bufferPath + "/buffer.ts";
}

FilesystemBuffer::~FilesystemBuffer()
{
  FilesystemBuffer::Close();

  // Remove the buffer file so it doesn't take up space once Kodi has exited
  kodi::vfs::DeleteFile(m_bufferPath);
}

bool FilesystemBuffer::Open(const std::string inputUrl)
{
  // Open the file handles
  m_outputWriteHandle.OpenFileForWrite(m_bufferPath, true);
  m_outputReadHandle.OpenFile(m_bufferPath, ADDON_READ_NO_CACHE);

  if (!Buffer::Open(inputUrl) || !m_outputReadHandle.IsOpen() || !m_outputWriteHandle.IsOpen())
    return false;

  // Start the input thread
  m_active = true;
  m_inputThread = std::thread([this]() { ConsumeInput(); });

  return true;
}

void FilesystemBuffer::Close()
{
  // Wait for the input thread to terminate
  m_active = false;

  if (m_inputThread.joinable())
    m_inputThread.join();

  Reset();
  Buffer::Close();
}

void FilesystemBuffer::Reset()
{
  // Close any open handles
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_outputReadHandle.IsOpen())
    CloseHandle(m_outputReadHandle);

  if (m_outputWriteHandle.IsOpen())
    CloseHandle(m_outputWriteHandle);

  // Reset
  m_readPosition = m_writePosition = 0;
}

int FilesystemBuffer::Read(byte* buffer, size_t length)
{
  // Wait until we have enough data
  int64_t requiredLength = Position() + length;

  std::unique_lock<std::mutex> lock(m_mutex);
  m_condition.wait_for(lock, std::chrono::seconds(m_readTimeout),
                       [this, requiredLength]()
                       {
                         return Length() >= requiredLength;
                       });

  // Now we can read
  int read = m_outputReadHandle.Read(buffer, length);

  m_readPosition += read;
  return read;
}

int64_t FilesystemBuffer::Seek(int64_t position, int whence)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  int64_t newPosition = m_outputReadHandle.Seek(position, whence);

  m_readPosition.exchange(newPosition);
  return newPosition;
}

void FilesystemBuffer::ConsumeInput()
{
  byte* buffer = new byte[INPUT_READ_LENGTH];

  while (m_active)
  {
    // Read from m_inputHandle
    ssize_t read = m_inputHandle.Read(buffer, INPUT_READ_LENGTH);

    // Write to m_outputHandle
    std::unique_lock<std::mutex> lock(m_mutex);
    ssize_t written = m_outputWriteHandle.Write(buffer, read);
    m_writePosition += written;

    // Signal that we have data again
    m_condition.notify_one();
  }

  delete[] buffer;
}
