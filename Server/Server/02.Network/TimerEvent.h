#pragma once
#include <queue>
#include <chrono>

using namespace std::chrono_literals;

enum class TimerEvent
{
	EV_GAME_START = 1,
	EV_UNIT_ATTACK, 
};

struct Timer
{
	uint16_t playerId;
	std::chrono::system_clock::time_point wakeUpTime;
	TimerEvent _eventType;

	uint16_t unitId;
	constexpr bool operator < (const Timer& L)const
	{
		return (wakeUpTime > L.wakeUpTime);
	}
};

extern std::priority_queue<Timer> timer_queue;
extern std::mutex timer_mutex;
extern std::condition_variable timer_cv;


