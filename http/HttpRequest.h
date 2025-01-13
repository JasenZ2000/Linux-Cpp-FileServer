#pragma once
#include <string>
#include <map>

class HttpRequest
{
public:
    enum Method
    {
        kGET,
        kPOST,
        kHEAD,
        kPUT,
        kDELETE,
        kINVALID_METHOD = 0
    };
    enum Version
    {
        kHTTP_1_0,
        kHTTP_1_1,
        kINVALID_VERSION = 0
    };

    HttpRequest();
    ~HttpRequest();

    void SetMethod(const std::string &method);
    Method GetMethod() const;
    std::string GetMethodString() const;

    void SetVersion(const std::string &version);
    Version GetVersion() const;
    std::string GetVersionString() const;

    void SetUrl(const std::string &url);
    const std::string &GetUrl() const;

    void SetRequestParams(const std::string &key, const std::string &value);
    std::string GetRequestParamsString(const std::string &key) const;
    const std::map<std::string, std::string> &GetRequestParams() const;

    void SetProtocol(const std::string &protocol);
    const std::string &GetProtocol() const;

    void SetHeader(const std::string &key, const std::string &value);
    std::string GetHeaderString(const std::string &key) const;
    const std::map<std::string, std::string> &GetHeaders() const;

    void SetBody(const std::string &body);
    const std::string &GetBody() const;
    void AddBody(const std::string &body);

private:
    Method method_;
    Version version_;

    std::map<std::string, std::string> request_params_;
    std::string url_;
    std::string protocol_;
    std::map<std::string, std::string> headers_;
    std::string body_;

};   