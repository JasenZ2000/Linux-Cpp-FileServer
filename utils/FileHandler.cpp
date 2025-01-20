#include "FileHandler.h"
#include "Buffer.h"
// #include "logger.h" // 实战中可以使用日志库记录错误信息，而不是直接抛出异常
#include <filesystem>
#include <stdexcept>
#include <algorithm>

namespace fs = std::filesystem;

std::string PathUtils::GetFileName(const std::string &path)
{
    return fs::path(path).filename().string();
}

std::string PathUtils::GetExtension(const std::string &path)
{
    return fs::path(path).extension().string();
}

std::string PathUtils::GetDirectory(const std::string &path)
{
    return fs::path(path).parent_path().string();
}

bool PathUtils::CreateDirectory(const std::string &path)
{
    try
    {
        return fs::create_directories(fs::path(path));
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to create directory: " + std::string(e.what()));
    }
}

void PathUtils::RemoveDirectory(const std::string &path)
{
    if (!std::filesystem::remove_all(path))
    {
        throw std::runtime_error("Failed to remove directory: " + path);
    }
}

std::string PathUtils::getMimeType(const std::string &filepath)
{
    std::string extension = PathUtils::GetExtension(filepath);
    if (mime_types.find(extension) != mime_types.end())
    {
        return mime_types.at(extension);
    }
    else
    {
        return "application/octet-stream"; 
    }
}

bool PathUtils::IsBinaryFile(const std::string &filepath)
{
    std::string extension = PathUtils::GetExtension(filepath);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension == ".exe" || extension == ".dll" || extension == ".so";
}

size_t FileHandler::getFileSize(const std::string &path)
{
    try
    {
        return fs::file_size(fs::path(path));
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to get file size: " + std::string(e.what()));
    }
    return 0;
}

std::string PathUtils::GetRelativePath(const std::string &from, const std::string &to)
{
    return fs::relative(fs::path(from), fs::path(to)).string();
}

std::string PathUtils::GetAbsolutePath(const std::string &relative_path)
{
    return fs::absolute(fs::path(relative_path)).string();
}

bool PathUtils::IsAbsolutePath(const std::string &path)
{
    return fs::path(path).is_absolute();
}

std::vector<std::string> FileHandler::ListFiles(const std::string &directory)
{
    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(directory))
    {
        files.push_back(entry.path().string());
    }
    return files;
}

bool FileHandler::deleteFile(const std::string &path)
{
    try
    {
        return fs::remove(fs::path(path));
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to remove file: " + std::string(e.what()));
    }
}

bool FileHandler::Exists(const std::string &path)
{
    return std::filesystem::exists(path);
}

bool FileHandler::moveFile(const std::string &from, const std::string &to)
{
    try
    {
        std::filesystem::rename(from, to);
        return true;
    }
    catch (const std::filesystem::filesystem_error &)
    {
        throw std::runtime_error("Failed to move file: " + from);
    }
}

std::string FileHandler::readFileBinary(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + path);
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string buffer(size, '\0');
    file.read(buffer.data(), size);
    file.close();
    return buffer;
}

std::string FileHandler::readFileBinary(const std::string &filepath, int begin, int end)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    file.seekg(begin, std::ios::beg);
    size_t size = end - begin;
    std::string buffer(size, '\0');
    file.read(buffer.data(), size);
    file.close();
    return buffer;
}

std::string FileHandler::readFileText(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str(); 
}

void FileHandler::readFileBinary(const std::string &filepath, Buffer *out)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    out->EnsureWritableBytes(size);
    out->Append(file, size);
}

void FileHandler::readFileText(const std::string &filepath, Buffer *out)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    out->Append(file); 
}

void FileHandler::writeFileBinary(const std::string &filepath, const std::string &data)
{
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    file.write(data.c_str(), data.size());
}

void FileHandler::writeFileText(const std::string &filepath, const std::string &data)
{
    std::ofstream file(filepath);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    file << data;
}

// 这里retrieve了
void FileHandler::writeFileBinary(const std::string &filepath, Buffer *data, int size)
{
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    file.write(data->RetrieveAsString(size).c_str(), size);
}

// 这里retrieve了
void FileHandler::writeFileText(const std::string &filepath, Buffer *data, int size)
{
    std::ofstream file(filepath);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    file << data->RetrieveAsString(size);
}