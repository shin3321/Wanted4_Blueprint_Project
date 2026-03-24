#pragma once
//===============================
//		    Player
// 캐릭터 스탯, 현재 속한 방 정보 등 게임 로직을 수행
// 유저 개인의 고유 정보
//===============================

class GameSession;
class Unit;
class Player
{
public:
	Player(std::string nickname);

public:
	bool addUnit();
	void subUnit();
	//	std::map<uint16, std::shared_ptr<Unit>> getUnits() { return _units; }
	void setId(uint16_t id) { _id = id; }
	uint16 getId() { return _id; }
private:
	//std::shared_ptr<GameSession> _session;
	uint16 _id;
	std::string _nickname;
	bool _isFull = false;
	std::atomic<uint16> _unitCount = 0;
};

