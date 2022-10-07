//
// Created by 李粟康 on 2022/9/10.
//

#ifndef LIFESIMULATION_WEB_H
#define LIFESIMULATION_WEB_H

#include <sys/epoll.h>

#include "HttpConnection.h"
#include "ThreadPool.h"

constexpr int MAX_FD = 65536;           //最大文件描述符
constexpr int MAX_EVENT_NUMBER = 10000; //最大事件数
constexpr int TIMESLOT = 5;             //最小超时单位


class BaseWebServer {
public:
    BaseWebServer();
    virtual ~BaseWebServer();

    void init(int port, int threadNum);

    void eventLoop();

    void eventListen();


private:
    bool doConnect();
    void m_doAdd(int fd, sockaddr_in addr);



    int m_port;
    char* m_root;
    int m_logWrite;
    int m_closeLog;
    int m_actorModel;

    int m_pipeFd[2];
    int m_epollFd;
    HttpConnection* m_users;

    /*
    //数据库相关
    connection_pool *m_connPool;
    string m_user;         //登陆数据库用户名
    string m_passWord;     //登陆数据库密码
    string m_databaseName; //使用数据库名
    int m_sql_num;
    */

    //线程池相关
    ThreadPool<HttpConnection>* m_threadPool;
    int m_threadNum;

    //epoll_event相关
    epoll_event m_events[MAX_EVENT_NUMBER];

    int m_listenFd;
    int m_OPT_LINGER;
    int m_trigMode;
    int m_listenTrigMode;
    int m_connectionTrigMode;

    //定时器相关
    //client_data *users_timer;
    //Utils utils;
};


#endif //LIFESIMULATION_WEB_H
