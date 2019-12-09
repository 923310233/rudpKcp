//
// Created by wushuohan on 19-11-9.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <string>
#include "rudpClient.h"
#include "test.h"


int kcp_op(const char *buf, int len, ikcpcb *kcp, void *user) {
    struct kcp_context *ctx = (struct kcp_context *) user;
    return sendto(ctx->fd, buf, len, 0, ctx->addr, ctx->socklen);
}


void rudpClient::create_client_socket(int port,const char *ipaddr) {
    port_ = port;
    ipaddr_ = ipaddr;

    int l;
    int sfd;

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    int n = 4096 * 1024;
    if (setsockopt(sfd, SOL_SOCKET, SO_SNDBUF, &n, sizeof(n)) == -1) {
        printf("Error setting buffer size");
    }
    if (setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)) == -1) {
        printf("Error setting buffer size");
    }

    if (sfd == -1) {
        perror("socket fail");
        return;
    }

    l = sizeof(struct sockaddr_in);
    bzero(&sock_serv, l);

    sock_serv.sin_family = AF_INET;
    sock_serv.sin_port = htons(port);
    if (inet_pton(AF_INET, ipaddr, &sock_serv.sin_addr) == 0) {
        printf("Invalid IP adress\n");
        return;
    }
    fd_ = sfd;
}


int duration (struct timeval *start,struct timeval *stop,struct timeval *delta)
{
    suseconds_t microstart, microstop, microdelta;

    microstart = (suseconds_t) (100000*(start->tv_sec))+ start->tv_usec;
    microstop = (suseconds_t) (100000*(stop->tv_sec))+ stop->tv_usec;
    microdelta = microstop - microstart;

    delta->tv_usec = microdelta%100000;
    delta->tv_sec = (time_t)(microdelta/100000);

    if((*delta).tv_sec < 0 || (*delta).tv_usec < 0)
        return -1;
    else
        return 0;
}



bool rudpClient::send(std::string s) {
    printf("文件：%s",s.c_str());
    struct timeval start, stop, delta;
    int fd;
    char buf[BUFFERSIZE];
    char buf2[BUFFERSIZE];
    off_t count = 0, m, sz;//long
    long int n;
    int x;
    int l = sizeof(struct sockaddr_in);
    struct stat buffer;
    ikcpcb *kcpobj;


    struct kcp_context *ctx = (struct kcp_context *) malloc(sizeof(struct kcp_context));
    ctx->addr = (struct sockaddr *) &sock_serv;
    ctx->socklen = l;
    ctx->fd = fd_;
    kcpobj = ikcp_create(123, ctx);
    ikcp_wndsize(kcpobj, 1024, 1024);
    ikcp_nodelay(kcpobj, 1, 20, 2, 1);
    ikcp_setoutput(kcpobj, kcp_op);
    ikcp_update(kcpobj, iclock());

    if ((fd = open(s.c_str(), O_RDONLY)) == -1) {
        perror("open fail");
        return EXIT_FAILURE;
    }

    bzero(&buf, BUFFERSIZE);

    gettimeofday(&start, NULL);
    n = read(fd, buf, BUFFERSIZE);
    if (n == -1) {
        perror("read fails");
        return EXIT_FAILURE;
    }

    while (n) {
        ikcp_update(kcpobj, iclock());
//        x=recvfrom(fd_,&buf2,BUFFERSIZE,MSG_DONTWAIT,(struct sockaddr *)&sock_serv,&l);

        if (ikcp_waitsnd(kcpobj) < kcpobj->snd_wnd) {
            ikcp_send(kcpobj, buf, n);
            printf("\n");
            printf("send success: %s", buf);
            usleep(20);
        } else {
            usleep(20);
            continue;
        }

        count+=m;
        break;
    }
    
    std::string end = "end";
    uint32_t number = htonl(0xDEADBEAF);
    ikcp_send(kcpobj,end.c_str(),end.size());
    usleep(20);
    ikcp_update(kcpobj,iclock());
    ikcp_flush(kcpobj);
    gettimeofday(&stop,NULL);
    duration(&start,&stop,&delta);

    printf("Total data sent : %lld\n",count);
    printf("send time  : %ld.%d \n",delta.tv_sec,delta.tv_usec);
    ikcp_release(kcpobj);

    close(fd_);
    close(fd);
    return EXIT_SUCCESS;
}