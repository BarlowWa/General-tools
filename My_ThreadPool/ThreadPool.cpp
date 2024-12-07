#include "ThreadPool.h"

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
    if(!this->m_pauseFlag.load()){
        this->m_cv.notify_one();
    }

    return ret;
}

void ThreadPool::setPauseFlag(const bool status=true){
    if(this->m_pauseFlag.load()==status)return;

    this->m_pauseFlag.store(status);
    if(!this->m_pauseFlag.load()){
        this->m_cv.notify_all();
    }
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
    }
}

void ThreadPool::doTask(){
    while(!this->m_stopFlag.load()){
        std::unique_ptr<std::function<void()>> task_ptr;
        {
            std::unique_lock<std::mutex> ulck(this->m_mtx);
            this->m_cv.wait(ulck, !m_pauseFlag.load() && !m_tasks.empty());
            
            task_ptr=std::move(m_tasks.front());
            m_tasks.pop_front();
        }

        (*task_ptr)();
    }

    return ;
}

ThreadPool::~ThreadPool(){
    m_stopFlag.store(true);
    for(std::thread& th:m_threads){
        if(th.joinable())th.join();
    }
}
