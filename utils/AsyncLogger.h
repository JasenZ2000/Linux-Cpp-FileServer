/**
 * @file AsyncLogger.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 异步日志输出类，默认输出到文件中
 * @version 0.1
 * @date 2025-01-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <vector>
#include <memory>
#include "Buffer.h"
#include "LogFile.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

static const double LoggerBufferTimeout = 3.0;
static const int64_t LoggerFileMaxSize = 1024 * 1024 * 1024;

class AsyncLogger
{
public:
    typedef FixedBuffer<kLargeBufferSize> Buffer;

    AsyncLogger(const char* path = nullptr);
    ~AsyncLogger();

    void Start();
    void Stop();

    void Append(const char *logline, int len);

    void Flush();

private:
    void ThreadFunc();
    const char *filepath_;
    
    bool running_;
    std::unique_ptr<LogFile> logFile_;

    std::unique_ptr<Buffer> buffer_;
    std::unique_ptr<Buffer> nextBuffer_;
    std::vector<std::unique_ptr<Buffer>> buffers_;

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;

};