//
// Created by 李粟康 on 2022/9/20.
//

#ifndef HTTP_HTTPCONNECTION_H
#define HTTP_HTTPCONNECTION_H

#include <string>
#include <netinet/in.h>
#include <map>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <cstring>


using std::map;
using std::string;
using std::strlen;


constexpr int FILENAME_LEN = 200;
constexpr int READ_BUFFER_SIZE = 2048;
constexpr int WRITE_BUFFER_SIZE = 1024;

namespace HTTP {
    // HTTP方法
    enum METHOD {
        GET = 0,
        POST,
        PUT,
        DELETE,
        HEAD,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };

    // 主状态机
    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };

    // 返回值
    enum HTTP_CODE {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    // 从状态机
    enum LINE_STATUS {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

    //定义http响应的一些状态信息
    extern const char *ok_200_title;
    extern const char *error_400_title;
    extern const char *error_400_form;
    extern const char *error_403_title;
    extern const char *error_403_form;
    extern const char *error_404_title;
    extern const char *error_404_form;
    extern const char *error_500_title;
    extern const char *error_500_form;
}

using HTTP::METHOD;
using HTTP::CHECK_STATE;
using HTTP::HTTP_CODE;
using HTTP::LINE_STATUS;


class HttpConnection {
public:
    HttpConnection();
    ~HttpConnection();

    void init(int sockFd, const sockaddr_in& addr, char*, int, int, string user, string pwd, string sqlName);
    void close(bool realClose = true);
    void process();
    bool readOnce();
    bool write();

    sockaddr_in* getAddress();

    // void initmysql_result(connection_pool *connPool);
    int m_timerFlag;
    int m_improv;


    static int m_epollFd;
    static int m_userCount;
    // MYSQL* mysql;
    // 读写状态，读为0，写为1
    int m_state;

private:
    void init();

    HTTP_CODE processRead();
    bool processWrite(HTTP_CODE ret);

    HTTP_CODE parseRequestLine(char* text);
    HTTP_CODE parseHeaders(char* text);
    HTTP_CODE parseContent(char* text);
    HTTP_CODE doRequest();

    char* getLine();

    LINE_STATUS parseLine();

    void unmap();

    bool addResponse(const char* format, ...);
    bool addContent(const char* content);
    bool addStatusLine(int status, const char* title);
    bool addHeaders(int length);
    bool addContentType();
    bool addContentLength(int length);
    bool addLinger();
    bool addBlankLine();

    int m_sockFd;
    sockaddr_in m_address;
    char m_readBuf[READ_BUFFER_SIZE];
    // 读写头
    int m_readIdx;
    int m_checkedIdx;
    int m_startLine;
    char m_writeBuf[WRITE_BUFFER_SIZE];
    int m_writeIdx;

    CHECK_STATE m_checkState;
    METHOD m_method;

    char m_realFile[FILENAME_LEN];
    char* m_url;
    char* m_version;
    char* m_host;
    int m_contentLength;
    bool m_linger;
    char* m_fileAddress;

    struct stat m_fileStat;
    struct iovec m_iv[2];
    int m_ivCount;
    int m_cgi;        //是否启用的POST
    char* m_string; //存储请求头数据
    int m_bytesToSend;
    int m_bytesHaveSend;
    char* m_docRoot;

    map<string, string> m_users;
    int m_TRIGMode;
    int m_closeLog;

    char m_sqlUser[100];
    char m_sqlPassword[100];
    char m_sqlName[100];
};


#endif //HTTP_HTTPCONNECTION_H
