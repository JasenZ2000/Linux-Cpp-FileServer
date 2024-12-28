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
#include "Acceptor.h"
#include "Channel.h"
#include "Socket.h"
#include "Buffer.h"
#include "util.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define READ_BUFFER 1024

Connection::Connection(EventLoop *_loop, Socket *_sock)
    : loop(_loop),
      sock(_sock),
      channel(new Channel(_loop, _sock->getFd())),
      sendBuffer(new Buffer()),
      readBuffer(new Buffer())
{
    // 事件响应独立包装出去了
    // std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    // channel->setReadCallback(cb);
    channel->setUseThreadPool(true);
    channel->enableReading();
    channel->useET();
    state = ConnState::Connected;
}

Connection::~Connection()
{
    delete channel;
    delete sock;
    delete sendBuffer;
    delete readBuffer;
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

void Connection::readNonBlocking()
{
    char buf[READ_BUFFER];
    while (true)
    {
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(sock->getFd(), buf, sizeof(buf));
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
            printf("EOF, client fd %d disconnected\n", sock->getFd());
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
        ssize_t bytes_write = write(sock->getFd(), buf + data_size - data_left, data_left);
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
    state = ConnState::Closed;
    deleteConnectionCallback(sock);
}

void Connection::setSendBuffer(const char* str) {
    sendBuffer->setBuf(str);
}

Buffer* Connection::getSendBuffer() {
    return sendBuffer;
}

Buffer* Connection::getReadBuffer() {
    return readBuffer;
}

const char* Connection::SendBuffer() {
    return sendBuffer->c_str();
}

const char* Connection::ReadBuffer() {
    return readBuffer->c_str();
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> const &cb)
{
    deleteConnectionCallback = cb;
}

void Connection::setOnConnectionCallback(std::function<void(Connection *)> const &callback) {
  onConnectCallback = callback;
  channel->setReadCallback([this]() { onConnectCallback(this); });
}

void Connection::getlineSendBuffer() {
    sendBuffer->getline();
}

Socket* Connection::getSocket() {
    return sock;
}
