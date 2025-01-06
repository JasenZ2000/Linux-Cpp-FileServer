#include "HttpContext.h"
#include "HttpRequest.h"

#include <algorithm>
#include <iostream>

HttpContext::HttpContext() : state_(START)
{
    request_ = std::make_unique<HttpRequest>();
}

HttpContext::~HttpContext() {}

bool HttpContext::ParseRequest(const char *begin, int size)
{
    char *start = const_cast<char *>(begin); // 某个词语头
    char *cur = start;                       // 当前字符，寻找词语尾
    char *keyend = cur;                      // 某个键值对的键的结尾
    char *valbegin = cur;                    // 某个键值对的值的开头

    while (cur <= begin + size && state_ != COMPLETE && state_ != kINVALID)
    {
        char ch = *cur;
        if (state_ == START) // 开始：检测是否是请求方法
        {
            if (ch == CR || ch == LF || isblank(ch))
            {
            }
            else if (isupper(ch))
            {
                state_ = METHOD;
                start = cur;
            }
            else
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == METHOD) // 方法：开始读取方法
        {
            if (isupper(ch))
            {
            }
            else if (isblank(ch))
            {
                request_->SetMethod(std::string(start, cur));
                state_ = BEFORE_URL;
            }
            else
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == BEFORE_URL) // 开始url：检测是否是'/'
        {
            if (ch == '/')
            {
                state_ = IN_URL;
                start = cur + 1;
            }
            else if (isblank(ch))
            {
            }
            else
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == IN_URL) // url：开始读取url
        {
            if (ch == '?')
            {
                request_->SetUrl(std::string(start, cur));
                state_ = BEFORE_URL_PARAM_KEY;
                start = cur + 1;
            }
            else if (isblank(ch))
            {
                request_->SetUrl(std::string(start, cur));
                state_ = BEFORE_PROTOCOL;
            }
            else if (ch == CR || ch == LF)
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == BEFORE_URL_PARAM_KEY) // 检测url参数键：是否是'=' ('?'之后不可直接为'=')
        {
            if (isblank(ch) || ch == CR || ch == LF || ch == '=')
            {
                state_ = kINVALID;
            }
            else
            {
                state_ = URL_PARAM_KEY;
            }
        }
        else if (state_ == URL_PARAM_KEY) // 读取url参数键：开始读取url参数键
        {
            if (ch == '=')
            {
                keyend = cur;
                state_ = BEFORE_URL_PARAM_VALUE;
            }
            else if (isblank(ch))
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == BEFORE_URL_PARAM_VALUE) // 检测url参数值：是否是'='
        {
            if (isblank(ch) || ch == CR || ch == LF)
            {
                state_ = kINVALID;
            }
            else
            {
                state_ = URL_PARAM_VALUE;
            }
        }
        else if (state_ == URL_PARAM_VALUE) // 读取url参数值：开始读取url参数值
        {
            if (isblank(ch))
            {
                request_->SetRequestParams(std::string(start, keyend), std::string(keyend + 1, cur));
                state_ = BEFORE_PROTOCOL;
                // start = cur + 1;
            }
            else if (ch == '&')
            {
                request_->SetRequestParams(std::string(start, keyend), std::string(keyend + 1, cur));
                state_ = BEFORE_URL_PARAM_KEY;
                start = cur + 1;
            }
        }
        else if (state_ == BEFORE_PROTOCOL) // 检测协议：跳过空格
        {
            if (isblank(ch))
            {
            }
            else
            {
                state_ = PROTOCOL;
                start = cur;
            }
        }
        else if (state_ == PROTOCOL) // 读取协议：开始读取协议，直到'/'
        {
            if (ch == '/')
            {
                request_->SetProtocol(std::string(start, cur));
                state_ = BEFORE_VERSION;
                start = cur + 1;
            }
            else
            {
            }
        }
        else if (state_ == BEFORE_VERSION) // 检测版本：必须是数字
        {
            if (isdigit(ch))
            {
                state_ = VERSION;
            }
            else
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == VERSION) // 读取版本：开始读取版本
        {
            if (isdigit(ch) || ch == '.')
            {
            }
            else if (ch == CR)
            {
                state_ = WHEN_CR;
            }
            else
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == WHEN_CR) // 检测回车：必须是'\n'
        {
            if (ch == LF)
            {
                state_ = CR_LF;
            }
            else
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == CR_LF) // 检测换行后：是头还是体
        {
            if (isblank(ch))
            {
            }
            else if (ch == CR)
            {
                state_ = CR_LF_CR;
            }
            else
            {
                state_ = HEADER_KEY;
                start = cur;
            }
        }
        else if (state_ == HEADER_KEY) // 读取键：开始读取键
        {
            if (ch == ':')
            {
                keyend = cur;
                state_ = HEADER_AFTER_COLON;
            }
            else if (isblank(ch))
            {
                keyend = cur;
                state_ = HEADER_BEFORE_COLON;
            }
            else if (ch == CR)
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == HEADER_BEFORE_COLON) // 检测冒号前：必须是空格
        {
            if (isblank(ch))
            {
            }
            else if (ch == ':')
            {
                state_ = HEADER_AFTER_COLON;
            }
            else
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == HEADER_AFTER_COLON) // 检测冒号后：跳过空格直到值
        {
            if (isblank(ch))
            {
            }
            else
            {
                state_ = HEADER_VALUE;
                valbegin = cur;
            }
        }
        else if (state_ == HEADER_VALUE) // 读取值：开始读取值
        {
            if (ch == CR)
            {
                request_->SetHeader(std::string(start, keyend), std::string(valbegin, cur));
                state_ = WHEN_CR;
            }
        }
        else if (state_ == CR_LF_CR) // 检测回车换行后又准备换行
        {
            if (ch == LF)
            {
                if (request_->GetHeaders().count("Content-Length"))
                {
                    int len = std::stoi(request_->GetHeaderString("Content-Length"));
                    if (len > 0)
                        state_ = BODY;
                    else
                        state_ = COMPLETE;
                }
                else
                {
                    if (cur - begin < size)
                        state_ = BODY;
                    else
                        state_ = COMPLETE;
                }
                start = cur + 1;
            }
            else
            {
                state_ = kINVALID;
            }
        }
        else if (state_ == BODY) // 读取body
        {
            int bodylength = size - (cur - begin);
            request_->SetBody(std::string(start, start + bodylength));
            state_ = COMPLETE;
        }
        else
        {
            state_ = kINVALID;
        }
        cur++;
    }
    return state_ == COMPLETE;
}
