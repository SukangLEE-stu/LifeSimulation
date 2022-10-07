//
// Created by 李粟康 on 2022/9/10.
//

#include "web.h"
#include "utils.h"
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <signal.h>

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

void BaseWebServer::init(int port, int threadNum) {
    m_port = port;
    m_threadNum = threadNum;

}

void BaseWebServer::eventListen() {
    m_listenFd = socket(PF_INET, SOCK_STREAM, 0);
    if(m_listenFd < 0) {
        throw std::exception();
    }

    //优雅关闭连接
    if (0 == m_OPT_LINGER) {
        struct linger tmp = {0, 1};
        setsockopt(m_listenFd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    } else if (1 == m_OPT_LINGER) {
        struct linger tmp = {1, 1};
        setsockopt(m_listenFd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    sockaddr_in address{};
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenFd, (sockaddr*)&address, sizeof(address));
    if(ret < 0) {
        throw std::exception();
    }
    ret = listen(m_listenFd, 5);
    if(ret < 0) {
        throw std::exception();
    }

    //epoll创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    m_epollFd = epoll_create(5);
    if(m_epollFd != -1) {
        throw std::exception();
    }
    Utils* utils = Utils::getInstance();
    utils->addFd(m_epollFd, m_listenFd, false, m_listenTrigMode);
    HttpConnection::m_epollFd = m_epollFd;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipeFd);
    if(ret != -1) {
        throw std::exception();
    }

    utils->setNonBlocking(m_pipeFd[1]);
    utils->addFd(m_epollFd, m_pipeFd[0], false, 0);

    utils->addSig(SIGPIPE, SIG_IGN);
    utils->addSig(SIGALRM, Utils::sigHandler, false);
    utils->addSig(SIGTERM, Utils::sigHandler, false);

    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    utils->setFd(m_pipeFd, m_epollFd);
}

void BaseWebServer::eventLoop() {
    bool timeout = false;
    bool stop_server = false;

    while (!stop_server)
    {
        int number = epoll_wait(m_epollFd, m_events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR) {
            // LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++) {
            int sockfd = m_events[i].data.fd;

            //处理新到的客户连接
            if (sockfd == m_listenFd) {
                bool flag = doConnect();
                if (!flag)  continue;
            } else if (m_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                //服务器端关闭连接，移除对应的定时器
                util_timer *timer = users_timer[sockfd].timer;
                deal_timer(timer, sockfd);
            }
                //处理信号
            else if ((sockfd == m_pipeFd[0]) && (m_events[i].events & EPOLLIN)) {
                bool flag = dealwithsignal(timeout, stop_server);
                // if (!flag)  LOG_ERROR("%s", "dealclientdata failure");
            }
                //处理客户连接上接收到的数据
            else if (m_events[i].events & EPOLLIN) {
                dealwithread(sockfd);
            }
            else if (m_events[i].events & EPOLLOUT) {
                dealwithwrite(sockfd);
            }
        }
        if (timeout) {
            utils.timer_handler();
            // LOG_INFO("%s", "timer tick");

            timeout = false;
        }
    }
}

bool BaseWebServer::doConnect() {
    sockaddr_in client_address{};
    socklen_t client_addrlength = sizeof(client_address);
    if (0 == m_listenTrigMode) {
        int connfd = accept(m_listenFd, (sockaddr*)&client_address, &client_addrlength);
        if (connfd < 0) {
            // LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }
        if (HttpConnection::m_userCount >= MAX_FD) {
            // utils.show_error(connfd, "Internal server busy");
            // LOG_ERROR("%s", "Internal server busy");
            return false;
        }
        m_doAdd(connfd, client_address);
    } else {
        while (true) {
            int connfd = accept(m_listenFd, (sockaddr *)&client_address, &client_addrlength);
            if (connfd < 0) {
                // LOG_ERROR("%s:errno is:%d", "accept error", errno);
                break;
            } if (HttpConnection::m_userCount >= MAX_FD) {
                //utils.show_error(connfd, "Internal server busy");
                //LOG_ERROR("%s", "Internal server busy");
                break;
            }
            m_doAdd(connfd, client_address);
        }
        return false;
    }
    return true;
}

void BaseWebServer::m_doAdd(int fd, sockaddr_in addr) {
    // 最后三项均为sql相关
    m_users[fd].init(fd, addr, m_root, m_connectionTrigMode, m_closeLog, "", "", "");

    /*
    // 初始化client_data数据
    // 创建定时器，设置回调函数和超时时间，绑定用户数据，将定时器添加到链表中
    users_timer[fd].address = client_address;
    users_timer[fd].sockfd = fd;
    util_timer *timer = new util_timer;
    timer->user_data = &users_timer[fd];
    timer->cb_func = cb_func;
    time_t cur = time(nullptr);
    timer->expire = cur + 3 * TIMESLOT;
    users_timer[fd].timer = timer;
    utils.m_timer_lst.add_timer(timer);
    */
}





