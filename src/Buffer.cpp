#include "Buffer.h"
#include <iostream>

Buffer::Buffer() {}

Buffer::~Buffer() {}

void Buffer::append(const char *str, int len) {
    for (int i = 0; i < len; ++i) {
        if (str[i] == '\0')
            break;
        buffer.push_back(str[i]);
    }
}

ssize_t Buffer::size() {
    return buffer.size();
}

const char* Buffer::c_str() {
    return buffer.c_str();
}

void Buffer::clear() {
    buffer.clear();
}

void Buffer::getline() {
    buffer.clear();
    std::getline(std::cin, buffer);
}

void Buffer::setBuf(const char* str) {
    buffer.clear();
    buffer.append(str);
}