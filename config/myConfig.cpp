//
// Created by 李粟康 on 2022/9/21.
//

#include "myConfig.h"
#include <string>
using std::string;

Config::Config() :
    m_OPT_LINGER(0),
    m_CONNTrigmode(0),
    m_LISTENTrigmode(0),
    m_LOGWrite(0),
    m_PORT(9002),
    m_TRIGMode(0),
    m_actor_model(0),
    m_close_log(0),
    m_sql_num(8),
    m_thread_num(8) {}

Config::~Config() {}

void Config::parse(int argc, char* argv[]) {
    for(int i = 1; i < argc; i += 2) {
        string s(argv[i]);
        if(s == "--port") {
            m_PORT = atoi(argv[i + 1]);
        } else if( s == "--threads") {
            m_thread_num = atoi(argv[i + 1]);
        }
    }
}