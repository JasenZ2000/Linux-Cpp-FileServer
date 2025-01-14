#include "HttpResponse.h"
#include <sstream>

HttpResponse::HttpResponse(bool on) : close_connection_(on), status_code_(kUnknown) {
    headers_.clear();
    body_.clear();
    content_length_ = 0;
    body_type_ = HTML_TYPE;
    file_fd_ = -1;
    status_message_.clear();
}

HttpResponse::~HttpResponse() {}

std::string HttpResponse::message()
{
    return GetHeaderString() + body_;
}

std::string HttpResponse::GetHeaderString() const
{
    std::stringstream messages;
    messages << "HTTP/1.1 " << status_code_ << " " << status_message_ << "\r\n";
    if (close_connection_)
    {
        messages << "Connection: close\r\n";
    }
    else
    {
        messages << "Connection: Keep-Alive\r\n";
    }
    for (auto &header : headers_)
    {
        messages << header.first << ": " << header.second << "\r\n";
    }

    messages << "Cache-Control: no-cache, no-store, must-revalidate\r\n";
    messages << "\r\n";

    return messages.str();
}