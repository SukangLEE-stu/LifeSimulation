//
// Created by 李粟康 on 2022/9/10.
//

#include "web.h"
#include "utils.h"

#include <iostream>
#include <mutex>
#include <thread>

int g_num = 0;
std::mutex* g_mutex = new std::mutex();


int main() {
    helloWorld();
    std::cout<<"hello!";
    auto func = [](int id)->void {
        for(int i = 0; i < 3; ++i) {
            LockGuard* guard = new LockGuard(g_mutex);
            g_num += 1;
            std::cout<<"from id: "<<id<<", current num is: "<<g_num<<std::endl;
            delete guard;
            guard = nullptr;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    };
    std::thread t1(func, 0);
    std::thread t2(func, 1);
    t1.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(450));
    t2.join();


    return 0;
}
