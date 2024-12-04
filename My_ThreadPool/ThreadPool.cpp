#include "ThreadPool.h"
#include <type_traits>

template<class Func, class... Args>
std::future<typename std::invoke_result_t<Func , Args...>::type>
ThreadPool::push( Func&& func, Args&&... args){
    
    
    std::packaged_task<std::invoke_result_t<Func,Args...>> task(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::unique_ptr<std::function<void()>> task_ptr(new std::function(task));
    std::future<typename std::invoke_result_t<Func , Args...>::type> ret=task.get_future();

    {
        std::lock_guard<std::mutex> lgd(this->m_mtx);
        this->m_tasks.emplace_back(std::move(task_ptr));
    }
    {
        std::shared_lock<std::shared_mutex> sl(this->m_rw_mtx);
        if(!this->m_stopFlag){
            this->m_cv.notify_one();
        task_ptr(new std::function(std::packaged_task(std::bind(std::forward<Func>(func), std::forward<Args>(args)...))));
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

    for(int i=0;i<n;i++){
        m_threads.emplace_back([this]{this->doTask();});
        m_threads.back().detach();
    }
}

void ThreadPool::doTask(){
    while(!this->m_stopFlag){
        std::unique_ptr<std::function<void()>> task_ptr;
        {
            std::unique_lock<std::mutex> ulck(this->m_mtx);
            (this->m_cv).wait(ulck,[this]{return !this->m_tasks.empty();});

            task_ptr=std::move(this->m_tasks.front());
            this->m_tasks.pop_front();
        }
        
        (*task_ptr)();
    }
}
