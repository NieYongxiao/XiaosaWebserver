#pragma once
#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H



#include <queue>
#include <mutex>
#include <condition_variable>
#include <shared_mutex>

using std::queue,std::mutex;

template< class T>
class ThreadSafeQueue{
public:
    ThreadSafeQueue()=default;
    ThreadSafeQueue(ThreadSafeQueue&&);
    bool empty();
    int size();
    void push(const T& value);
    void push(T&& value);
    T pop();
    T front();
    T back();
private:
    mutex mtx;
    queue<T> safe_queue;
};


#endif