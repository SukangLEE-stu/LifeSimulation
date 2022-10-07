//
// Created by 李粟康 on 2022/9/10.
//

#ifndef UTILS_UTILS_H
#define UTILS_UTILS_H

class Utils {
public:
    void helloWorld();

    void init();

    static Utils* getInstance();

    void setFd(int* pFd, int eFd);

    // 对文件描述符设置非阻塞
    int setNonBlocking(int fd);

    // 将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addFd(int epollFd, int fd, bool oneShot, int TRIGMode);

    // 从内核时间表删除描述符
    void removeFd(int epollFd, int fd);

    // 将事件重置为EPOLLONESHOT
    void modFd(int epollFd, int fd, int ev, int TRIGMode);

    // 信号处理函数
    static void sigHandler(int sig);

    void addSig(int sig, void(handler)(int), bool restart = true);


private:
    Utils();
    ~Utils();
    int m_timeSlot;
    static int* m_pipeFd;
    static int m_epollFd;
};






#endif //UTILS_UTILS_H