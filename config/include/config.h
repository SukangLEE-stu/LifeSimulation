//
// Created by 李粟康 on 2022/9/21.
//

#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H

class Config
{
public:
    Config();
    ~Config();

    void parse_arg(int argc, char* argv[]);

    //端口号
    int m_PORT;

    //日志写入方式
    int m_LOGWrite;

    //触发组合模式
    int m_TRIGMode;

    //listenfd触发模式
    int m_LISTENTrigmode;

    //connfd触发模式
    int m_CONNTrigmode;

    //优雅关闭链接
    int m_OPT_LINGER;

    //数据库连接池数量
    int m_sql_num;

    //线程池内的线程数量
    int m_thread_num;

    //是否关闭日志
    int m_close_log;

    //并发模型选择
    int m_actor_model;
};

#endif //CONFIG_CONFIG_H
