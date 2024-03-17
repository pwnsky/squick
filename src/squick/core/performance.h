

#ifndef SQUICK_PERFORMANCE_H
#define SQUICK_PERFORMANCE_H

#include <iostream>
#include <time.h>
#include <squick/core/base.h>
class Performance {
  private:
    int64_t time = 0;
    int64_t checkTime = 0;

  public:
    Performance() { time = SquickGetTimeMS(); }

    bool CheckTimePoint(const int milliSecond = 1) {
        checkTime = SquickGetTimeMS();
        if (checkTime > milliSecond + time) {
            return true;
        }

        return false;
    }

    int64_t TimeScope() { return SquickGetTimeMS() - time; }
};

#endif