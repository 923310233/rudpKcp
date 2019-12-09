//
// Created by wushuohan on 19-11-9.
//

#include <iostream>
#include <cstring>
#include "rudpClient.h"


int main() {
    rudpClient client;
    std::string ip = "127.0.0.1";
    client.create_client_socket(9090, ip.c_str());
    std::string s = "/Users/wushuohan/Desktop/index1.html";

    client.send(s);
}