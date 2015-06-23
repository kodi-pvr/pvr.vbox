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

#include "FilesystemBuffer.h"
#include <cstring>

using namespace timeshift;

const int FilesystemBuffer::INPUT_READ_LENGTH = 32768;

// Fix a stupid #define on Windows which causes XBMC->DeleteFile() to break
#ifdef _WIN32
#undef DeleteFile
#endif // _WIN32

FilesystemBuffer::FilesystemBuffer(const std::string &bufferPath)
  : Buffer(), m_outputReadHandle(nullptr), m_outputWriteHandle(nullptr),
  m_readPosition(0), m_writePosition(0)
{
  m_bufferPath = bufferPath + "/buffer.ts";
}

FilesystemBuffer::~FilesystemBuffer()
{
  FilesystemBuffer::Close();

  // Remove the buffer file so it doesn't take up space once Kodi has exited
  XBMC->DeleteFile(m_bufferPath.c_str());
}

bool FilesystemBuffer::Open(const std::string inputUrl)
{
  // Open the file handles
  m_outputWriteHandle = XBMC->OpenFileForWrite(m_bufferPath.c_str(), true);
  m_outputReadHandle = XBMC->OpenFile(m_bufferPath.c_str(), 0x08/*READ_NO_CACHE*/);

  if (!Buffer::Open(inputUrl) || !m_outputReadHandle || !m_outputWriteHandle)
    return false;

  // Start the input thread
  m_active = true;
  m_inputThread = std::thread([this]()
  {
    ConsumeInput();
  });

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

  if (m_outputReadHandle)
    CloseHandle(m_outputReadHandle);

  if (m_outputWriteHandle)
    CloseHandle(m_outputWriteHandle);

  // Reset
  m_outputReadHandle = m_outputWriteHandle = nullptr;
  m_readPosition = m_writePosition = 0;
}

int FilesystemBuffer::Read(byte *buffer, size_t length)
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
  int read = XBMC->ReadFile(m_outputReadHandle, buffer, length);

  m_readPosition += read;
  return read;
}

int64_t FilesystemBuffer::Seek(int64_t position, int whence)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  int64_t newPosition = XBMC->SeekFile(m_outputReadHandle, position, whence);

  m_readPosition.exchange(newPosition);
  return newPosition;
}

void FilesystemBuffer::ConsumeInput()
{
  byte *buffer = new byte[INPUT_READ_LENGTH];

  while (m_active)
  {
    memset(buffer, 0, INPUT_READ_LENGTH);

    // Read from m_inputHandle
    ssize_t read = XBMC->ReadFile(m_inputHandle, buffer, INPUT_READ_LENGTH);

    // Write to m_outputHandle
    std::unique_lock<std::mutex> lock(m_mutex);
    ssize_t written = XBMC->WriteFile(m_outputWriteHandle, buffer, read);
    m_writePosition += written;

    // Signal that we have data again
    m_condition.notify_one();
  }

  delete[] buffer;
}
