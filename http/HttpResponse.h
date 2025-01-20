#pragma once
#include <unordered_map>
#include <string>

class HttpResponse
{
public:
    enum StatusCode
    {
        kUnknown = 1,
        k100Continue = 100,
        k200OK = 200,
        k201Created = 201,
        k206PartialContent = 206,
        k301MovedPermanently = 301,
        k302MovedTemporarily = 302,
        k400BadRequest = 400,
        k403Forbidden = 403,
        k404NotFound = 404,
        k409Conflict = 409,
        k500InternalServerError = 500,
    };

    enum HttpBodyType
    {
        HTML_TYPE,
        TEXT_TYPE,
        FILE_TYPE,
    };

    HttpResponse(bool on);
    ~HttpResponse();

    void SetStatusCode(StatusCode code) { status_code_ = code; };
    void SetStatusMessage(const std::string &message) { status_message_ = std::move(message); };
    void SetCloseConnection(bool on) { close_connection_ = on;}; 

    void SetContentType(const std::string &content_type) { AddHeader("Content-Type", content_type); };
    void SetContentLength(size_t length) { AddHeader("Content-Length", std::to_string(length)); content_length_ = length; };
    int GetContentLength() const { return content_length_;};

    void AddHeader(const std::string &key, const std::string &value) { headers_[key] = value; };
    void SetBody(const std::string &body) { body_ = std::move(body); };

    std::string message();
    std::string GetHeaderString() const; // 单独获取头部

    bool close_connection() const { return close_connection_; }
    int GetFileFd() const { return file_fd_; }
    HttpBodyType GetBodyType() const { return body_type_; }
    void SetFileFd(int fd) { file_fd_ = fd; }
    void SetBodyType(HttpBodyType type) { body_type_ = type; }

private:
    bool close_connection_;
    std::unordered_map<std::string, std::string> headers_;
    std::string status_message_;
    std::string body_;

    StatusCode status_code_;
    int content_length_;
    HttpBodyType body_type_;
    int file_fd_;

};