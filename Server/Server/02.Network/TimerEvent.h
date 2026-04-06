#pragma once
#include <queue>
#include <chrono>

using namespace std::chrono_literals;

enum class TimerEvent
{
	EV_GAME_WAIT = 0,
	EV_GAME_START = 1,
};

struct Timer
{
	int32 playerId;
	std::chrono::system_clock::time_point wakeUpTime;
	TimerEvent _eventType;

	uint16_t targetId;
	bool isKiller;
	constexpr bool operator < (const Timer& L)const
	{
		return (wakeUpTime > L.wakeUpTime);
	}
};

extern std::priority_queue<Timer> timer_queue;
extern std::mutex timer_mutex;
extern std::condition_variable timer_cv;


