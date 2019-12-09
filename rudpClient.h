//
// Created by wushuohan on 19-11-9.
//

#ifndef INTERNET_RUDPCLIENT_H
#define INTERNET_RUDPCLIENT_H

#include "rudpServer.h"


class rudpClient {
public:
    rudpClient() : port_(9090),ipaddr_("127.0.0.1"){

    }

    void create_client_socket (int port, const char* ipaddr);

    bool send(std::string s);


private:
    struct sockaddr_in sock_serv;
    int fd_;
    char buf[BUFFERSIZE];
    int port_;
    const char *ipaddr_;


};

#endif //INTERNET_RUDPCLIENT_H
