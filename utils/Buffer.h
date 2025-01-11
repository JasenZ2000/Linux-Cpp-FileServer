/**
 * @file Buffer.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 定长缓冲区用于日志输出，不定长缓冲区用于网络传输
 * @version 0.1
 * @date 2025-01-10
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <memory.h>
#include <string.h>
#include <string>
#include <vector>
#include "common.h"


class Buffer{
    public:
        DISALLOW_COPY_AND_MOVE(Buffer);
        Buffer() = default;
        ~Buffer() = default;

        const std::string &buf() const { return buf_; };
        const char *c_str() const { return buf_.c_str(); };

        void set_buf(const char *buf);

        size_t Size() const { return buf_.size(); };
        void Append(const char *_str, int _size);
        void Clear() { buf_.clear(); };
    
    private:
        std::string buf_;
};

static const int kBufferSize = 4096;
static const int kLargeBufferSize = 4096 * 1000;
static const int kMaxNumSize = 48;

/**
 * @brief 定长缓冲区，无需二次分配内存
 */
template <int SIZE>
class FixedBuffer
{
public:
    // template <int SIZE> 只有在类外定义以及特化时才需要
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

    void clear() { memset(data_, 0, sizeof(data_)); }

    int avail() const { return static_cast<int>(end() - cur_); };

    int len() const { return static_cast<int>(cur_ - data_); }
    
    const char *data() const { return data_; }

    char *current() { return cur_; }

    const char *end() const { return data_ + sizeof(data_); }

private:
    char data_[SIZE];
    char *cur_;
};