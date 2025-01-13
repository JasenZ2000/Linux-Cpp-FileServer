#include "HttpRequest.h"
#include <iostream>

HttpRequest::HttpRequest() : method_(kINVALID_METHOD), version_(kINVALID_VERSION) {};

HttpRequest::~HttpRequest() {};

void HttpRequest::SetMethod(const std::string &method)
{
    if (method == "GET")
    {
        method_ = kGET;
    }
    else if (method == "POST")
    {
        method_ = kPOST;
    }
    else if (method == "HEAD")
    {
        method_ = kHEAD;
    }
    else if (method == "PUT")
    {
        method_ = kPUT;
    }
    else if (method == "DELETE")
    {
        method_ = kDELETE;
    }
    else
    {
        method_ = kINVALID_METHOD;
    }
}

HttpRequest::Method HttpRequest::GetMethod() const{ return method_; }

std::string HttpRequest::GetMethodString() const
{
    if (method_ == kGET)
    {
        return "GET";
    }
    else if (method_ == kPOST)
    {
        return "POST";
    }
    else if (method_ == kHEAD)
    {
        return "HEAD";
    }
    else if (method_ == kPUT)
    {
        return "PUT";
    }
    else if (method_ == kDELETE)
    {
        return "DELETE";
    }
    else
    {
        return "INVALID_METHOD";
    }
}

void HttpRequest::SetVersion(const std::string &version)
{
    if (version == "HTTP/1.0")
    {
        version_ = kHTTP_1_0;
    }
    else if (version == "HTTP/1.1")
    {
        version_ = kHTTP_1_1;
    }
    else
    {
        version_ = kINVALID_VERSION;
    }
}

HttpRequest::Version HttpRequest::GetVersion() const{ return version_; }

std::string HttpRequest::GetVersionString() const
{
    if (version_ == kHTTP_1_0)
    {
        return "http1.0";
    }
    else if (version_ == kHTTP_1_1)
    {
        return "http1.1";
    }
    else
    {
        return "INVALID_VERSION";
    }
}

void HttpRequest::SetUrl(const std::string &url) { url_ = std::move(url); }

const std::string &HttpRequest::GetUrl() const{ return url_; }

void HttpRequest::SetRequestParams(const std::string &key, const std::string &value)
{
    request_params_[key] = value;
}

std::string HttpRequest::GetRequestParamsString(const std::string &key) const
{
    std::string res; // !
    if (request_params_.find(key) != request_params_.end())
    {
        res = request_params_.at(key);
    }
    return res;
}

const std::map<std::string, std::string> &HttpRequest::GetRequestParams() const
{
    return request_params_;
}

void HttpRequest::SetProtocol(const std::string &protocol) { protocol_ = std::move(protocol); }

const std::string &HttpRequest::GetProtocol() const{ return protocol_; }

void HttpRequest::SetHeader(const std::string &key, const std::string &value)
{
    headers_[key] = value;
}

std::string HttpRequest::GetHeaderString(const std::string &key) const
{
    std::string res; //!
    if (headers_.find(key)!= headers_.end())
    {
        res = headers_.at(key);
    }
    return res;
}

const std::map<std::string, std::string> &HttpRequest::GetHeaders() const{ return headers_; }

void HttpRequest::SetBody(const std::string &body) { body_ = std::move(body); }

const std::string &HttpRequest::GetBody() const{ return body_; }

void HttpRequest::AddBody(const std::string &body)
{
    body_ += body;
}
