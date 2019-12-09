//
// Created by wushuohan on 19-11-9.
//

#include <cstring>
#include <string>
#include "rudpServer.h"
#include "test.h"



bool contains(std::string s1, std::string s2) {
    if (s1.find(s2) != std::string::npos) {
        return true;
    }
    return false;
}


bool rudpServer::send(int conv, const char *data, int len) {

}

void rudpServer::setOption() {
    int s = 4096 * 1024;
    if (setsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &s, sizeof(s)) == -1) {
        printf("Error setting buffer size");
    }
    if (setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &s, sizeof(s)) == -1) {
        printf("Error setting buffer size");
    }

}

bool rudpServer::udpBind() {
    int length;
    int sfd;

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1) {
        perror("create socket fail");
        return false;
    }

    //preparation de l'adresse de la socket destination
    length = sizeof(struct sockaddr_in);
    bzero(&sock_server, length);

    sock_server.sin_family = AF_INET;
    sock_server.sin_port = htons(port_);
    sock_server.sin_addr.s_addr = htonl(INADDR_ANY);

    //Affecter une identité au socket
    if (bind(sfd, (struct sockaddr *) &sock_server, length) < 0) {
        perror("bind fail");
        return false;
    }
    fd_ = sfd;
    return true;
}

int kcp_op(const char *buf, int len, ikcpcb *kcp, void *user) {
    struct kcp_context *ctx = (struct kcp_context *) user;
    return sendto(ctx->fd, buf, len, 0, ctx->addr, ctx->socklen);
}


bool rudpServer::read() {
    long count = 0, n, m; // long type
    char filename[256];
    unsigned int l = sizeof(struct sockaddr_in);
    std::string end = "end";

    time_t intps;
    time_t st, et;

    bzero(&buf, BUFFERSIZE);

    struct kcp_context *ctx = (struct kcp_context *) malloc(sizeof(struct kcp_context));


    ctx->addr = (struct sockaddr *) &clt;
    ctx->socklen = l;
    ctx->fd = fd_;

    ikcpcb *kcpobj = ikcp_create(123, ctx);
    ikcp_setoutput(kcpobj, kcp_op);
    ikcp_nodelay(kcpobj, 1, 20, 2, 1);
    ikcp_wndsize(kcpobj, 32, 1024);
    ikcp_update(kcpobj, iclock());
//    ikcp_input(kcpobj, buf, n);

    bzero(&clt, l);

    while (true) {
        printf("RECV\n");
        bzero(buf, BUFFERSIZE);
        n = recvfrom(fd_, &buf, BUFFERSIZE, 0, (struct sockaddr *) &clt, &l);
        printf("TEST:%d \n", n);
        if (n < 0) {
            printf("NOT Receive");
            break;
        }
        buf[n] = '\0';

        ikcp_input(kcpobj, buf, n);
        ikcp_update(kcpobj, iclock());
        // 用户获得收到的数据
        m = ikcp_recv(kcpobj, buf, BUFFERSIZE);
        if (m > 0) {
            buf[m] = '\0';
            printf("Receive Data: \n");
            printf("%s    %d", buf, m);

            std::string data(buf);
            if (count == 0 && m > 0) {
                st = time(NULL);
            }
            
            count += m;
            if (contains(data, end)) {
                printf("receive end");
                break;
            }
        }
    }

//    int ocount = 0;
//    int last = 0;
//    last = ikcp_waitsnd(kcpobj);
//    while (1) {
//        int now = ikcp_waitsnd(kcpobj);
//        if (last != now || ocount < 50) {
//            ikcp_update(kcpobj, iclock());
//            n = recvfrom(fd_, &buf, BUFFERSIZE, MSG_DONTWAIT, (struct sockaddr *) &sock_server, &l);
//            printf("Window %d,last %d\n", now, last);
//            if (n > 0) {
//                ikcp_input(kcpobj, buf, n);
//
//            }
//            if (last == now)
//                ocount += 1;
//            else {
//                last = now;
//                count = 0;
//            }
//            usleep(20);
//        } else {
//            break;
//        }
//    }

    et = time(NULL);
    printf("Total received data : %lld ,speed %lld K /s \n", count, count / 1024 / (et - st));

    ikcp_release(kcpobj);
    close(fd_);
    return true;
}