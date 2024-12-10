#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <vector>
#include <iostream>

class ThreadPool {
public:
  /*
  push 新的任务
  @func 函数名
  @args 函数参数
  @ret 返回对应future对象
  */
  template <class Func, class... Args>
  std::future<std::invoke_result_t<Func, Args...>>
  push(Func &&func, Args &&...args);

  // 设置线程任务暂停/运行
  void setPauseFlag(const bool status);

  /*
   * 构造线程池
   * @n 线程数量
   */
  explicit ThreadPool(const int nThreads);

  ~ThreadPool();

private:
  void doTask();

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  std::vector<std::thread> m_threads;
  std::deque<std::function<void()>> m_tasks;

  // 控制m_tasks访问
  std::mutex m_mtx;

  // 线程运行通知
  std::condition_variable m_cv;

  std::atomic<bool> m_stopFlag;
  std::atomic<bool> m_pauseFlag;
};


template <class Func, class... Args>
std::future<std::invoke_result_t<Func, Args...>>
ThreadPool::push(Func &&func, Args &&...args) {
  using return_type = std::invoke_result_t<Func, Args...>;
  std::cout<<"push task--";

  auto task_ptr = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
      );
  std::future<return_type> ret = task_ptr->get_future();
  {
    std::lock_guard<std::mutex> lgd(m_mtx);
    m_tasks.emplace_back([task_ptr]{ (*task_ptr)();});
  }

  if (!m_pauseFlag.load()) {
    m_cv.notify_one();
  }
    std::cout<<"tasks count:"<<m_tasks.size()<<std::endl;
  return ret;
}
