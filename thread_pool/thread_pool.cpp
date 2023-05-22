#include "thread_pool.h"
using namespace std;

inline ThreadPool::ThreadPool():is_stop(false){
    int size = thread::hardware_concurrency();
    for(size_t i=0;i<size;++i){
        threads.emplace_back([this]{
            while(true){
                function<void()> task;
                {
                    std::unique_lock<mutex> lock(this->mtx);
                    this->cv.wait(lock,[this]{
                        return this->is_stop || !this->tasks.empty();
                    });
                    if(this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

inline ThreadPool::ThreadPool(size_t size):is_stop(false){
    size = size>0?size:1;
    for(size_t i=0;i<size;++i){
        threads.emplace_back([this]{
            while(true){
                function<void()> task;
                {
                    std::unique_lock<mutex> lock(this->mtx);
                    this->cv.wait(lock,[this]{
                        return this->is_stop ||!this->tasks.empty();
                    });
                    if(this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

template<class F,class... Args>
auto ThreadPool::submit(F&&f,Args&&... args)->future<decltype(f(args...))> {
    if(is_stop) throw std::runtime_error("submit has been stopped");
    using ResType = decltype(f(args...));
    auto task_ptr = make_shared<packaged_task<ResType()>>(bind(forward<F>(f),forward<Args>(args)...));
    future<ResType> task_future = task_ptr->get_future();
    {
        unique_lock<mutex> lock(mtx);
        tasks.push([task_ptr](){
            (*task_ptr)();
        });
    }
    cv.notify_one();
    return task_future;
}

inline ThreadPool::~ThreadPool(){
    {
        std::unique_lock<mutex> lock(mtx);
        is_stop = true;
    }
    cv.notify_all();
    for(auto& t:threads){
        t.join();
    }
}