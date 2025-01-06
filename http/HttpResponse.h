#pragma once
#include <map>
#include <string>

class HttpResponse
{
public:
    enum StatusCode
    {
        kUnknown = 1,
        k100Continue = 100,
        k200OK = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k403Forbidden = 403,
        k404NotFound = 404,
        k500InternalServerError = 500,
    };

    HttpResponse(bool on);
    ~HttpResponse();

    void SetStatusCode(StatusCode code) { status_code_ = code; };
    void SetStatusMessage(const std::string &message) { status_message_ = std::move(message); };
    void SetCloseConnection(bool on) { close_connection_ = on;}; 

    void SetContentType(const std::string &content_type) { AddHeader("Content-Type", content_type); };
    void AddHeader(const std::string &key, const std::string &value) { headers_[key] = value; };
    void SetBody(const std::string &body) { body_ = std::move(body); };

    std::string message();

    bool close_connection() const { return close_connection_; }

private:
    bool close_connection_;
    std::map<std::string, std::string> headers_;
    std::string status_message_;
    std::string body_;
    StatusCode status_code_;

};