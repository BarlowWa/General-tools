# General-tools

## My\_ThreadPool
- 线程池通过`setPauseFlag()`方法支持暂停/恢复，而不会析构线程池
- 由于使用了`std::bind`和`std::packaged_task`方法，`push`方法传入的任务参数必须支持copy construct和move construct,对于复杂参数建议使用对象指针传递参数

