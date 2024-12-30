/**
 * @file Connection.cpp
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 比想象中的简单
 * @version 0.1
 * @date 2024-12-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "Connection.h"
#include "Channel.h"
#include "Buffer.h"
#include "util.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <memory>

#define READ_BUFFER 1024

Connection::Connection(EventLoop *_loop, int _connid, int _clntFd)
    : loop(_loop),
      connid(_connid),
      clntFd(_clntFd)
{
    if (loop != nullptr) {
        channel = std::make_unique<Channel>(loop, clntFd);
        channel->setReadCallback(std::bind(&Connection::onConnect, this)); // 实时绑定
        // channel->setWriteCallback(std::bind(&Connection::connWrite, this));
        channel->enableReading();
        channel->useET();
        state = ConnState::Connected;
    }
    readBuffer = std::make_unique<Buffer>();
    sendBuffer = std::make_unique<Buffer>();
}

Connection::~Connection()
{
    ::close(clntFd);
}

void Connection::connRead()
{
    assert(state == ConnState::Connected);
    readBuffer->clear();
    readNonBlocking();
}

void Connection::connWrite()
{
    assert(state == ConnState::Connected);
    writeNonBlocking();
    sendBuffer->clear();
}

void Connection::connSend(const std::string &msg)
{
    assert(state == ConnState::Connected);
    sendBuffer->setBuf(msg.c_str());
    connWrite();
}

void Connection::connSend(const char *msg)
{
    assert(state == ConnState::Connected);
    sendBuffer->setBuf(msg);
    connWrite();
}

void Connection::readNonBlocking()
{
    char buf[READ_BUFFER];
    while (true)
    {
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(clntFd, buf, sizeof(buf));
        if (bytes_read > 0)
        {
            readBuffer->append(buf, static_cast<int>(bytes_read));
        }
        else if (bytes_read == -1 && errno == EINTR)
        {
            printf("continue reading\n");
            continue;
        }
        else if (bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            printf("finish reading once\n");
            break;
        }
        else if (bytes_read == 0)
        {
            printf("EOF, client fd %d disconnected\n", clntFd);
            state = ConnState::Closed;
            break;
        }
        else
        {
            printf("Connection::readNonBlocking error\n");
            state = ConnState::Closed;
            break;
        }
    }
}

void Connection::writeNonBlocking() {
    char buf[sendBuffer->size()];
    memcpy(buf, sendBuffer->c_str(), sendBuffer->size());
    int data_size = sendBuffer->size();
    int data_left = data_size;

    while (data_left > 0) {
        ssize_t bytes_write = write(clntFd, buf + data_size - data_left, data_left);
        if (bytes_write == -1 && errno == EINTR) {
            printf("continue writing\n");
            continue;
        }
        else if (bytes_write == -1 && errno == EAGAIN) {
            printf("finish writing once\n");
            break;
        }
        else if (bytes_write == -1) {
            printf("Connection::writeNonBlocking error\n");
            state = ConnState::Closed;
            break;
        }
        data_left -= bytes_write;
    }
}

ConnState Connection::getState() const {
    return state;
}

void Connection::close() {
    if (state == ConnState::Closed)
        return;
    state = ConnState::Closed;
    if (deleteConnectionCallback)
        deleteConnectionCallback(clntFd);
}

void Connection::setSendBuffer(const char* str) {
    sendBuffer->setBuf(str);
}

Buffer* Connection::getSendBuffer() {
    return sendBuffer.get();
}

Buffer* Connection::getReadBuffer() {
    return readBuffer.get();
}

void Connection::setDeleteConnectionCallback(std::function<void(int)> const &cb)
{
    deleteConnectionCallback = std::move(cb);
}

void Connection::setOnConnectionCallback(std::function<void(Connection *)> const &cb) {
    onConnectCallback = std::move(cb);
}

void Connection::getlineSendBuffer() {
    sendBuffer->getline();
}

int Connection::getId() {
    return connid;
}

int Connection::getClntFd() {
    return clntFd;
}

EventLoop* Connection::getLoop() {
    return loop;
}

void Connection::onConnect() {
    connRead();
    if (onConnectCallback)
        onConnectCallback(this);
}

void Connection::deleteConnection() {
    //std::cout << CurrentThread::tid() << " TcpConnection::HandleClose" << std::endl;
    if (state != ConnState::Closed)
    {
        state = ConnState::Closed;
        if(deleteConnectionCallback){
            deleteConnectionCallback(clntFd);
        }
    }
}