//
// Created by 李粟康 on 2022/9/19.
//

#include "ThreadPool.h"

template<typename T>
ThreadPool<T>::ThreadPool(int model, int threadNumber, int maxRequests) :
        m_actorModel(model), m_threadNumber(threadNumber), m_maxRequests(maxRequests) {
    // 生成线程组
    if(m_threadNumber <= 0 || m_maxRequests <= 0) {
        throw std::exception();
    }
    m_threads = std::vector<std::thread*>(m_threadNumber, new std::thread());
    if(m_threads.size() != m_threadNumber) {
        throw std::exception();
    }

    // 线程组detach逃离
    bool valid = true;
    for(std::thread* &thread :m_threads) {
        if(thread == nullptr) {
            valid = true;
            continue;
        }
        thread->detach();
    }
    // 线程创建异常
    if(!valid) {
        for(std::thread* &thread :m_threads) {
            if(thread == nullptr) {
                continue;
            }
            delete thread;
            thread = nullptr;
        }
        throw std::exception();
    }

}

template<typename T>
ThreadPool<T>::~ThreadPool() {
    for(std::thread* &thread :m_threads) {
        if(thread == nullptr) {
            continue;
        }
        delete thread;
        thread = nullptr;
    }
}

template<typename T>
bool ThreadPool<T>::append(T* request, int state) {
    auto* guard = new std::lock_guard<std::mutex>(m_lock);
    if (m_workQueue.size() >= m_maxRequests)
    {
        delete guard;
        guard = nullptr;
        return false;
    }
    request->m_state = state;
    m_workQueue.push_back(request);
    delete guard;
    guard = nullptr;
    m_queueState.signal();
    return true;
}

template<typename T>
bool ThreadPool<T>::appendP(T *request) {
    auto* guard = new std::lock_guard<std::mutex>(m_lock);
    if (m_workQueue.size() >= m_maxRequests) {
        delete guard;
        guard = nullptr;
        return false;
    }
    m_workQueue.push_back(request);
    delete guard;
    guard = nullptr;
    m_queueState.signal();
    return true;
}

template <typename T>
void* ThreadPool<T>::worker(void *arg)
{
    ThreadPool* pool = (ThreadPool*)arg;
    pool->run();
    return pool;
}

template <typename T>
[[noreturn]] void ThreadPool<T>::run()
{
    while (true) {
        m_queueState.wait();
        auto* guard = new std::lock_guard<std::mutex>(m_lock);
        if (m_workQueue.empty()) {
            delete guard;
            guard = nullptr;
            continue;
        }
        T* request = m_workQueue.front();
        m_workQueue.pop_front();
        delete guard;
        guard = nullptr;
        if (!request) {
            continue;
        }

        if (1 == m_actorModel) {
            if (0 == request->m_state) {
                request->improv = 1;
                if (request->read_once()) {
                    //connectionRAII mysqlcon(&request->mysql, m_connPool);
                    request->process();
                } else {
                    request->timer_flag = 1;
                }
            } else {
                request->improv = 1;
                if(!request->write()) {
                    request->timer_flag = 1;
                }
            }
        } else {
            // connectionRAII mysqlcon(&request->mysql, m_connPool);
            request->process();
        }
    }
}