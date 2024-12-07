#include <iostream>
#include "ThreadPool.h"
#include <chrono>
#include <thread>
#include <future>

int func(int id, int time){
    std::cout<<"id:"<<id<<'\t';
    std::cout<<"thread_id:"<<std::this_thread::get_id()<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(time));
    return id;
}

void test(ThreadPool& tp, std::vector<std::future<int>>& res0,std::vector<std::future<int>>& res1) {
    for(int i=0;i<5;i++){
        res0.push_back(tp.push(func,0,2));
    }
    for(int i=0;i<5;i++){
        res1.push_back(tp.push(func,1,3));
    }
}
int main(){
    ThreadPool tp(2);
    std::vector<std::future<int>> res0,res1;
    
    test(tp,res0,res1);
    for(std::future<int>& f:res0 ){
        std::cout<<f.get()<<'\t';
    }
    std::cout<<std::endl;
    for(std::future<int>& f:res1 ){
        std::cout<<f.get()<<'\t';
    }
    std::cout<<std::endl;

    return 0;
}

