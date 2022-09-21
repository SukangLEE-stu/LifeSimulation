//
// Created by 李粟康 on 2022/9/10.
//

#include "web.h"

#include <iostream>

BaseWebServer::BaseWebServer() :
    m_port(-1),
    m_epollFd(-1),
    m_closeLog(-1),
    m_actorModel(-1),
    m_OPT_LINGER(-1),
    m_connectionTrigMode(-1),
    m_listenFd(-1),
    m_listenTrigMode(-1),
    m_logWrite(-1),
    m_root(nullptr),
    m_threadNum(0),
    m_threadPool(nullptr),
    m_trigMode(-1) {
    m_pipeFd[0] = -1;
    m_pipeFd[1] = -1;
}







