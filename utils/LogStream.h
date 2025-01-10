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

#include "common.h"
#include <string>
#include <string.h>
#include <type_traits>
#include <assert.h>
#include <algorithm>

static const int kBufferSize = 4096;
static const int kMaxNumSize = 48;

/**
 * @brief 定长缓冲区，无需二次分配内存
 */
class FixedBuffer
{
public:
    FixedBuffer() : cur_(data_) {}
    ~FixedBuffer() = default;

    void append(const char *buf, int len)
    {
        if (avail() > len)
        {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    void add(int len) { cur_ += len; }
    void reset() { clear(); cur_ = data_; }
    void clear()
    {
        memset(data_, 0, sizeof(data_));
    }

    int avail() const { return static_cast<int>(end() - cur_); };
    int len() const { return static_cast<int>(cur_ - data_); }
    
    const char *data() const { return data_; }
    char *current() { return cur_; }
    const char *end() const { return data_ + sizeof(data_); }

private:
    char data_[kBufferSize];
    char *cur_;
};

/**
 * @brief 日志输出流
 */
class LogStream
{
public:
    DISALLOW_COPY_AND_MOVE(LogStream);

    LogStream() = default;
    ~LogStream() = default;

    void append(const char *data, int len) { buffer_.append(data, len); }

    const FixedBuffer &buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

    LogStream &operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream &operator<<(const std::string &v)
    {
        buffer_.append(v.c_str(), v.size());
        return *this;
    }

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

private:
    template <typename T>
    void formatInteger(T);
    FixedBuffer buffer_;
};

// 泛型化只能在头文件中实现
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

inline LogStream & operator<<(LogStream& s, const Fmt& fmt){
    s.append(fmt.data(), fmt.length());
    return s;
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

// 格式化输出 - 字符(串)类型 - 对高度重复输出的字符串进行优化，避免反复读取strlen
class Template
{
public:
    Template(const char *str, int len) : str_(str), len_(len) {}
    const char *str_;
    const unsigned len_;
};

inline LogStream & operator<<(LogStream& s, const Template& v){
    s.append(v.str_, v.len_);
    return s;
};

inline LogStream& LogStream:: operator<<(short num){
    return (*this) << static_cast<int>(num);
}

inline LogStream& LogStream:: operator<<(unsigned short num){
    return (*this) << static_cast<unsigned int>(num);
}
inline LogStream& LogStream:: operator<<(int num){
    formatInteger(num);
    return *this;
}
inline LogStream& LogStream:: operator<<(unsigned int num){
    formatInteger(num);
    return *this;
}
inline LogStream& LogStream:: operator<<(long num){
    formatInteger(num);
    return *this;
}
inline LogStream& LogStream:: operator<<(unsigned long num){
    formatInteger(num);
    return *this;
}
inline LogStream& LogStream:: operator<<(long long num){
    formatInteger(num);
    return *this;
}
inline LogStream& LogStream:: operator<<(unsigned long long num){
    formatInteger(num);
    return *this;
}

// 浮点类型数据转换成字符串
inline LogStream& LogStream:: operator<<(float num){
    return (*this) << static_cast<const double>(num);
}

inline LogStream& LogStream:: operator<<(double num){
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%g", num);
    buffer_.append(buf, len);
    return *this;
}

inline LogStream& LogStream:: operator<<(char v){
    buffer_.append(&v, 1);
    return *this;
}

// 原生字符串输出到缓冲区
inline LogStream& LogStream:: operator<<(const char* str){
    if (str){ buffer_.append(str, strlen(str)); }
    else    { buffer_.append("(null)", 6);      }

    return *this;
}