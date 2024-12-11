#include "ThreadPool.h"
#include <thread>
/*
template <class Func, class... Args>
std::future<typename std::invoke_result<Func, Args...>::type>
ThreadPool::push(Func &&func, Args &&...args) {
  using return_type = std::invoke_result_t<Func, Args...>;

  std::packaged_task<return_type()> task(
      std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
  std::unique_ptr<std::function<void()>> task_ptr(new std::function(task));
  std::future<std::invoke_result_t<Func, Args...>> ret = task.get_future();

  {
    std::lock_guard<std::mutex> lgd(this->m_mtx);
    this->m_tasks.emplace_back(std::move(task_ptr));
  }
  if (!this->m_pauseFlag.load()) {
    this->m_cv.notify_one();
  }

  return ret;
}
*/
void ThreadPool::setPauseFlag(const bool status = true) {
  if (this->m_pauseFlag.load() == status)
    return;

  this->m_pauseFlag.store(status);
  if (!this->m_pauseFlag.load()) {
    std::cout << "Child threads run...\n";
    this->m_cv.notify_all();
  } else {
    std::cout << "Child threads paused\n";
  }
}

ThreadPool::ThreadPool(const int nThreads = 2)
    : m_stopFlag(false), m_pauseFlag(false) {
  if (nThreads <= 0) {
    throw std::range_error("nThreads out of range.\n");
  }
  int n = std::thread::hardware_concurrency();
  if (n == 0 || n > nThreads) {
    n = nThreads;
  }

  for (int i = 0; i < n; i++) {
    m_threads.emplace_back([this] { this->doTask(); });
  }
  std::cout << "Child threads' number:" << m_threads.size() << std::endl;
  std::cout << "ThreadPool constructed\n";
}

void ThreadPool::doTask() {
  {
    std::lock_guard<std::mutex> lck(this->m_mtx);
    std::cout << "Child thread start:" << std::this_thread::get_id()
              << std::endl;
  }
  while (1) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> ulck(this->m_mtx);
      this->m_cv.wait(ulck, [&] {
        return m_stopFlag.load() || !m_pauseFlag.load() && !m_tasks.empty();
      });

      if (m_stopFlag.load())
        break;

      task = std::move(m_tasks.front());
      m_tasks.pop_front();
    }

    task();
  }
  {
    std::unique_lock<std::mutex> ulck(this->m_mtx);
    std::cout << "Child thread exit:" << std::this_thread::get_id()
              << std::endl;
  }

  return;
}

ThreadPool::~ThreadPool() {
  m_stopFlag.store(true);
  m_cv.notify_all();

  for (std::thread &th : m_threads) {
    if (th.joinable())
      th.join();
  }
}
