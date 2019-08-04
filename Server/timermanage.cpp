
#include "timermanage.h"
#include <sys/time.h>
#include <chrono>

namespace myserver {

TimerManage* TimerManage::ptimermanage_ = nullptr;
std::mutex TimerManage::mutex_;
TimerManage::GC TimerManage::gc_;

const int TimerManage::intervaleveryslot_ = 1;
const int TimerManage::slotnum_ = 1024;

TimerManage::TimerManage() :
    currslot_(0),
    timerwheel_(slotnum_, nullptr),
    running_(false) {}

TimerManage::~TimerManage() {
    stop();
}

TimerManage* TimerManage::getInstance() {
    if (ptimermanage_ == nullptr) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (ptimermanage_ == nullptr) {
            ptimermanage_ = new TimerManage();
        }
    }
    return ptimermanage_;
}

void TimerManage::addTimer(Timer* ptimer) {
    if (ptimer == nullptr) return;
    std::unique_lock<std::mutex> lock(timerwheelmutex_);
    caculateTimer(ptimer);
    addTimerToTimerWheel(ptimer);
}

void TimerManage::removeTimer(Timer* ptimer) {
    if (ptimer == nullptr) return;
    std::unique_lock<std::mutex> lock(timerwheelmutex_);
    removeTimerFromTimerWheel(ptimer);
}

void TimerManage::adjustTimer(Timer* ptimer) {
    if (ptimer == nullptr) return;
    std::unique_lock<std::mutex> lock(timerwheelmutex_);
    adjustTimerToTimerWheel(ptimer);
}

void TimerManage::start() {
    running_ = true;
    thread_ = std::thread(&TimerManage::checkTick, this);
}

void TimerManage::stop() {
    running_ = false;
    if (thread_.joinable())
        thread_.join();
}

void TimerManage::checkTick() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int oldtime = (tv.tv_sec % 10000) * 1000 + tv.tv_usec / 1000;
    int newtime;
    int slotcount;
    while (running_) {
        gettimeofday(&tv, nullptr);
        newtime = (tv.tv_sec % 10000) * 1000 + tv.tv_usec / 1000;
        slotcount = (newtime - oldtime) / intervaleveryslot_;
        oldtime = oldtime + slotcount*intervaleveryslot_;
        for (int i = 0; i < slotcount; ++ i) {
            checkTimerListForCurrentSlot();
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
}

void TimerManage::checkTimerListForCurrentSlot() {
    std::unique_lock<std::mutex> lock(timerwheelmutex_);
    Timer* ptimer = timerwheel_[currslot_];
    while (ptimer != nullptr) {
        if (ptimer->rotation_ > 0) {
            --ptimer->rotation_;
            ptimer = ptimer->next_;
        } else {
            ptimer->callback_();
            if (ptimer->timertype_ == Timer::TimerType::TIMER_ONCE) {
                Timer* oldptimer = ptimer;
                ptimer = ptimer->next_;
                removeTimerFromTimerWheel(oldptimer);
            } else {
                Timer* oldptimer = ptimer;
                ptimer = ptimer->next_;
                adjustTimerToTimerWheel(oldptimer);
                if (currslot_ == oldptimer->endsolt_ && oldptimer->rotation_ > 0) {
                    --oldptimer->rotation_;
                }
            }
        }
    }
    currslot_ = (currslot_ + 1) % slotnum_;
}

void TimerManage::caculateTimer(Timer* ptimer) {
    if (ptimer == nullptr) return;
    int timeout = ptimer->timeout_;
    int tick = 0; // need to run tick slots
    if (timeout < intervaleveryslot_) {
        tick = 1;
    } else {
        tick = timeout / intervaleveryslot_;
    }
    ptimer->rotation_ = tick / slotnum_;
    ptimer->endsolt_ = (currslot_ + tick) % slotnum_;
}

void TimerManage::addTimerToTimerWheel(Timer* ptimer) {
    if (ptimer == nullptr) return;
    int endslot = ptimer->endsolt_;
    if (timerwheel_[endslot]) {
        ptimer->next_ = timerwheel_[endslot];
        timerwheel_[endslot]->prev_ = ptimer;
        timerwheel_[endslot] = ptimer;
    } else {
        timerwheel_[endslot] = ptimer;
    }
}

void TimerManage::removeTimerFromTimerWheel(Timer* ptimer) {
    if (ptimer == nullptr) return;
    int endslot = ptimer->endsolt_;
    if (timerwheel_[endslot] == ptimer) {
        timerwheel_[endslot] = ptimer->next_;
        if (ptimer->next_ != nullptr) {
            ptimer->next_->prev_ = nullptr;
        }
        ptimer->prev_ = ptimer->next_ = nullptr;
    } else {
        if (ptimer->prev_ == nullptr) { // is not exist
            return;
        }
        ptimer->prev_->next_ = ptimer->next_;
        if (ptimer->next_ != nullptr) {
            ptimer->next_->prev_ = ptimer->prev_;
        }
        ptimer->prev_ = ptimer->next_ = nullptr;
    }
}

void TimerManage::adjustTimerToTimerWheel(Timer* ptimer) {
    if (ptimer == nullptr) return;
    removeTimerFromTimerWheel(ptimer);
    caculateTimer(ptimer);
    addTimerToTimerWheel(ptimer);
}

}
