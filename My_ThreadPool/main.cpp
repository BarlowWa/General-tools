#include "ThreadPool.h"
#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <thread>

class Test {
public:
  Test() { std::cout << "Test construction\n"; }
  Test(const Test &) { std::cout << "Test copy construction\n"; }
  Test(const Test &&) { std::cout << "Test move construction\n"; }
};

void test_push(ThreadPool &tp) {
    std::cout<<"------------- test_push -------------\n";
  // 多线程执行测试
  {
    std::vector<std::future<int>> res;
    for (int i = 0; i < 5; i++) {
      res.push_back(tp.push(
          [](int a) {
            std::cout << "[task" << a
                      << "]\tChild thread id:" << std::this_thread::get_id()
                      << std::endl;
            return a;
          },
          i));
    }
    std::cout << "answers:\t";
    for (std::future<int> &fut : res) {
      std::cout << fut.get() << "\t";
    }
  }

  // 参数传递测试
  {
    Test t;
    std::cout << "t_addr:" << &t << std::endl;
    auto addr = tp.push(
        [](Test &t) { std::cout << "parameter t's addr:" << &t << std::endl; },
        t);
    addr.get();
  }

  //变量捕获测试
  {
      Test t;
    std::cout << "t_addr:" << &t << std::endl;
    auto addr = tp.push(
        [&t] { std::cout << "variable t's addr:" << &t << std::endl; }
        );
    addr.get();
    }
  std::cout << "\ntest_push finished.\n";
}

void test_pause(ThreadPool &tp) {
    std::cout<<"------------- test_pause -------------\n";
    std::vector<std::future<int>> res;
    for (int i = 0; i < 5; i++) {
      res.push_back(tp.push(
          [](int a) {
            std::cout << "[task" << a
                      << "]\tChild thread id:" << std::this_thread::get_id()
                      << std::endl;
            return a;
          },
          i));
    }
    
    using namespace std::chrono;
    res[3].get();
    tp.setPauseFlag(true);

    auto t=system_clock::to_time_t(system_clock::now());
    std::cout<<"time:"<<ctime(&t);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    tp.setPauseFlag(false);

    t=system_clock::to_time_t(system_clock::now());
    std::cout<<"time:"<<ctime(&t);

    res.back().get();

    std::cout<<"test_pause finished.\n";
}

int main() {
  ThreadPool tp(2);

  std::cout << "test start:\n";
    try{
  test_push(tp);
  test_pause(tp);
    }
    catch(std::exception &e){
        std::cout<<"catch error:"<<e.what();
    }

  std::cout << "test end\n";
  return 0;
}
