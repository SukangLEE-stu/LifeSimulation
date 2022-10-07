//
// Created by 李粟康 on 2022/9/21.
//

#include <iostream>
#include "myConfig.h"
#include "web.h"

using std::cout;

int main(int argc, char* argv[]) {
    Config config;
    config.parse(argc, argv);

    BaseWebServer webServer;
    webServer.init(config.m_PORT, config.m_thread_num);

    webServer.eventListen();
    webServer.eventLoop();
}