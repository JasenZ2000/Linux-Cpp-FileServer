#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

class Buffer;

// 文件操作类
class FileHandler
{
public:
    static std::string readFileBinary(const std::string &filepath);
    static std::string readFileBinary(const std::string &filepath, int begin, int end);

    static std::string readFileText(const std::string &filepath);

    static void readFileBinary(const std::string &filepath, Buffer *);
    static void readFileText(const std::string &filepath, Buffer *);

    static void writeFileBinary(const std::string &filepath, const std::string &data);
    static void writeFileText(const std::string &filepath, const std::string &data);

    static void writeFileBinary(const std::string &filepath, Buffer *data, int size);
    static void writeFileText(const std::string &filepath, Buffer *data, int size);

    static bool Exists(const std::string &filepath);
    static size_t getFileSize(const std::string &filepath);

    static bool deleteFile(const std::string &path);
    static bool moveFile(const std::string &from, const std::string &to);

    static std::vector<std::string> ListFiles(const std::string &directory);
};

class PathUtils
{
public:
    static std::string GetFileName(const std::string &filepath);
    static std::string GetExtension(const std::string &filepath);
    static std::string GetDirectory(const std::string &filepath);

    static std::string GetRelativePath(const std::string &from, const std::string &to);
    static std::string GetAbsolutePath(const std::string &relativepath);

    static bool IsAbsolutePath(const std::string &path);

    static bool CreateDirectory(const std::string &path);
    static void RemoveDirectory(const std::string &path);

    static std::string getMimeType(const std::string &filepath);
    static bool IsBinaryFile(const std::string &filepath);
    static const std::unordered_map<std::string, std::string> mime_types;
};

const std::unordered_map<std::string, std::string> PathUtils::mime_types = {
    {".html", "text/html"},
    {".htm", "text/html"},
    {".js", "text/javascript"},
    {".css", "text/css"},
    {".json", "application/json"},
    {".xml", "application/xml"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".gif", "image/gif"},
    {".svg", "image/svg+xml"},
    {".mp4", "video/mp4"},
    {".avi", "video/x-msvideo"},
    {".mp3", "audio/mpeg"},
    {".wav", "audio/wav"},
    {".pdf", "application/pdf"}};
