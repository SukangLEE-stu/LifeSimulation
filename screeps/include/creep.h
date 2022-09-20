//
// Created by 李粟康 on 2022/9/11.
//

#ifndef SCREEPS_CREEP_H
#define SCREEPS_CREEP_H

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

#endif //SCREEPS_CREEP_H
