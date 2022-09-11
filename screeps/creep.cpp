//
// Created by 李粟康 on 2022/9/11.
//

#include "creep.h"

constexpr int TTL = 1500;

Creep::Creep() {
    this->ttl = TTL;
}

Creep::Creep(string name) {
    this->name = name;
    this->ttl = TTL;
}