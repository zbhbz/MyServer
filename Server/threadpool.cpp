
#include "threadpool.h"
#include <sstream>
#include <iostream>
#include <exception>

myserver::ThreadPool::ThreadPool(const int threadnum) :
started_(false), threadnum_(threadnum){}

myserver::ThreadPool::~ThreadPool() {
    stop();

    for (int i = 0; i < threads_.size(); ++ i) {
        threads_[i]->join();
    }
    for (int i = 0; i < threads_.size(); ++ i) {
        delete threads_[i];
    }
}

void myserver::ThreadPool::start() {
    started_ = true;
    for (int i = 0; i < threadnum_; ++ i) {
        threads_.push_back(new std::thread(&ThreadPool::threadFunc, this));
    }
}

void myserver::ThreadPool::stop() {
    started_ = false;
    condition_.notify_all();
}

void myserver::ThreadPool::addTask(Task task){
    {
        std::unique_lock<std::mutex> lock(mutex_);
        taskqueue_.push(task);
    }
    condition_.notify_one();
}

void myserver::ThreadPool::threadFunc(){
    std::thread::id tid = std::this_thread::get_id();
    std::stringstream ss;
    ss << tid;
    std::cout << " worker thread tid:" << ss.str() << std::endl;
    Task task;
    while (started_) {
        task = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (taskqueue_.empty() && started_) {
                condition_.wait(lock);
            }
            if (started_ == false) {
                break;
            }
            task = taskqueue_.front();
            taskqueue_.pop();
        }
        if (task) {
            try {
                task();
            } catch (std::exception e) {
                std::cerr << "run task error in worker thread tid:" << ss.str() << " for" << e.what() << std::endl;
            }
        }
    }
}
