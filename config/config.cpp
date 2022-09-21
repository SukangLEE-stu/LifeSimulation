//
// Created by 李粟康 on 2022/9/21.
//

#include "config.h"

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

void Config::parse_arg(int argc, char* argv[]) {

}