//
// Created by 李粟康 on 2022/9/19.
//

#include "Semaphore.h"
using std::mutex;

Semaphore::Semaphore(long count) : m_count(count) {}

void Semaphore::signal() {
    std::unique_lock<mutex> unique(m_lock);
    ++m_count;
    if (m_count <= 0) {
        m_cond.notify_one();
    }
}

void Semaphore::wait() {
    std::unique_lock<mutex> unique(m_lock);
    --m_count;
    if (m_count < 0)
        m_cond.wait(unique);
}