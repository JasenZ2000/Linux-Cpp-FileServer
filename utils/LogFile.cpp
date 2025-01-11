#include "LogFile.h"
#include <cassert>
#include <cstdio>
#include <string>

LogFile::LogFile(const char* path)
    : fp_(::fopen(path, "a+")),
      writtenBytes_(0),
      lastFlush_(0),
      lastWrite_(0)
{
    if (fp_ == nullptr) {
        // 文件夹得手动创建，C++17可以用filesystem
        std::string DefaultPath = std::move("./LogFiles/LogFile_" +
            TimerStamp::Now().TimerStamp::GetTimeString() + ".log");
        
        fp_ = ::fopen(DefaultPath.c_str(), "a+");
    }
    assert(fp_ != nullptr);
}

LogFile::~LogFile()
{
    Flush();
    if (!fp_)
        ::fclose(fp_);
}

void LogFile::Write(const char *logline, int len)
{
    int pos = 0;
    while (pos != len)
    {
        pos += static_cast<int>(fwrite_unlocked(logline + pos, sizeof(char), len - pos, fp_));
    }
    int64_t now = TimerStamp::Now().GetMicroSeconds();
    if (len != 0)
    {
        lastWrite_ = now;
        writtenBytes_ += len;
    }

    if (now - lastFlush_ > FlushInterval)
    {
        Flush();
        lastFlush_ = now;
    }
    
}

void LogFile::Flush() { fflush(fp_); }

std::string LogFile::RollFile() 
{
    std::string DefaultPath = std::move("../LogFiles/LogFile_" +
            TimerStamp::Now().TimerStamp::GetTimeString() + ".log");
    path_ = DefaultPath;
    return DefaultPath;
}