#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <vector>
#include <memory>
#include <thread>
#include <condition_variable>
#include <future>
#include <functional>
#include <mutex>
#include "/home/xiaosa/desktop/xiaosa_webserver/thread_safe_queue/thread_safe_queue.h"
#include "/home/xiaosa/desktop/xiaosa_webserver/thread_safe_queue/thread_safe_queue.cpp"
//#include "thread_safe_queue/thread_safe_queue.cpp"

using std::vector,std::mutex,std::thread,std::function,std::condition_variable,std::future;


class ThreadPool{
public:
    ThreadPool();
    ThreadPool(size_t);
    template <class F,class... Args>
    auto submit(F&&f,Args&&... args)->future<decltype(f(args...))>;
    ~ThreadPool();
private:
    mutex mtx;
    vector<thread> threads;
    ThreadSafeQueue<function<void()>> tasks;
    bool is_stop;
    condition_variable cv;
    inline void restart(){
        this->is_stop = false;
        this->cv.notify_all();
    }
    inline void stop(){
        this->is_stop = true;
    }
};


#endif 