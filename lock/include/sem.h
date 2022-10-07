//
// Created by 李粟康 on 2022/9/19.
//

#ifndef LOCK_SEMAPHORE_H
#define LOCK_SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class Semaphore
{
public:
    Semaphore(long count = 0);

    //V操作，唤醒
    void signal();

    //P操作，阻塞
    void wait();

private:
    std::mutex m_lock;
    std::condition_variable m_cond;
    long m_count;
};

#endif //LOCK_SEMAPHORE_H
