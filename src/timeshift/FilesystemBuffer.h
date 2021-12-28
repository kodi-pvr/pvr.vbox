/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2015 Sam Stenvall
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "Buffer.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <kodi/Filesystem.h>

namespace timeshift
{

  /**
   * Timeshift buffer which buffers into a file
   */
  class ATTR_DLL_LOCAL FilesystemBuffer : public Buffer
  {
  public:
    /**
     * @param bufferPath the directory to store the buffer files in
     */
    FilesystemBuffer(const std::string& bufferPath);
    virtual ~FilesystemBuffer();

    virtual bool Open(const std::string inputUrl) override;
    virtual void Close() override;
    virtual int Read(byte* buffer, size_t length) override;
    virtual int64_t Seek(int64_t position, int whence) override;

    virtual bool CanPauseStream() const override { return true; }

    virtual bool CanSeekStream() const override { return true; }

    virtual int64_t Position() const override { return m_readPosition.load(); }

    virtual int64_t Length() const override { return m_writePosition.load(); }

  private:
    const static int INPUT_READ_LENGTH;

    /**
     * The method that runs on m_inputThread. It reads data from the input
     * handle and writes it to the output handle
     */
    void ConsumeInput();


    /**
     * Closes any open file handles and resets all file positions
     */
    void Reset();

    /**
     * The path to the buffer file
     */
    std::string m_bufferPath;

    /**
     * Read-only handle to the buffer file
     */
    kodi::vfs::CFile m_outputReadHandle;

    /**
     * Write-only handle to the buffer file
     */
    kodi::vfs::CFile m_outputWriteHandle;

    /**
     * The thread that reads from m_inputHandle and writes to the output
     * handles
     */
    std::thread m_inputThread;

    /**
     * Whether the buffer is active, i.e. m_inputHandle should be read from
     */
    std::atomic<bool> m_active;

    /**
     * Protects m_output*Handle
     */
    mutable std::mutex m_mutex;

    /**
     * Signaled whenever new packets have been added to the buffer
     */
    mutable std::condition_variable m_condition;

    /**
     * The current read position in the buffer file
     */
    std::atomic<int64_t> m_readPosition;

    /**
     * The current write position in the buffer file
     */
    std::atomic<int64_t> m_writePosition;
  };
} // namespace timeshift
