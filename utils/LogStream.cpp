#include "LogStream.h"

LogStream &LogStream::operator<<(const Template &v)
{
    buffer_.append(v.str_, v.len_);
    return *this;
};

LogStream &LogStream::operator<<(const Fmt &fmt)
{
    buffer_.append(fmt.data(), fmt.length());
    return *this;
};

LogStream &LogStream::operator<<(bool v)
{
    buffer_.append(v ? "1" : "0", 1);
    return *this;
}

LogStream &LogStream::operator<<(const std::string &v)
{
    buffer_.append(v.c_str(), v.size());
    return *this;
}

LogStream &LogStream::operator<<(short num)
{
    return (*this) << static_cast<int>(num);
}

LogStream &LogStream::operator<<(unsigned short num)
{
    return (*this) << static_cast<unsigned int>(num);
}
LogStream &LogStream::operator<<(int num)
{
    formatInteger(num);
    return *this;
}

LogStream &LogStream::operator<<(unsigned int num)
{
    formatInteger(num);
    return *this;
}

LogStream &LogStream::operator<<(long num)
{
    formatInteger(num);
    return *this;
}

LogStream &LogStream::operator<<(unsigned long num)
{
    formatInteger(num);
    return *this;
}

LogStream &LogStream::operator<<(long long num)
{
    formatInteger(num);
    return *this;
}

LogStream &LogStream::operator<<(unsigned long long num)
{
    formatInteger(num);
    return *this;
}

// 浮点类型数据转换成字符串

LogStream &LogStream::operator<<(float num)
{
    return (*this) << static_cast<const double>(num);
}

LogStream &LogStream::operator<<(double num)
{
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%g", num);
    buffer_.append(buf, len);
    return *this;
}

LogStream &LogStream::operator<<(char v)
{
    buffer_.append(&v, 1);
    return *this;
}

// 原生字符串输出到缓冲区
LogStream &LogStream::operator<<(const char *str)
{
    if (str)
    {
        buffer_.append(str, strlen(str));
    }
    else
    {
        buffer_.append("(null)", 6);
    }

    return *this;
}