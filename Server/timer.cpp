
#include "timer.h"
#include "timermanage.h"

myserver::Timer::Timer(int timeout, TimerType timertype, const Callback& callback) :
    timeout_(timeout),
    timertype_(timertype),
    callback_(callback),
    prev_(nullptr),
    next_(nullptr),
    rotation_(0),
    endsolt_(0) {}

myserver::Timer::~Timer() {
    removeFromTimerManage();
}

void myserver::Timer::addToTimerManage() {
    TimerManage::getInstance()->addTimer(this);
}

void myserver::Timer::removeFromTimerManage() {
    TimerManage::getInstance()->removeTimer(this);
}

void myserver::Timer::adjustInTimerManage(int timeout, TimerType timertype, const Callback& callback) {
    timeout_ = timeout;
    timertype_ = timertype;
    callback_ = callback;
    TimerManage::getInstance()->adjustTimer(this);
}

