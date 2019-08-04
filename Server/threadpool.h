#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

namespace myserver {

class ThreadPool
{
public:
    using Task = std::function<void()>;

    ThreadPool(const int threadnum = 0);
    ~ThreadPool();

    void start();
    void stop();

    int getThreadNum() const {return threadnum_;}

    void addTask(Task task);

    void threadFunc();
private:
    bool started_;

    int threadnum_;
    std::vector<std::thread*> threads_;
    std::mutex mutex_;
    std::condition_variable condition_;

    std::queue<Task> taskqueue_;
};
}
#endif // THREADPOOL_H
