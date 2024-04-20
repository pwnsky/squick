#include "base.h"
#include <atomic>

std::atomic<bool> is_squick_main_thread_sleep = true;

void SetSquickMainThreadSleep(bool is_sleep) { is_squick_main_thread_sleep = is_sleep; }

bool IsSquickMainThreadSleep() { return is_squick_main_thread_sleep; }