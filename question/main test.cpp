#include<iostream>
#include</home/xiaosa/桌面/xiaosa_webserver/thread_pool/thread_pool.cpp>
#include</home/xiaosa/桌面/xiaosa_webserver/thread_pool/thread_pool.h>

mutex mtx;


class Temp{
public:
    ThreadPool thread_pool;
    void func1(int start,int end){
        for(int i=start;i<end;++i){
            unique_lock<mutex> lock(mtx);
            cout<<"func1 "<<i<<endl;
        }
    }
};


int main(){
    Temp temp;
    return 0;
}