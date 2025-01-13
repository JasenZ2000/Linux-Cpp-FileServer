#pragma once
#include <string>
#include <memory>

class HttpRequest;

#define CRLF "\r\n"
#define CR '\r'
#define LF '\n'

class HttpContext
{
public:
    HttpContext();
    ~HttpContext();

    // bool ParseRequest(const char *begin, int size); 反正会自动转换
    bool ParseRequest(const std::string &str);
    HttpRequest *GetRequest();
    bool IsComplete();
    void Reset();
    void ClearBuffer();
    int GetContentLength();

private:
    std::unique_ptr<HttpRequest> request_;
    size_t content_length_;
    bool complete_headers_;
    bool complete_request_;
    std::string buffer_; // 增量式读取

    bool ParseRequestLine(const std::string &str);
    bool ParseRequestHeaders(const std::string &str);
    bool ParseChunkedBody();

};