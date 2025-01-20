#include "Buffer.h"
#include <assert.h>

Buffer::Buffer() : buf_(kPrePendIndex) {}

char *Buffer::begin() { return &*buf_.begin(); }
const char *Buffer::begin() const { return &*buf_.begin(); }

char *Buffer::beginread() { return begin() + read_index_; }
const char *Buffer::beginread() const { return begin() + read_index_; }

char *Buffer::beginwrite() { return begin() + write_index_; }
const char *Buffer::beginwrite() const { return begin() + write_index_; }

void Buffer::EnsureWritableBytes(int len)
{
    if (writablebytes() >= len)
        return;
    if (writablebytes() + prependablebytes() >= kPrePendIndex + len)
    {
        std::copy(beginread(), beginwrite(), begin() + kPrePendIndex);
        write_index_ = kPrePendIndex + readablebytes();
        read_index_ = kPrePendIndex;
    }
    else
    {
        buf_.resize(write_index_ + len);
    }
}

void Buffer::Append(const char *str, int size)
{
    EnsureWritableBytes(size);
    std::copy(str, str + size, beginwrite());
    write_index_ += size;
}

void Buffer::Append(const char *str)
{
    Append(str, static_cast<int>(strlen(str)));
}

void Buffer::Append(const std::string &str)
{
    Append(str.c_str(), static_cast<int>(str.size()));
}

void Buffer::Append(std::istream &in)
{
    const size_t buffer_size = 4096; // 每次读取4KB
    char temp_buffer[buffer_size];
    while (in)
    {
        in.read(temp_buffer, buffer_size);
        Append(temp_buffer, in.gcount());
    }
}

void Buffer::Append(std::istream &in, size_t size)
{
    const size_t buffer_size = 4096; // 每次读取4KB
    char temp_buffer[buffer_size];

    while (in && size > 0)
    {
        size_t bytes_to_read = std::min(size, buffer_size);
        in.read(temp_buffer, bytes_to_read);
        Append(temp_buffer, in.gcount());

        size -= bytes_to_read;
    }
}

int Buffer::readablebytes() const { return write_index_ - read_index_; }
int Buffer::writablebytes() const { return static_cast<int>(buf_.size()) - write_index_; }
int Buffer::prependablebytes() const { return read_index_; }

const char *Buffer::Peek() const { return beginread(); }
char *Buffer::Peek() { return beginread(); }

std::string Buffer::PeekAsString(int len)
{
    return std::string(beginread(), beginread() + len);
}

std::string Buffer::PeekAllAsString()
{
    return std::string(beginread(), beginwrite());
}

// 相当于清空
void Buffer::RetrieveAll()
{
    write_index_ = kPrePendIndex;
    read_index_ = write_index_;
}

void Buffer::Retrieve(int len)
{
    assert(len <= readablebytes());
    if (len + read_index_ < write_index_)
    {
        // 如果读的内容不超过可读空间，则只用更新read_index_
        read_index_ += len;
    }
    else
    {
        RetrieveAll();
    }
}

std::string Buffer::RetrieveAsString(int len)
{
    assert(len <= write_index_ - read_index_);
    std::string str = PeekAsString(len);
    Retrieve(len);
    return str;
}

std::string Buffer::RetrieveAllAsString()
{
    assert(read_index_ <= write_index_);
    std::string str = PeekAllAsString();
    RetrieveAll();
    return str;
}

void Buffer::RetrieveUtil(const char *end)
{
    assert(end >= beginread());
    assert(end <= beginwrite());
    Retrieve(end - beginread());
}

std::string Buffer::RetrieveUtilAsString(const char *end)
{
    assert(end >= beginread());
    assert(end <= beginwrite());
    std::string str = PeekAsString(end - beginread());
    Retrieve(end - beginread());
    return str;
}