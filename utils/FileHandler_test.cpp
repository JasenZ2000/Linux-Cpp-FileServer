#include <gtest/gtest.h>
#include "utils/FileHandler.h"

TEST(FileHandlerTest, ReadFileBinary) {
    // 测试读取二进制文件
    std::string filePath = "test.bin";
    std::vector<char> data = FileHandler::readFileBinary(filePath);
    // 可以添加对读取结果的验证
}

TEST(FileHandlerTest, ReadFileText) {
    // 测试读取文本文件
    std::string filePath = "test.txt";
    std::string text = FileHandler::readFileText(filePath);
    // 可以添加对读取结果的验证
}

TEST(FileHandlerTest, WriteFileBinary) {
    // 测试写入二进制文件
    std::string filePath = "output.bin";
    std::vector<char> data = {1, 2, 3, 4, 5};
    FileHandler::writeFileBinary(filePath, data);
    // 可以检查文件是否成功写入
}

TEST(FileHandlerTest, WriteFileText) {
    // 测试写入文本文件
    std::string filePath = "output.txt";
    std::string text = "Hello, World!";
    FileHandler::writeFileText(filePath, text);
    // 可以检查文件是否成功写入
}

TEST(FileHandlerTest, FileExists) {
    // 测试文件是否存在
    std::string filePath = "test.txt";
    bool exists = FileHandler::fileExists(filePath);
    // 可以添加对文件存在性的验证
}

TEST(FileHandlerTest, GetFileSize) {
    // 测试获取文件大小
    std::string filePath = "test.txt";
    size_t size = FileHandler::getFileSize(filePath);
    // 可以添加对文件大小的验证
}

TEST(FileHandlerTest, DeleteFile) {
    // 测试删除文件
    std::string filePath = "test.txt";
    bool deleted = FileHandler::deleteFile(filePath);
    // 可以添加对文件删除结果的验证
}

TEST(FileHandlerTest, MoveFile) {
    // 测试移动文件
    std::string fromPath = "test.txt";
    std::string toPath = "new_test.txt";
    bool moved = FileHandler::moveFile(fromPath, toPath);
    // 可以添加对文件移动结果的验证
}
