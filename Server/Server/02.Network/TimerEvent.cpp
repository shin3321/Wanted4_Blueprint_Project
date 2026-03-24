#include "pch.h"
#include "TimerEvent.h"

std::priority_queue<Timer> timer_queue;
std::mutex timer_mutex;
std::condition_variable timer_cv;
