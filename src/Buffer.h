#pragma once
#include <string>

class Buffer {
private:
    std::string buffer;
public:
    Buffer();
    ~Buffer();

    void append(const char *str, int len);
    ssize_t size();
    const char* c_str();
    void clear();
    void getline();
    void setBuf(const char*);
};