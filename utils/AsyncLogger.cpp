#include "AsyncLogger.h"

AsyncLogger::AsyncLogger(const char* path)
    : filepath_(path),
      running_(false),
      logFile_(std::make_unique<LogFile>(filepath_)),
      buffer_(std::make_unique<AsyncLogger::Buffer>()),
      nextBuffer_(std::make_unique<AsyncLogger::Buffer>()) {}

AsyncLogger::~AsyncLogger() {
    if (running_) {
        Stop();
    }
}

void AsyncLogger::Start() {
    running_ = true;
    thread_ = std::thread(&AsyncLogger::ThreadFunc, this);
}

void AsyncLogger::Stop() {
    running_ = false;
    cond_.notify_one();
    thread_.join();
}

/// @brief 记录日志唯一接口，要求线程安全，即前端线程，对应Output
/// @param logline 
/// @param len 
void AsyncLogger::Append(const char* logline, int len) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (buffer_->avail() > len) {
        buffer_->append(logline, len);
    } else {
        // 当前buffer已满，将其加入待写入队列，置空
        buffers_.push_back(std::move(buffer_));
        if (nextBuffer_) {
            // 复用nextBuffer_
            buffer_ = std::move(nextBuffer_);
        } else {
            // 分配新的buffer
            buffer_.reset(new Buffer);
        }
        buffer_->append(logline, len);
        cond_.notify_one();
    }
}

void AsyncLogger::Flush()
{
    logFile_->Flush();
}

/// @brief 后端线程，对应Input，收到信号后将待写入队列中的buffer写入文件
void AsyncLogger::ThreadFunc() {
    // 每次读入buffer时必须为空
    std::vector<std::unique_ptr<Buffer>> active_buffers;

    std::unique_ptr<Buffer> new_current = std::make_unique<Buffer>();
    std::unique_ptr<Buffer> new_next = std::make_unique<Buffer>();

    std::string newFilePath;

    while (running_) {
        // 阻塞区间：将待写入队列中的buffer取出至后端线程，更新可写Buffer_
        {
        std::unique_lock<std::mutex> lock(mutex_);
        if (buffers_.empty()) {
            cond_.wait_for(lock, LoggerBufferTimeout * std::chrono::milliseconds(1000), [this] { return !buffers_.empty(); });
        }
        buffers_.push_back(std::move(buffer_));
        active_buffers.swap(buffers_);

        if (!buffer_ && new_current) {
            buffer_ = std::move(new_current);
            buffer_->reset();
        }

        if (!nextBuffer_ && new_next) {
            nextBuffer_ = std::move(new_next);
            nextBuffer_->reset();
        }

        }

        // 非阻塞区间：将待写入队列中的buffer写入文件
        for (auto& buffer : active_buffers) {
            logFile_->Write(buffer->data(), buffer->len());
        }

        if (logFile_->WrittenBytes() > LoggerFileMaxSize)
        {
            newFilePath = logFile_->RollFile();
            filepath_ = newFilePath.data();
            logFile_.reset(new LogFile(filepath_));
        }

        if (!active_buffers.empty())
        {
            new_current = std::move(active_buffers.back());
            active_buffers.pop_back();
            new_current->reset();
        }
        else
        {
            new_current = std::make_unique<Buffer>();
        }

        if (!active_buffers.empty())
        {
            new_next = std::move(active_buffers.back());
            active_buffers.pop_back();
            new_next->reset();
        }
        else
        {
            new_next = std::make_unique<Buffer>();
        }

    }

    for (auto& buffer : buffers_) {
        logFile_->Write(buffer->data(), buffer->len());
        buffer->reset();
    }

    if (buffer_) {
        logFile_->Write(buffer_->data(), buffer_->len());
        buffer_->reset();
    }

    logFile_->Flush();
}