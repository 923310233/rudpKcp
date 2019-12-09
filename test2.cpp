//
// Created by wushuohan on 19-11-9.
//
#include <iostream>
#include "rudpServer.h"

void isleep(unsigned long millisecond)
{
    usleep((millisecond << 10) - (millisecond << 4) - (millisecond << 3));
}


int main(){
    rudpServer server(9090);
    server.udpBind();
    server.setOption();
    while(1){
        isleep(5);
        server.read();
    }
}