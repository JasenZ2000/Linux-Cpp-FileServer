#include "FileServer.h"
#include "FileHandler.h"

#include "EventLoop.h"
#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <fstream>
#include <dirent.h>
#include "Logger.h"
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


FileServer::FileServer(const char *ip, int port, std::string root, double timeout) 
    : loop_(new EventLoop()), root_path_(root), timeout_(timeout){
    http_server_ = std::make_unique<HttpServer>(loop_, ip, port, timeout_);
    http_server_->set_http_callback(std::bind(&FileServer::AnalyzeRequest, this, std::placeholders::_1, std::placeholders::_2));
    http_server_->Start();
}

FileServer::~FileServer(){}

void FileServer::AnalyzeRequest(const HttpRequest &request, HttpResponse *response){
    std::string path = request.GetUrl();
    HttpRequest::Method method = request.GetMethod();
    LOG_DEBUG << "method: " << method << " path: " << path;

    if (path == "/"){
        HandleList(request, response);
    }
    else if (path == "/favicon.ico"){
        BuildDownloadResponse(response, "../resources/cat.jpg", false);
    }
    else if (method == HttpRequest::Method::kGET){
        HandleDownload(request, response);
    }
    else if (method == HttpRequest::Method::kPOST){
        HandleUpload(request, response); 
    }
    else if (method == HttpRequest::Method::kDELETE){
        HandleDelete(request, response); 
    }
    else{
        response->SetStatusCode(HttpResponse::StatusCode::k404NotFound);
        response->SetStatusMessage("Not Found");
        response->SetCloseConnection(true); 
    }
}

void FileServer::HandleDownload(const HttpRequest &request, HttpResponse *response){
    std::string path = request.GetUrl();
    std::string filename = path.substr(path.find_last_of("/") + 1);
    std::string filepath = root_path_ + filename;
    LOG_DEBUG << "filename: " << filename << " filepath: " << filepath;
    if (FileHandler::Exists(filepath) && request.GetHeaders().count("Range") > 0){
        std::string range = request.GetHeaders().at("Range");
        int start = std::stoi(range.substr(range.find("=") + 1, range.find("-") - range.find("=") - 1));
        int end = std::stoi(range.substr(range.find("-") + 1));
        BuildRangeDownloadResponse(response, filepath, true, start, end); 
    }
    else{
        BuildDownloadResponse(response, filepath, true); 
    }
}

void Fileserver::BuildRangeDownloadResponse(HttpResponse *response, std::string filepath, int start, int end)
{
    response->SetStatusCode(HttpResponse::StatusCode::k206PartialContent);
    response->SetStatusMessage("Partial Content");
    response->SetContentType(PathUtils::getMimeType(filepath));
    response->SetContentLength(end - start + 1);
    response->SetBody(FileHandler::readFileBinary(filepath, start, end));
    response->AddHeader("Cache-Control", "no-cache");
    response->AddHeader("Content-Range", "bytes " + std::to_string(start) + "-" + std::to_string(end) + "/" + std::to_string(FileHandler::getFileSize(filepath)));
}

void FileServer::BuildDownloadResponse(HttpResponse *response, std::string filepath, bool is_file)
{
    response->SetStatusCode(HttpResponse::StatusCode::k200OK);
    response->SetStatusMessage("OK");
    response->SetContentType(PathUtils::getMimeType(filepath));
    response->SetBody(FileHandler::readFileBinary(filepath));
    response->SetContentLength(FileHandler::getFileSize(filepath));

    if (is_file){
        response->AddHeader("Content-Disposition", "attachment; filename=" + PathUtils::GetFileName(filepath));
    }
    else{
        response->AddHeader("Content-Disposition", "inline"); 
    }

    response->AddHeader("Cache-Control", "no-cache");
}

void FileServer::HandleUpload(const HttpRequest &request, HttpResponse *response){
    std::string path = request.GetUrl();
    std::string filename = path.substr(path.find_last_of("/") + 1);
    std::string filepath = root_path_ + filename;
    LOG_DEBUG << "filename: " << filename << " filepath: " << filepath;
    if (FileHandler::Exists(filepath)){
        response->SetStatusCode(HttpResponse::StatusCode::k409Conflict);
        response->SetStatusMessage("Conflict");
    } 
    else{
        std::string file_content = request.GetBody();
        FileHandler::writeFileBinary(filepath, file_content);
        response->SetStatusCode(HttpResponse::StatusCode::k201Created);
        response->SetStatusMessage("Created");
    }
}

void FileServer::HandleDelete(const HttpRequest &request, HttpResponse *response){
    std::string path = request.GetUrl();
    std::string filename = path.substr(path.find_last_of("/") + 1);
    std::string filepath = root_path_ + filename;
    LOG_DEBUG << "filename: " << filename << " filepath: " << filepath;
    if (FileHandler::Exists(filepath)){
        FileHandler::deleteFile(filename); 
    } 
}

void FileServer::HandleList(const HttpRequest &request, HttpResponse *response){
    std::string filehtml = BuildFileHtml();
    response->SetStatusCode(HttpResponse::StatusCode::k200OK);
    response->SetStatusMessage("OK"); 
    response->SetContentType("text/html");
    response->SetContentLength(filehtml.size());
    response->AddHeader("Content-Disposition", "inline"); 
    response->SetBody(filehtml);
}

// 构建filelist.html
std::string FileServer::BuildFileHtml(){
    std::vector<std::string> filelist = FileHandler::ListFiles("../files");

    // 为文件生成模板
    std::string file = "";
    for (auto filename : filelist)
    {
        //将fileitem中的所有filename替换成
        file += "<tr><td>" + filename + "</td>" +
                "<td>" +
                "<a href=\"/download/" + filename + "\">下载</a>" +
                "<a href=\"/delete/" + filename + "\">删除</a>" +
                "</td></tr>" + "\n";
    }

    //生成html页面
    // 主要通过将<!--filelist-->直接进行替换实现
    std::string tmp = "<!--filelist-->";
    std::string filehtml = FileHandler::readFileText("../resources/fileserver.html");
    filehtml = filehtml.replace(filehtml.find(tmp), tmp.size(), file);
    return filehtml;
}

// void HttpResponseCallback(const HttpRequest &request, HttpResponse *response)
// {
//     // LOG_INFO << request.GetMethodString() << " " << request.url();
//     std::string url = request.GetUrl();
//     if(request.GetMethod() == HttpRequest::Method::kGET){
        
//         if(url == "/"){
//             std::string body = ReadFile("../resources/index.html");
//             response->SetStatusCode(HttpResponse::StatusCode::k200OK);
//             response->SetContentLength(body.size());
//             response->SetBody(body);
//             response->SetContentType("text/html");
//         }else if(url == "/mhw"){
//             std::string body = ReadFile("../resources/mhw.html");
//             response->SetStatusCode(HttpResponse::StatusCode::k200OK);
//             response->SetContentLength(body.size());
//             response->SetBody(body);
//             response->SetContentType("text/html");
//         }else if(url == "/cat.jpg"){
//             std::string body = ReadFile("../resources/cat.jpg"); // 图发不出去
//             response->SetContentLength(body.size());
//             response->SetStatusCode(HttpResponse::StatusCode::k200OK);
//             response->SetBody(body);
//             response->SetContentType("image/jpeg");
//         }else if(url == "/fileserver"){
//             std::string body = BuildFileHtml();
//             response->SetContentLength(body.size());
//             response->SetStatusCode(HttpResponse::StatusCode::k200OK);
//             response->SetBody(body);
//             response->SetContentType("text/html");
//         }else if(url.substr(0, 7) == "/delete") {
//             // 删除特定文件，由于使用get请求，并且会将相应删掉文件的名称放在url中
//             RemoveFile(url.substr(8));
//             // 发送重定向报文，删除后返回自身应在的位置
//             response->SetStatusCode(HttpResponse::StatusCode::k302MovedTemporarily);
//             response->SetStatusMessage("Moved Temporarily");
//             response->SetContentType("text/html");
//             response->AddHeader("Location", "/fileserver");

//         }else if(url.substr(0, 9) == "/download"){
//             DownloadFile(url.substr(10), response);
//             response->SetStatusCode(HttpResponse::StatusCode::k200OK);
//         }else if(url == "/favicon.ico"){
//             std::string body = ReadFile("../resources/cat.jpg");
//             response->SetStatusCode(HttpResponse::StatusCode::k200OK);
//             response->SetBody(body);
//             response->SetContentType("image/jpeg");
//         }else
//         {
//             response->SetStatusCode(HttpResponse::StatusCode::k404NotFound);
//             response->SetStatusMessage("Not Found");
//             response->SetBody("Sorry Not Found\n");
//             response->SetCloseConnection(true);
//         }
//     }
//     else if(request.GetMethod() == HttpRequest::Method::kPOST){
//         if(url == "/login"){
//             // 进入登陆界面
//             std::string rqbody = request.GetBody();

//             // 解析
//             int usernamePos = rqbody.find("username=");
//             int passwordPos = rqbody.find("password=");

//             usernamePos += 9; // "username="的长度
//             passwordPos += 9; // 

//             // 找到中间分割符
//             size_t usernameEndPos = rqbody.find('&', usernamePos);
//             size_t passwordEndPos = rqbody.length();

//             // Extract the username and password substrings
//             std::string username = rqbody.substr(usernamePos, usernameEndPos - usernamePos);
//             std::string password = rqbody.substr(passwordPos, passwordEndPos - passwordPos);

//             if (username == "wlgls"){
//                 response->SetBody("login ok!\n");
//             }
//             else{
//                 response->SetBody("error!\n");
//             }
//             response->SetStatusCode(HttpResponse::StatusCode::k200OK);
//             response->SetStatusMessage("OK");
//             response->SetContentType("text/plain");
//         }else if(url == "/upload")
//         {
//             response->SetStatusCode(HttpResponse::StatusCode::k200OK);
//             response->SetStatusMessage("Moved Temporarily");
//             response->SetContentType("text/html");
//             response->AddHeader("Location", "/fileserver");
//         }
//     }
//     LOG_INFO << "Response Header:\n" << response->GetHeaderString();
//     LOG_INFO << "Response BodySize: " << response->GetContentLength();
//     return;
// }
