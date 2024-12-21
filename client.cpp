#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

#define BUFFER_SIZE 1024

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    // 小技巧，清空结构体地址中的存储后再进行定义
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr));
    errif(connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1, "socket connect error");

    // message transfer
    while (true) {
        char buf[BUFFER_SIZE];
        bzero(&buf, sizeof(buf));
        scanf("%s", buf);
        ssize_t write_bytes = write(sockfd, buf, sizeof(buf));
        if (write_bytes == -1) {
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        bzero(&buf, sizeof(buf));
        ssize_t ret = read(sockfd, buf, sizeof(buf));
        if (ret == -1) {
            close(sockfd);
            errif(true, "socket read error");
        } else if (ret == 0) {
            printf("server close\n");
            break;
        } else if (ret > 0) {
            printf("server message: %s\n", buf);
        }
    }
    close(sockfd);

    return 0;
}