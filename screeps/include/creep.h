//
// Created by 李粟康 on 2022/9/11.
//

#ifndef UTILS_CREEP_H
#define UTILS_CREEP_H

#include <string>

using std::string;

class Creep{
public:
    Creep();
    Creep(string name);

private:
    string name;
    int ttl;
};

#endif //UTILS_CREEP_H
