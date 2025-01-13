/**
 * @file HttpContext.cpp
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 生气了，重构一个支持增量解析的HttpContext版本
 * @version 0.1
 * @date 2025-01-11
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "HttpContext.h"
#include "HttpRequest.h"

#include <algorithm>
#include <iostream>

HttpContext::HttpContext() : content_length_(0), complete_request_(false)
{
    request_ = std::make_unique<HttpRequest>();
}

HttpContext::~HttpContext() {}

void HttpContext::Reset()
{
    request_.reset(new HttpRequest());
    content_length_ = 0;
    complete_request_ = false;
    complete_headers_ = false;
}

void HttpContext::ClearBuffer()
{
    buffer_.clear();
}

bool HttpContext::IsComplete()
{
    return complete_request_;
}

HttpRequest *HttpContext::GetRequest()
{
    return request_.get();
}

int HttpContext::GetContentLength()
{
    return content_length_;
}

/// @brief 将字符串解析为HttpRequest对象，buffer_中只存储尚未解析的部分
/// @param str 
/// @return 
bool HttpContext::ParseRequest(const std::string &str)
{
    buffer_ += str; // 将字符串追加到缓冲区中，实现增量式处理

    if (!complete_headers_)
    { // 分离请求行和请求头
        size_t header_end = buffer_.find("\r\n\r\n");

        if (header_end == std::string::npos)
            return true; // complete = false, Parse return true: 不完整

        std::string header = buffer_.substr(0, header_end + 2);
        buffer_.erase(0, header_end + 4); // 擦除请求头和空行

        // 解析请求行
        size_t pos = header.find("\r\n");

        if (pos == std::string::npos || pos == 0)
            return false;

        std::string request_line = header.substr(0, pos);

        if (!ParseRequestLine(request_line))
            return false;

        // 解析请求头
        if (!ParseRequestHeaders(header.substr(pos + 2)))
            return false;

        complete_headers_ = true;
    }

    // 遇到chunked编码，需要额外解析请求体
    if (request_->GetHeaders().count("Transfer-Encoding") &&
        request_->GetHeaderString("Transfer-Encoding") == "chunked")
        return ParseChunkedBody();

    if ((request_->GetHeaders().count("Connection") && request_->GetHeaderString("Connection") == "close") 
        || request_->GetVersion() == HttpRequest::Version::kHTTP_1_0)
        content_length_ = buffer_.size();
    else if (request_->GetHeaders().count("Content-Length"))
    {
        try {
            content_length_ = std::stoul(request_->GetHeaderString("Content-Length"));
        } catch (const std::exception &e) {
            return false;
        }

        if (buffer_.size() < content_length_)
            return true; // complete = false, Parse return true: 不完整
    }

    if (content_length_ > 0)
    {
        request_->SetBody(buffer_.substr(0, content_length_));
        buffer_.erase(0, content_length_);
    }

    complete_request_ = true;
    return true;
}

bool HttpContext::ParseRequestLine(const std::string &line)
{
    size_t pos1 = line.find(' ');
    if (pos1 == std::string::npos)
        return false;
    size_t pos2 = line.find(' ', pos1 + 1);
    if (pos2 == std::string::npos)
        return false;

    request_->SetMethod(line.substr(0, pos1));
    request_->SetUrl(line.substr(pos1 + 1, pos2 - pos1 - 1));
    request_->SetVersion(line.substr(pos2 + 1));

    return !(request_->GetMethodString().empty() || request_->GetUrl().empty() || request_->GetVersionString().empty());
}

// 辅助函数：解析头部
bool HttpContext::ParseRequestHeaders(const std::string &header_data)
{
    size_t pos = 0, end;
    while ((end = header_data.find("\r\n", pos)) != std::string::npos)
    {
        std::string line = header_data.substr(pos, end - pos);
        pos = end + 2;

        size_t colon = line.find(':');
        if (colon == std::string::npos)
            return false;

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 2);
        request_->SetHeader(key, value);
    }
    return true;
}

bool HttpContext::ParseChunkedBody()
{
    while (true) {
        // 查找下一个分块长度
        size_t chunk_size_end = buffer_.find("\r\n");
        if (chunk_size_end == std::string::npos) return true;  // 等待更多数据

        // 解析分块长度
        size_t chunk_size = std::stoul(buffer_.substr(0, chunk_size_end), nullptr, 16);
        
        // 检查当前分块是否已在缓存数据中完整
        if (buffer_.size() < chunk_size_end + 2 + chunk_size + 2) return true;

        buffer_.erase(0, chunk_size_end + 2);

        if (chunk_size == 0) {
            complete_request_ = true;  // 结束块
            buffer_.erase(0, 2);
            return true;
        }      

        // 提取 chunk 数据
        request_->AddBody(buffer_.substr(0, chunk_size));
        buffer_.erase(0, chunk_size + 2);
    }
}
