/**
 * @file Logger.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 目前看起来是个写入文件的日志输出流
 * @version 0.1
 * @date 2025-01-09
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once
#include <string>
#include <string.h>
#include "TimerStamp.h"
#include "LogStream.h"
#include "common.h"

// 日志输出流-用户级:仅提供接口，不提供实现，甚至不独立保存数据
class Logger
{
public:
    DISALLOW_COPY_AND_MOVE(Logger);
    enum LogLevel
    {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };

    Logger(const char *file, int line, LogLevel level);
    ~Logger();
    LogStream &stream() { return impl_.stream(); }
    
    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char *data, int len); // 定义函数指针
    typedef void (*FlushFunc)();
    // 默认fwrite到stdout
    static void setOutput(OutputFunc);
    // 默认fflush到stdout
    static void setFlush(FlushFunc);

private:
    // 日志输出流-实现级
    class Impl
    {
    public:
        DISALLOW_COPY_AND_MOVE(Impl);
        typedef Logger::LogLevel LogLevel;
        Impl(const char *file, int line, LogLevel level);
        ~Impl() = default;

        static const char *GetBaseName(const char *file);
        void FormatTime();
        void Finish();

        LogStream &stream() { return stream_; }

        const char* logLevelString() const;
        LogLevel level_;

    private:
        LogStream stream_;
        int line_;
        const char * basename_;
    };
    Impl impl_;
};

// 全局日志级别
extern Logger::LogLevel g_logLevel;
inline Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}

// 日志宏，快速获取日志输出流
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::DEBUG).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__, Logger::INFO).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()