#pragma once

class Player;
class Killer;
class Survivor;
class Session;

struct Slot
{
	Location _loc;
	bool _isOccupied = false;
};

class Room
{
public:
	Room(uint16 id);

	//킬러 추가 함수
	bool addKiller(std::shared_ptr<Session> killer, int32 id);
	bool addSurvivor(std::shared_ptr<Session> survivor, int32 id);

	Location WaitingPlayerLocation();

	void sendWaitingRoom(std::shared_ptr<Player> player, int32 playerId, bool isKiller);
	void sendReadyPacket(C_ReadyPacket* readyPacket);
	void sendGameStart();

	void roomBroadcast(void* packet, uint16_t size);
	
	bool _isStarted = false;


	void sendMovePacket(const C_MovePacket& movePacket);

private:
	uint16 _roomId;

	Slot slots[4];
	Location killerLocation = Location(90.f, 170.f, 0.f);

	uint8 readyCount = 0;
	uint8 startCount = 2;


	std::map<int32, std::shared_ptr<Survivor>> _survivors;
	std::mutex _survivorLock;
	std::shared_ptr<Killer> _killer;
};