//
// Created by 李粟康 on 2022/9/10.
//

#include <iostream>
#include "utils.h"
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
// #include "../sys/epoll.h"

using std::cout;

void helloWorld() {
    cout<<"hello world!";
}

//对文件描述符设置非阻塞
int setNonBlocking(int fd) {
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addFd(int epollFd, int fd, bool oneShot, int TRIGMode) {
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

//从内核时间表删除描述符
void removeFd(int epollFd, int fd) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

//将事件重置为EPOLLONESHOT
void modFd(int epollFd, int fd, int ev, int TRIGMode) {
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event);
}