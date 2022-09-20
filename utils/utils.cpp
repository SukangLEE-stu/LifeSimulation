//
// Created by 李粟康 on 2022/9/10.
//

#include <iostream>
#include "utils.h"
#include <fcntl.h>
#include <sys/epoll.h>
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