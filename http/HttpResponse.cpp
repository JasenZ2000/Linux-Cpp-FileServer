#include "HttpResponse.h"
#include <sstream>

HttpResponse::HttpResponse(bool on) : close_connection_(on), status_code_(kUnknown) {}

HttpResponse::~HttpResponse() {}

std::string HttpResponse::message()
{
    std::stringstream messages;
    messages << "HTTP/1.1 " << status_code_ << " " << status_message_ << "\r\n";
    if (close_connection_)
    {
        messages << "Connection: close\r\n";
    }
    else
    {
        messages << "Content-Length: " << body_.size() << "\r\n";
        messages << "Connection: Keep-Alive\r\n";
    }

    for (auto &header : headers_)
    {
        messages << header.first << ": " << header.second << "\r\n";
    }

    messages << "\r\n";
    messages << body_;
    return messages.str();
}