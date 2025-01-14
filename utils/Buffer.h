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

static const int kPrePendIndex = 8; // prependindex长度
static const int kInitalSize = 1024; // 初始化开辟空间长度

/// @brief 不定长缓冲区，切分preadable、writable、readable区域
class Buffer{
    public:
        DISALLOW_COPY_AND_MOVE(Buffer);
        Buffer();
        ~Buffer() = default;

        // 访问接口
        char *begin();
        const char *begin() const;

        char *beginread();
        const char *beginread() const;

        char *beginwrite();
        const char *beginwrite() const;

        // 缓冲区大小接口
        int readablebytes() const;
        int writablebytes() const;
        int prependablebytes() const;

        // 缓冲区写入接口
        void Append(const char *_str);
        void Append(const char *_str, int _size);
        void Append(const std::string &_str);

        // 缓冲区阅读接口
        const char *Peek() const;
        char *Peek();
        std::string PeekAsString(int _size);
        std::string PeekAllAsString();

        // 缓冲区读取接口
        void Retrieve(int _size);
        std::string RetrieveAsString(int _size);
        void RetrieveAll();
        std::string RetrieveAllAsString();
        void RetrieveUtil(const char *end);
        std::string RetrieveUtilAsString(const char *end);
    
        void EnsureWritableBytes(int len);

        bool IsEmpty() const { return beginread() == beginwrite(); };

    private:
        std::vector<char> buf_;
        int read_index_ = kPrePendIndex;
        int write_index_ = kPrePendIndex;
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