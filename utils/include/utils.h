//
// Created by 李粟康 on 2022/9/10.
//

#ifndef UTILS_UTILS_H
#define UTILS_UTILS_H

void helloWorld();

//对文件描述符设置非阻塞
int setNonBlocking(int fd);

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addFd(int epollFd, int fd, bool oneShot, int TRIGMode);



#endif //UTILS_UTILS_H
