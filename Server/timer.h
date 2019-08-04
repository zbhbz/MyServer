#ifndef TIMER_H
#define TIMER_H
#include <functional>
namespace myserver {
class Timer {

public:
    using Callback = std::function<void()>;
    enum TimerType{TIMER_ONCE = 0, TIMER_PERIOD = 1};

    Timer(int timeout, TimerType timertype, const Callback& callback);
    ~Timer();

    int rotation_;
    int endsolt_;

    Timer* prev_;
    Timer* next_;

    int timeout_;
    TimerType timertype_;
    Callback callback_;

    void addToTimerManage();
    void removeFromTimerManage();
    void adjustInTimerManage(int timeout, TimerType timertype, const Callback& callback);
};
}
#endif // TIMER_H
