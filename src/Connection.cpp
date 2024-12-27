#include "Connection.h"
#include "Acceptor.h"
#include "Channel.h"
#include "Socket.h"
#include "Buffer.h"
#include "util.h"
#include <string.h>
#include <unistd.h>

#define READ_BUFFER 1024

Connection::Connection(EventLoop* _loop, Socket* _sock)
    : loop(_loop),
      sock(_sock),
      channel(new Channel(_loop, _sock->getFd())),
      inBuffer(new Buffer()),
      readBuffer(new Buffer()) {
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setReadCallback(cb);
    channel->setUseThreadPool(true);
    channel->enableReading();
    channel->useET();
}

Connection::~Connection() {
    delete channel;
    delete sock;
    delete readBuffer;
}

void Connection::echo(int fd)
{
    char buf[READ_BUFFER];

    while (true)
    {
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = read(fd, buf, sizeof(buf));
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
            // printf("finish reading\n");
            printf("message from client fd %d: %s\n", fd, readBuffer->c_str());
            send(fd);
            //errif(write(fd, readBuffer->c_str(), readBuffer->size()) == -1, "write error");
            readBuffer->clear();
            break;
        }
        else if (bytes_read == 0)
        {
            printf("EOF, client fd %d disconnected\n", fd);
            deleteConnectionCallback(sock->getFd());
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(int)> cb) {
    deleteConnectionCallback = cb;
}

void Connection::send(int sockfd){
    char buf[readBuffer->size()];
    strcpy(buf, readBuffer->c_str());
    int  data_size = readBuffer->size(); 
    int data_left = data_size; 
    while (data_left > 0) 
    { 
        ssize_t bytes_write = write(sockfd, buf + data_size - data_left, data_left); 
        if (bytes_write == -1 && errno == EAGAIN) { 
            break;
        }
        data_left -= bytes_write; 
    }
}