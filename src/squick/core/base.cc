#include "base.h"
#include <atomic>

std::atomic<bool> is_squick_main_thread_sleep = true;

std::atomic<int> squick_reload_state = 0;

void SetSquickMainThreadSleep(bool is_sleep) { is_squick_main_thread_sleep = is_sleep; }

bool IsSquickMainThreadSleep() { return is_squick_main_thread_sleep; }


void SetSquickReloadState(int state) { squick_reload_state = state; }

int GetSquickReloadState() { return squick_reload_state; }