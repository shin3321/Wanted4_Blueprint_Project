#pragma once
#include "99.Header/Protocol.h"
//===============================
//		    Player
// 현재 속한 방 정보 등 게임 로직을 수행
// 
//===============================

class GameSession;
class Session;

class Player
{
public:
	Player() = default;
	Player(int32 id);
public:
	void setId(int32 id) { _id = id; }
	int32 getId() { return _id; }

	void setRoomId(int32 roomId) { _roomId = roomId;}
	void setLocation(Location location) { _location = location; }
	Location getLocation() { return _location; }

	void setSession(std::shared_ptr<Session> session)
	{
		_session = session;
	}


	void send(void* packet, uint16_t packetSize);

private:
	int32 _id;
	uint16 _roomId;

	std::string _nickname;

	Location _location;
	uint8 _slotIndex;

	std::weak_ptr<Session> _session;
};