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

class EventLoop;
class Socket;
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
    Socket* sock;
    Channel* channel{nullptr};
    std::function<void(Socket*)> deleteConnectionCallback;
    std::function<void(Connection*)> onConnectCallback;
    ConnState state{Invalid};
    Buffer* readBuffer{nullptr};
    Buffer* sendBuffer{nullptr}; 

    void readNonBlocking();
    void writeNonBlocking();
    void readBlocking();
    void writeBlocking();

public:
    Connection(EventLoop*, Socket*);
    ~Connection();

    void connRead();
    void connWrite();

    void setDeleteConnectionCallback(std::function<void(Socket*)> const &callback);
    void setOnConnectionCallback(std::function<void(Connection*)> const &callback);
    
    ConnState getState() const;

    void close();

    void setSendBuffer(const char*);
    void getlineSendBuffer();
    Buffer* getSendBuffer();
    const char* SendBuffer(); 

    Buffer* getReadBuffer();
    const char* ReadBuffer();

    Socket* getSocket();

    void onConnect(void(Connection*));
};