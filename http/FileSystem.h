#pragma once
#include <string>
#include <vector>
#include "HttpRequest.h"
#include "HttpResponse.h"

// 读取文件
std::string ReadFile(const std::string& path);

// 获取当前目录下所有文件的名字
void FindAllFiles(const std::string& path, std::vector<std::string> &filelist);

// 构建filelist.html
std::string BuildFileHtml();

void RemoveFile(const std::string & filename);

void DownloadFile(const std::string &filename, HttpResponse *response);

void HttpResponseCallback(const HttpRequest &request, HttpResponse *response);