#include "FileSystem.h"

#include <fstream>
#include <dirent.h>
#include "Logger.h"
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

// 读取文件
std::string ReadFile(const std::string& path) {
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) != 0) {
        LOG_ERROR << "File not found: " + path;
    }

    std::ifstream file(path, std::ios::binary);  // 二进制模式
    if (!file) {
        LOG_ERROR << "Failed to open file: " + path;
    }

    // 读取文件内容到string
    std::string content(file_stat.st_size, '\0');
    file.read(&content[0], file_stat.st_size);
    if (!file) {
        LOG_ERROR << "Failed to read the entire file.";
    }

    return content;
}

// 获取当前目录下所有文件的名字
void FindAllFiles(const std::string& path, std::vector<std::string> &filelist){
    DIR *dir;
    struct dirent *dir_entry = NULL;
    if((dir = opendir(path.c_str())) == NULL){
        LOG_ERROR << "Opendir " << path << " failed";
        return;
    }
    
    while((dir_entry = readdir(dir))!= NULL){
        std::string filename = dir_entry->d_name;
        if (filename != "." && filename != ".."){
            filelist.push_back(filename);
        }
            
    }
}

// 构建filelist.html
std::string BuildFileHtml(){
    std::vector<std::string> filelist;
    // 以/files文件夹为例
    FindAllFiles("../files", filelist);

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
    std::string filehtml = ReadFile("../resources/fileserver.html");
    filehtml = filehtml.replace(filehtml.find(tmp), tmp.size(), file);
    return filehtml;
}

void RemoveFile(const std::string & filename){
    int ret = remove(("../files/" + filename).c_str());
    if(ret != 0){
        LOG_ERROR << "删除文件 " << filename << " 失败";
    }
}

void DownloadFile(const std::string &filename, HttpResponse *response){
    int filefd = ::open(("../files/" + filename).c_str(), O_RDONLY);
    if(filefd == -1){
        LOG_ERROR << "OPEN FILE ERROR";
        response->SetStatusCode(HttpResponse::StatusCode::k302MovedTemporarily);
        response->SetStatusMessage("Moved Temporarily");
        response->SetContentType("text/html");
        response->AddHeader("Location", "/fileserver");
    }else{
        // 获取文件信息
        struct stat fileStat;
        fstat(filefd, &fileStat);
        // 设置响应头字段
        response->SetStatusCode(HttpResponse::StatusCode::k200OK);
        response->SetContentLength(fileStat.st_size);
        response->SetContentType("application/octet-stream");
        
        response->SetBodyType(HttpResponse::HttpBodyType::FILE_TYPE);
        response->AddHeader("Transfer-Encoding", "chunked");

        // 设置文件
        response->SetFileFd(filefd);
    }
}

void HttpResponseCallback(const HttpRequest &request, HttpResponse *response)
{
    // LOG_INFO << request.GetMethodString() << " " << request.url();
    std::string url = request.GetUrl();
    if(request.GetMethod() == HttpRequest::Method::kGET){
        
        if(url == "/"){
            std::string body = ReadFile("../resources/index.html");
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
            response->SetContentLength(body.size());
            response->SetBody(body);
            response->SetContentType("text/html");
        }else if(url == "/mhw"){
            std::string body = ReadFile("../resources/mhw.html");
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
            response->SetContentLength(body.size());
            response->SetBody(body);
            response->SetContentType("text/html");
        }else if(url == "/cat.jpg"){
            std::string body = ReadFile("../resources/cat.jpg"); // 图发不出去
            response->SetContentLength(body.size());
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
            response->SetBody(body);
            response->SetContentType("image/jpeg");
        }else if(url == "/fileserver"){
            std::string body = BuildFileHtml();
            response->SetContentLength(body.size());
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
            response->SetBody(body);
            response->SetContentType("text/html");
        }else if(url.substr(0, 7) == "/delete") {
            // 删除特定文件，由于使用get请求，并且会将相应删掉文件的名称放在url中
            RemoveFile(url.substr(8));
            // 发送重定向报文，删除后返回自身应在的位置
            response->SetStatusCode(HttpResponse::StatusCode::k302MovedTemporarily);
            response->SetStatusMessage("Moved Temporarily");
            response->SetContentType("text/html");
            response->AddHeader("Location", "/fileserver");

        }else if(url.substr(0, 9) == "/download"){
            DownloadFile(url.substr(10), response);
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
        }else if(url == "/favicon.ico"){
            std::string body = ReadFile("../resources/cat.jpg");
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
            response->SetBody(body);
            response->SetContentType("image/jpeg");
        }else
        {
            response->SetStatusCode(HttpResponse::StatusCode::k404NotFound);
            response->SetStatusMessage("Not Found");
            response->SetBody("Sorry Not Found\n");
            response->SetCloseConnection(true);
        }
    }
    else if(request.GetMethod() == HttpRequest::Method::kPOST){
        if(url == "/login"){
            // 进入登陆界面
            std::string rqbody = request.GetBody();

            // 解析
            int usernamePos = rqbody.find("username=");
            int passwordPos = rqbody.find("password=");

            usernamePos += 9; // "username="的长度
            passwordPos += 9; // 

            // 找到中间分割符
            size_t usernameEndPos = rqbody.find('&', usernamePos);
            size_t passwordEndPos = rqbody.length();

            // Extract the username and password substrings
            std::string username = rqbody.substr(usernamePos, usernameEndPos - usernamePos);
            std::string password = rqbody.substr(passwordPos, passwordEndPos - passwordPos);

            if (username == "wlgls"){
                response->SetBody("login ok!\n");
            }
            else{
                response->SetBody("error!\n");
            }
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
            response->SetStatusMessage("OK");
            response->SetContentType("text/plain");
        }else if(url == "/upload")
        {
            response->SetStatusCode(HttpResponse::StatusCode::k200OK);
            response->SetStatusMessage("Moved Temporarily");
            response->SetContentType("text/html");
            response->AddHeader("Location", "/fileserver");
        }
    }
    LOG_INFO << "Response Header:\n" << response->GetHeaderString();
    LOG_INFO << "Response BodySize: " << response->GetContentLength();
    return;
}
