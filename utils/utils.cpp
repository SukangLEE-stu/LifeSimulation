//
// Created by 李粟康 on 2022/9/10.
//

#include <iostream>
#include "utils.h"
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <cstring>
#include <cassert>

using std::cout;

Utils* Utils::getInstance() {
    static Utils utils;
    return &utils;
}

Utils::Utils() {
    m_timeSlot = 5;
}

Utils::~Utils() = default;

void Utils::helloWorld() {
    cout<<"hello world!";
}

// 对文件描述符设置非阻塞
int Utils::setNonBlocking(int fd) {
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
}

// 将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void Utils::addFd(int epollFd, int fd, bool oneShot, int TRIGMode) {
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode) {
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    }
    else {
        event.events = EPOLLIN | EPOLLRDHUP;
    }

    if (oneShot) {

    }
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

// 从内核时间表删除描述符
void Utils::removeFd(int epollFd, int fd) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

// 将事件重置为EPOLLONESHOT
void Utils::modFd(int epollFd, int fd, int ev, int TRIGMode) {
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
}

void Utils::sigHandler(int sig) {
    // 为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(m_pipeFd[1], (char*)&msg, 1, 0);
    errno = save_errno;
}

void Utils::addSig(int sig, void (*handler)(int), bool restart) {
    struct sigaction sa{};
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void Utils::setFd(int *pFd, int eFd) {
    m_pipeFd = pFd;
    m_epollFd = eFd;
}
