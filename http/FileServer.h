#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>

class HttpServer;
class HttpRequest;
class HttpResponse;
class EventLoop;
class TcpServer;

class FileServer
{
public:
    FileServer(const char *ip, int port, std::string root = "../files/", double timeout = 0.0);
    ~FileServer();

    // 处理文件上传，从request -> 硬盘
    void HandleUpload(const HttpRequest &request, HttpResponse *response);

    // 处理文件下载，包括界面和文件，从硬盘 -> response
    void HandleDownload(const HttpRequest &request, HttpResponse *response);

    void BuildRangeDownloadResponse(HttpResponse *response, std::string filepath, int start, int end);

    void BuildDownloadResponse(HttpResponse *response, std::string filepath, bool is_file);

    // 处理文件删除
    void HandleDelete(const HttpRequest &request, HttpResponse *response);

    // 处理文件列表
    void HandleList(const HttpRequest &request, HttpResponse *response);

    // 分析请求
    void AnalyzeRequest(const HttpRequest &request, HttpResponse *response);

    std::string BuildFileHtml();

private:
    EventLoop *loop_;
    std::string root_path_;
    double timeout_;
    std::unique_ptr<HttpServer> http_server_;
};

    // // 处理登录
    // void HandleLogin(const HttpRequest &request, HttpResponse *response);

    // // 处理注册
    // void HandleRegister(const HttpRequest &request, HttpResponse *response);

// 读取文件
// std::string ReadFile(const std::string& path);

// // 获取当前目录下所有文件的名字
// void FindAllFiles(const std::string& path, std::vector<std::string> &filelist);

// // 构建filelist.html
// std::string BuildFileHtml();

// void RemoveFile(const std::string & filename);

// void DownloadFile(const std::string &filename, HttpResponse *response);

// void HttpResponseCallback(const HttpRequest &request, HttpResponse *response);