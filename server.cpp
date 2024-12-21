#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8888);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr));
    errif(bind(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");

    //listen(sockfd, SOMAXCONN);
    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    struct sockaddr_in clnt_addr;
    bzero(&clnt_addr, sizeof(clnt_addr));
    socklen_t clnt_addr_len = sizeof(clnt_addr);

    int clnt_sockfd = accept(sockfd, (sockaddr *)&clnt_addr, &clnt_addr_len);
    errif(clnt_sockfd == -1, "socket accept error");

    printf("client ip: %s, port: %d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

    // message transfer
    while (true) {
        char buf[1024];
        bzero(&buf, sizeof(buf));
        ssize_t ret = read(clnt_sockfd, buf, sizeof(buf));
        if (ret == -1) {
            close(clnt_sockfd);
            errif(true, "socket accept error");
        } else if (ret == 0) {
            printf("client fd %d close\n", clnt_sockfd);
            close(clnt_sockfd);
            break;
        } else if (ret > 0) {
            printf("client fd %d message: %s\n", clnt_sockfd, buf);
            write(clnt_sockfd, buf, strlen(buf));
        }
    }
    close(sockfd);
    return 0;
}