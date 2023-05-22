#include"thread_safe_queue.h"

template< class T>
ThreadSafeQueue<T>::ThreadSafeQueue(ThreadSafeQueue&& other){ 
    mtx = move(other.mtx);
    safe_queue = move(other.safe_queue);
}

template< class T>
bool ThreadSafeQueue<T>::empty(){
    std::unique_lock<mutex> lock(mtx);
    return safe_queue.empty();
}
// template< class T>
// bool ThreadSafeQueue<T>::empty(){
//     std::shared_lock<mutex> lock(mtx);
//     return safe_queue.empty();
// }

template< class T>
int ThreadSafeQueue<T>::size(){
    std::unique_lock<mutex> lock(mtx);
    return safe_queue.size();
}
// template< class T>
// int ThreadSafeQueue<T>::size(){
//     std::shared_lock<mutex> lock(mtx);
//     return safe_queue.size();
// }

template< class T>
void ThreadSafeQueue<T>::push(const T& item){
    std::unique_lock<mutex> lock(mtx);
    safe_queue.push(item);
}

template< class T>
void ThreadSafeQueue<T>::push(T&& item){
    std::unique_lock<mutex> lock(mtx);
    safe_queue.push(item);
}

template< class T>
T ThreadSafeQueue<T>::pop(){
    if(safe_queue.empty()){
        throw std::overflow_error("pop queue size is empty");
    }
    std::unique_lock<mutex> lock(mtx);
    T obj=safe_queue.front();
    safe_queue.pop();
    return obj;
}

template< class T>
T ThreadSafeQueue<T>::front(){
    if(safe_queue.empty()){
        throw std::overflow_error("front queue size is empty");
    }
    std::unique_lock<mutex> lock(mtx);
    return safe_queue.front();
}

// template< class T>
// T ThreadSafeQueue<T>::front(){
//     if(safe_queue.empty()){
//         throw std::overflow_error("size if empty");
//     }
//     std::shared_lock<mutex> lock(mtx);
//     return safe_queue.front();
// }

template< class T>
T ThreadSafeQueue<T>::back(){
    if(safe_queue.empty()){
        throw std::overflow_error("size if empty");
    }
    std::unique_lock<mutex> lock(mtx);
    return safe_queue.back();
}
// template< class T>
// T ThreadSafeQueue<T>::back(){
//     if(safe_queue.empty()){
//         throw std::overflow_error("size if empty");
//     }
//     std::shared_lock<mutex> lock(mtx);
//     return safe_queue.back();
// }