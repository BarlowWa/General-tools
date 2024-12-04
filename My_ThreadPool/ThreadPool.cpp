#include "ThreadPool.h"
#include <type_traits>

template<class Func, class... Args>
std::future<typename std::invoke_result_t<Func , Args...>::type>
ThreadPool::push( Func&& func, Args&&... args){

    std::packaged_task task(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::future<typename std::invoke_result_t<Func , Args...>::type> ret=task.get_future();

    {
        std::lock_guard<std::mutex> lgd(this->m_mtx);
        this->m_threads.emplace_back(std::move(task));
    }
    {
        std::shared_lock<std::shared_mutex> sl(this->m_rw_mtx);
        if(!this->m_stopFlag){
            this->m_cv.notify_one();
        }
    }

    return ret;
}

void ThreadPool::setStopFlag(const bool status=true){
    std::unique_lock<std::shared_mutex> ul(this->m_rw_mtx);
    this->m_stopFlag=status;
}

ThreadPool::ThreadPool(const int nThreads=2):m_stopFlag(false){
    if(nThreads<=0){
        throw std::range_error("nThreads out of range");
    }
    int n=std::thread::hardware_concurrency();
    if(n==0 || n>nThreads){
        n=nThreads;
    }

    m_threads.resize(n);
}

