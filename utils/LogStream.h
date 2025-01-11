/**
 * @file LogStream.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 包含日志输出流的定义以及其辅助类
 * @version 0.1
 * @date 2025-01-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "common.h"
#include "Buffer.h"
#include <string>
#include <string.h>
#include <type_traits>
#include <assert.h>
#include <algorithm>

// 格式化输出 - 数值类型
class Fmt{
public:
    template <typename T>
    Fmt(const char *fmt, T val);

    const char *data() const { return buf_; }

    int length() const { return length_; }

private:
    char buf_[32];
    int length_;
};

template<typename T>
Fmt::Fmt(const char* fmt, T val)
{
    static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");

    length_ = snprintf(buf_, sizeof(buf_), fmt, val);
    assert(static_cast<size_t>(length_) < sizeof(buf_));
};

// 格式化输出 - 字符(串)类型 - 对高度重复输出的字符串进行优化，避免反复读取strlen
class Template
{
public:
    Template(const char *str, int len) : str_(str), len_(len) {}
    const char *str_;
    const unsigned len_;
};

/**
 * @brief 日志输出流，使用更短的缓冲区，避免二次分配内存
 */
class LogStream
{
public:
    DISALLOW_COPY_AND_MOVE(LogStream);
    typedef FixedBuffer<kBufferSize> Buffer;

    LogStream() = default;
    ~LogStream() = default;

    void append(const char *data, int len) { buffer_.append(data, len); }

    const Buffer &buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

    LogStream &operator<<(bool v);
    LogStream &operator<<(const std::string &v);

    LogStream &operator<<(short);
    LogStream &operator<<(unsigned short);
    LogStream &operator<<(int);
    LogStream &operator<<(unsigned int);
    LogStream &operator<<(long);
    LogStream &operator<<(unsigned long);
    LogStream &operator<<(long long);
    LogStream &operator<<(unsigned long long);

    LogStream &operator<<(float v);
    LogStream &operator<<(double);

    LogStream &operator<<(char v);
    LogStream &operator<<(const char *);

    LogStream &operator<<(const Fmt& fmt);

    LogStream &operator<<(const Template& v);

private:
    template <typename T>
    void formatInteger(T);
    Buffer buffer_;
};

// 泛型化只能在头文件中实现,非格式化数值类默认调用该函数
template <typename T>
void LogStream::formatInteger(T value)
{
    if(buffer_.avail() >= kMaxNumSize){
        char *buf = buffer_.current();
        char *now = buf;

        do {
            int remainder = value % 10;
            *(now++) = remainder + '0';
            value /= 10;
        } while (value != 0);
        if (value < 0){
            *(now++) = '-';
        }
        std::reverse(buf, now);
        buffer_.add(now - buf);
    }
};

// Explicit instantiations

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);