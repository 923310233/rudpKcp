//
// Created by wushuohan on 19-11-9.
//

#ifndef INTERNET_RUDPSERVER_H
#define INTERNET_RUDPSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include "rudp.h"


#define BUFFERSIZE 1400


struct kcp_context {
    struct sockaddr *addr;
    int socklen;
    int fd;
};


class rudpServer {
public:
    rudpServer() : port_(9090) {
    }

    rudpServer(int port) {
        port_ = port;
    }

    bool start();

    void update();

    bool send(int conv, const char *data, int len);

    bool udpBind();

    void setOption();

    bool read();


private:
    int port_;
    int fd_;
    char buf[BUFFERSIZE];
    struct sockaddr_in sock_server, clt;

    void clear();

};


#endif //INTERNET_RUDPSERVER_H
