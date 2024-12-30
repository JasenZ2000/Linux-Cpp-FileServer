/**
 * @file Connection.h
 * @author Zasen (zasen2000@buaa.edu.cn)
 * @brief 从包装echo，变为包装了具体的读写响应操作
 * @version 0.1
 * @date 2024-12-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <functional>
#include <memory>
#include "common.h"

class EventLoop;
class Channel;
class Buffer;

enum ConnState {
    Invalid = 1,
    Handshaking,
    Connected,
    Closed,
    Failed,
};

class Connection {
private:
    EventLoop* loop;
    int connid;
    int clntFd;
    std::unique_ptr<Channel> channel;

    std::function<void(int)> deleteConnectionCallback;
    std::function<void(Connection*)> onConnectCallback;
    ConnState state{Invalid};

    std::unique_ptr<Buffer> readBuffer;
    std::unique_ptr<Buffer> sendBuffer; 

    void readNonBlocking();
    void writeNonBlocking();

public:
    DISALLOW_COPY_AND_MOVE(Connection);
    Connection(EventLoop*, int, int);
    ~Connection();

    void connRead();
    void connWrite();
    void connSend(const std::string &msg); // 输出信息
    void connSend(const char *msg);

    void setDeleteConnectionCallback(std::function<void(int)> const &callback);
    void setOnConnectionCallback(std::function<void(Connection*)> const &callback);
    
    ConnState getState() const;

    void close();

    void setSendBuffer(const char*);
    void getlineSendBuffer();

    Buffer* getSendBuffer();
    Buffer* getReadBuffer();

    int getId();
    int getClntFd();

    void onConnect();
    void deleteConnection();

    EventLoop* getLoop();
};