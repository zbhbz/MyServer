#ifndef TIMERMANAGE_H
#define TIMERMANAGE_H

#include <vector>
#include <mutex>
#include <thread>
#include "timer.h"
namespace myserver {

// single instance
class TimerManage {
public:
    static TimerManage* getInstance();
    void addTimer(Timer*);
    void removeTimer(Timer*);
    void adjustTimer(Timer*);
    void start();
    void stop();

    class GC{
      public:
        ~GC() {
            if (ptimermanage_ != nullptr) {
                delete ptimermanage_;
            }
        }
    };

private:
    static TimerManage* ptimermanage_;
    static std::mutex mutex_;
    static GC gc_;

    const static int intervaleveryslot_;
    const static int slotnum_;
    int currslot_;

    std::vector<Timer*> timerwheel_;
    std::mutex timerwheelmutex_;

    std::thread thread_; // forever caculate time

    bool running_;

    TimerManage();
    ~TimerManage();

    void checkTick(); // run in thread, forever caculate time
    void checkTimerListForCurrentSlot(); // check if any timer is timeout

    void caculateTimer(Timer*); // used to caculate endslot and rotation for Timer

    void addTimerToTimerWheel(Timer*);
    void removeTimerFromTimerWheel(Timer*);
    void adjustTimerToTimerWheel(Timer*);
};
}
#endif //TIMERMANAGE_H
