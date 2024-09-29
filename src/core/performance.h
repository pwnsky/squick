#pragma once

#include <iostream>
#include <core/base.h>
#include <time.h>
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
