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
      readBuffer(new Buffer()) {
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    channel->setCallback(cb);
    channel->enableReading();
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
            printf("finish reading\n");
            printf("message from client fd %d: %s\n", fd, readBuffer->c_str());
            errif(write(fd, readBuffer->c_str(), readBuffer->size()) == -1, "write error");
            readBuffer->clear();
            break;
        }
        else if (bytes_read == 0)
        {
            printf("EOF, client fd %d disconnected\n", fd);
            deleteConnectionCallback(sock);
            break;
        }
    }
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> cb) {
    deleteConnectionCallback = cb;
}