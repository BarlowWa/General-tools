#include <vector>
#include <functional>
#include <future>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

class ThreadPool{
public:
    
    /*
    push 新的任务
    @func 函数名
    @args 函数参数
    @ret 返回对应future对象
    */
    template<class Func,class... Args>
    std::future<typename std::invoke_result_t<Func, Args...>::type> push(Func&& func, Args&&... args);
    
    //设置线程任务暂停/运行
    void setStopFlag(const bool status);
    
    /*
     * 构造线程池
     * @n 线程数量
    */
    explicit ThreadPool(const int nThreads);

    ~ThreadPool();

private:
    void doTask();

    ThreadPool(const ThreadPool&)=delete;
    ThreadPool& operator=(const ThreadPool&)=delete;

    std::vector<std::thread> m_threads;
    std::deque<std::unique_ptr<std::function<void()>>> m_tasks;
    
    //控制m_tasks访问
    std::mutex m_mtx;
    //控制m_StopFlag访问
    std::shared_mutex m_rw_mtx;
    std::condition_variable m_cv;

    bool m_stopFlag;
};
