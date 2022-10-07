//
// Created by 李粟康 on 2022/9/19.
//

#ifndef UTILS_THREADPOOL_H
#define UTILS_THREADPOOL_H

#include <thread>
#include <list>
#include <mutex>
#include <vector>
#include "sem.h"

template<typename T>
class ThreadPool {
public:
    ThreadPool(int model, int threadNumber = 8, int maxRequests = 10000);
    ~ThreadPool();
    bool append(T* request, int state);
    bool appendP(T* request);

private:
    static void* worker(void* args);

    [[noreturn]] void run();

    int m_threadNumber;
    int m_maxRequests;
    std::vector<std::thread*> m_threads;
    // 工作队列和锁
    std::list<T*> m_workQueue;
    Semaphore m_queueState;
    std::mutex m_lock;

    int m_actorModel;
};


#endif //UTILS_THREADPOOL_H
