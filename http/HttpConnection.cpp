//
// Created by 李粟康 on 2022/9/20.
//

#include "HttpConnection.h"
#include "utils.h"
#include <fstream>
#include <sys/mman.h>
#include <sys/uio.h>
#include <stdarg.h>


namespace HTTP {
    const char *ok_200_title = "OK";
    const char *error_400_title = "Bad Request";
    const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
    const char *error_403_title = "Forbidden";
    const char *error_403_form = "You do not have permission to get file form this server.\n";
    const char *error_404_title = "Not Found";
    const char *error_404_form = "The requested file was not found on this server.\n";
    const char *error_500_title = "Internal Error";
    const char *error_500_form = "There was an unusual problem serving the request file.\n";
}


using namespace HTTP;

int HttpConnection::m_epollFd = -1;
int HttpConnection::m_userCount = 0;



// 关闭连接，关闭一个连接，客户总量减一
void HttpConnection::close(bool realClose) {
    if (realClose && (m_sockFd != -1)) {
        printf("close %d\n", m_sockFd);
        Utils::getInstance()->removeFd(m_epollFd, m_sockFd);
        m_sockFd = -1;
        m_userCount--;
    }
}

// 初始化连接,外部调用初始化套接字地址
void HttpConnection::init(int sockFd, const sockaddr_in &addr, char *root, int TRIGMode,
                     int closeLog, string user, string password, string sqlName) {
    m_sockFd = sockFd;
    m_address = addr;

    Utils::getInstance()->addFd(m_epollFd, sockFd, true, m_TRIGMode);
    m_userCount++;

    //当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
    m_docRoot = root;
    m_TRIGMode = TRIGMode;
    m_closeLog = closeLog;

    strcpy(m_sqlUser, user.c_str());
    strcpy(m_sqlPassword, password.c_str());
    strcpy(m_sqlName, sqlName.c_str());

    init();
}

// 初始化新接受的连接
// check_state默认为分析请求行状态
void HttpConnection::init() {
    // mysql = NULL;
    m_bytesToSend = 0;
    m_bytesHaveSend = 0;
    m_checkState = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = 0;
    m_version = 0;
    m_contentLength = 0;
    m_host = 0;
    m_startLine = 0;
    m_checkedIdx = 0;
    m_readIdx = 0;
    m_writeIdx = 0;
    m_cgi = 0;
    m_state = 0;
    m_timerFlag = 0;
    m_improv = 0;

    memset(m_readBuf, '\0', READ_BUFFER_SIZE);
    memset(m_writeBuf, '\0', WRITE_BUFFER_SIZE);
    memset(m_realFile, '\0', FILENAME_LEN);
}

// 从状态机，用于分析出一行内容
// 返回值为行的读取状态，有LINE_OK,LINE_BAD,LINE_OPEN
LINE_STATUS HttpConnection::parseLine() {
    char temp;
    for (; m_checkedIdx < m_readIdx; ++m_checkedIdx) {
        temp = m_readBuf[m_checkedIdx];
        if (temp == '\r') {
            if ((m_checkedIdx + 1) == m_readIdx) {
                return LINE_OPEN;
            } else if (m_readBuf[m_checkedIdx + 1] == '\n') {
                m_readBuf[m_checkedIdx++] = '\0';
                m_readBuf[m_checkedIdx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        } else if (temp == '\n') {
            if (m_checkedIdx > 1 && m_readBuf[m_checkedIdx - 1] == '\r') {
                m_readBuf[m_checkedIdx - 1] = '\0';
                m_readBuf[m_checkedIdx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

// 循环读取客户数据，直到无数据可读或对方关闭连接
// 非阻塞ET工作模式下，需要一次性将数据读完
bool HttpConnection::readOnce() {
    if (m_readIdx >= READ_BUFFER_SIZE) {
        return false;
    }
    int bytesRead = 0;

    // LT读取数据
    if (0 == m_TRIGMode) {
        bytesRead = recv(m_sockFd, m_readBuf + m_readIdx, READ_BUFFER_SIZE - m_readIdx, 0);
        m_readIdx += bytesRead;
        return bytesRead > 0;
    }
    // ET读数据
    else {
        while (true) {
            bytesRead = recv(m_sockFd, m_readBuf + m_readIdx, READ_BUFFER_SIZE - m_readIdx, 0);
            if (bytesRead == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)    break;
                return false;
            } else if (bytesRead == 0) {
                return false;
            }
            m_readIdx += bytesRead;
        }
        return true;
    }
}

// 解析http请求行，获得请求方法，目标url及http版本号
HTTP_CODE HttpConnection::parseRequestLine(char *text) {
    m_url = strpbrk(text, " \t");
    if (!m_url) {
        return BAD_REQUEST;
    }
    *m_url++ = '\0';
    char* method = text;
    // 请求方法
    if (strcasecmp(method, "GET") == 0) {
        m_method = GET;
    } else if (strcasecmp(method, "POST") == 0) {
        m_method = POST;
        m_cgi = 1;
    } else {
        return BAD_REQUEST;
    }

    m_url += strspn(m_url, " \t");
    m_version = strpbrk(m_url, " \t");
    if (!m_version) return BAD_REQUEST;

    *m_version++ = '\0';
    m_version += strspn(m_version, " \t");
    if (strcasecmp(m_version, "HTTP/1.1") != 0) return BAD_REQUEST;
    if (strncasecmp(m_url, "http://", 7) == 0) {
        m_url += 7;
        m_url = strchr(m_url, '/');
    }

    if (strncasecmp(m_url, "https://", 8) == 0) {
        m_url += 8;
        m_url = strchr(m_url, '/');
    }

    if (!m_url || m_url[0] != '/') return BAD_REQUEST;

    // 当url为/时，显示判断界面

    if (strlen(m_url) == 1) strcat(m_url, "judge.html");
    m_checkState = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

//解析http请求的一个头部信息
HTTP_CODE HttpConnection::parseHeaders(char *text) {
    if (text[0] == '\0') {
        if (m_contentLength != 0) {
            m_checkState = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    } else if (strncasecmp(text, "Connection:", 11) == 0) {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0) {
            m_linger = true;
        }
    } else if (strncasecmp(text, "Content-length:", 15) == 0) {
        text += 15;
        text += strspn(text, " \t");
        m_contentLength = atol(text);
    } else if (strncasecmp(text, "Host:", 5) == 0) {
        text += 5;
        text += strspn(text, " \t");
        m_host = text;
    } else {
        //LOG_INFO("oop!unknow header: %s", text);
        return NO_REQUEST; // add because blank 'else' is not permitted
    }
    return NO_REQUEST;
}

//判断http请求是否被完整读入
HTTP_CODE HttpConnection::parseContent(char *text) {
    if (m_readIdx >= (m_contentLength + m_checkedIdx)) {
        text[m_contentLength] = '\0';
        //POST请求中最后为输入的用户名和密码
        m_string = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

HTTP_CODE HttpConnection::processRead() {
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text = nullptr;

    while ((m_checkState == CHECK_STATE_CONTENT && line_status == LINE_OK) ||
           ((line_status = parseLine()) == LINE_OK)) {
        text = getLine();
        m_startLine = m_checkedIdx;
        // LOG_INFO("%s", text);
        switch (m_checkState) {
            case CHECK_STATE_REQUESTLINE: {
                ret = parseRequestLine(text);
                if (ret == BAD_REQUEST) return BAD_REQUEST;
                break;
            }
            case CHECK_STATE_HEADER: {
                ret = parseHeaders(text);
                if (ret == BAD_REQUEST) return BAD_REQUEST;
                else if (ret == GET_REQUEST) {
                    return doRequest();
                }
                break;
            }
            case CHECK_STATE_CONTENT: {
                ret = parseContent(text);
                if (ret == GET_REQUEST) return doRequest();
                line_status = LINE_OPEN;
                break;
            }
            default: {
                return INTERNAL_ERROR;
            }
        }
    }
    return NO_REQUEST;
}

// 完成request的处理，与业务逻辑相关
HTTP_CODE HttpConnection::doRequest() {
    return BAD_REQUEST;
}

void HttpConnection::unmap() {
    if (m_fileAddress){
        munmap(m_fileAddress, m_fileStat.st_size);
        m_fileAddress = 0;
    }
}

bool HttpConnection::write()
{
    int temp = 0;

    if (m_bytesToSend == 0) {
        Utils::getInstance()->modFd(m_epollFd, m_sockFd, EPOLLIN, m_TRIGMode);
        init();
        return true;
    }

    while (true) {
        temp = writev(m_sockFd, m_iv, m_ivCount);

        if (temp < 0) {
            if (errno == EAGAIN) {
                Utils::getInstance()->modFd(m_epollFd, m_sockFd, EPOLLOUT, m_TRIGMode);
                return true;
            }
            unmap();
            return false;
        }

        m_bytesHaveSend += temp;
        m_bytesToSend -= temp;
        if (m_bytesHaveSend >= m_iv[0].iov_len) {
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = m_fileAddress + (m_bytesHaveSend - m_readIdx);
            m_iv[1].iov_len = m_bytesToSend;
        } else {
            m_iv[0].iov_base = m_writeBuf + m_bytesHaveSend;
            m_iv[0].iov_len = m_iv[0].iov_len - m_bytesHaveSend;
        }

        if (m_bytesToSend <= 0) {
            unmap();
            Utils::getInstance()->modFd(m_epollFd, m_sockFd, EPOLLIN, m_TRIGMode);

            if (m_linger) {
                init();
                return true;
            } else {
                return false;
            }
        }
    }
    return true;
}

bool HttpConnection::addResponse(const char *format, ...) {
    if (m_readIdx >= WRITE_BUFFER_SIZE) return false;
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_writeBuf + m_readIdx, WRITE_BUFFER_SIZE - 1 - m_readIdx, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - m_readIdx)) {
        va_end(arg_list);
        return false;
    }
    m_readIdx += len;
    va_end(arg_list);

    //LOG_INFO("request:%s", m_writeBuf);

    return true;
}

bool HttpConnection::addStatusLine(int status, const char *title) {
    return addResponse("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool HttpConnection::addHeaders(int length) {
    return addContentLength(length) &&
           addLinger() &&
           addBlankLine();
}

bool HttpConnection::addContentLength(int length) {
    return addResponse("Content-Length:%d\r\n", length);
}

bool HttpConnection::addContentType() {
    return addResponse("Content-Type:%s\r\n", "text/html");
}

bool HttpConnection::addLinger() {
    return addResponse("Connection:%s\r\n", m_linger ? "keep-alive" : "close");
}

bool HttpConnection::addBlankLine() {
    return addResponse("%s", "\r\n");
}

bool HttpConnection::addContent(const char* content) {
    return addResponse("%s", content);
}

bool HttpConnection::processWrite(HTTP_CODE ret) {
    switch (ret) {
        case INTERNAL_ERROR: {
            addStatusLine(500, error_500_title);
            addHeaders(strlen(error_500_form));
            if (!addContent(error_500_form))
                return false;
            break;
        }
        case BAD_REQUEST: {
            addStatusLine(404, error_404_title);
            addHeaders(strlen(error_404_form));
            if (!addContent(error_404_form))   return false;
            break;
        }
        case FORBIDDEN_REQUEST: {
            addStatusLine(403, error_403_title);
            addHeaders(strlen(error_403_form));
            if (!addContent(error_403_form))   return false;
            break;
        }
        case FILE_REQUEST: {
            addStatusLine(200, ok_200_title);
            if (m_fileStat.st_size != 0) {
                addHeaders(m_fileStat.st_size);
                m_iv[0].iov_base = m_writeBuf;
                m_iv[0].iov_len = m_writeIdx;
                m_iv[1].iov_base = m_fileAddress;
                m_iv[1].iov_len = m_fileStat.st_size;
                m_ivCount = 2;
                m_bytesToSend = m_writeIdx + m_fileStat.st_size;
                return true;
            }
            else {
                const char *ok_string = "<html><body></body></html>";
                addHeaders(strlen(ok_string));
                if (!addContent(ok_string))
                    return false;
            }
        }
        default:
            return false;
    }
    m_iv[0].iov_base = m_writeBuf;
    m_iv[0].iov_len = m_writeIdx;
    m_ivCount = 1;
    m_bytesToSend = m_writeIdx;
    return true;
}

void HttpConnection::process()
{
    HTTP_CODE read_ret = processRead();
    if (read_ret == NO_REQUEST)
    {
        Utils::getInstance()->modFd(m_epollFd, m_sockFd, EPOLLIN, m_TRIGMode);
        return;
    }
    bool write_ret = processWrite(read_ret);
    if (!write_ret) {
        close();
    }
    Utils::getInstance()->modFd(m_epollFd, m_sockFd, EPOLLOUT, m_TRIGMode);
}

sockaddr_in* HttpConnection::getAddress() {
    return &m_address;
}

char* HttpConnection::getLine() {
    return m_readBuf + m_startLine;
}
