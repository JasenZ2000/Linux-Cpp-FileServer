#pragma once

#include <stdio.h>
#include <string>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "TimerStamp.h"

/// @brief 日志文件类，用于将日志写入文件
class LogFile
{
public:
    LogFile(const char* path);
    ~LogFile();
    void Write(const char *logline, int len);
    void Flush();
    std::string RollFile();
    int64_t WrittenBytes() const { return writtenBytes_; }

private:
    FILE *fp_;
    std::string path_;
    int64_t writtenBytes_;
    int64_t lastFlush_;
    int64_t lastWrite_;
    int64_t FlushInterval = 3 * 1000000;
};