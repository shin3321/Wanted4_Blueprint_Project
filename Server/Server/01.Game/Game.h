#pragma once
#include "01.Game/QuadTree/QuadTree.h"
#include <set>
class Player;
class Session;
class Castle;
//class Unit;

// 공격 이벤트를 모아서 한 번에 처리
struct AttackEvent {
	Unit* attacker;
	Unit* target;
};

class Game
{
public:
	~Game();
	Game();
	//Game 초기화

	void init(HANDLE iocpHandle, OverlappedExPool& overlappedPool);
	//void runWorkerThread(SOCKET socket);

	//accept 함수 - 플레이어와 연결 후 sessions에 넣음 
	void accept(SOCKET socket);

	//서버 recv
	void recv(uint16_t key, uint16_t numbytes);

	//클라이언트 아이디 생성
	uint16 getClientId();
	uint16 getUnittId();

	//closeSocket 함수 - 플레이어의 소켓을 닫는 함수
	void closeSocket(int sessioneId);
	void broadcast(const char* data, uint16 packetSize);

	//맵 전송 함수
	void waitingRoom(uint16 id);
	void loadMap();
	void sendCastle();
	void setUnitPosTiles(Vector2 NewPos, Vector2 OldPos, uint16 unitId);

	//유닛을 생성할 수 있는 곳인지 판단하는 함수
	bool isSpawnableUnit(uint16 playerId, Vector2 spawnPos);
	bool isMovableUnit(uint16 playerId, Vector2 movePos);

	//유닛을 생성하는 함수
	void spawnUnit(uint16 playerId, Vector2 spawnPos);
	void addUnit(std::shared_ptr<Unit> unit);
	void removeUnit(uint16 unitId, uint16 playerId);
	void moveUnit(uint16 playerId, uint16 unitId, Vector2 movePos);
	std::shared_ptr<Unit> findUnit(uint16 unitId);

	//트리 리빌드 함수
	void rebuild();
	void attackUnit();
	void attackedUnit(std::set<uint16> attackedUnits);
	void attackCastle();
	void removeCastle(uint16 castleId, uint16 playerId);

	//setter,getter
	//아이디에 맞는 세션을 반환	
	void setPlayer(std::shared_ptr<Player> player, uint16_t playerId);
	std::shared_ptr<Session> getSession(uint16_t id);

	//싱글톤 함수
	static Game& get();

private:
	//싱글톤 정적 변수
	static Game* _instance;

	//클라이언트 아이디
	std::atomic<uint16> _clientId = 4;

	//unitId
	std::atomic<uint16> _unitId = 10;

	HANDLE _iocpHandle;
	OverlappedExPool* _overlappedPool;

private:
	//맵 너비, 높이
	uint16_t _width = 256;
	uint16_t _height = 98;

	std::vector<uint8_t> _tiles;
	std::mutex _tileLock;

private:
	std::map<uint16, std::shared_ptr<Session>> sessions;
	std::map<uint16, std::shared_ptr<Player>> players;

	std::unordered_set<uint16> _moveUnits;
	std::unique_ptr<QuadTree> _tree;
	std::mutex _treeLock;

	Rect _mapBoundary;
	int _capacity = 4;

	std::vector<AttackEvent> events;
	std::set<std::pair<int, int>> processed;

	std::mutex _attackeUnitsLock;

	std::mutex _unitsLock;
	std::map<uint16, std::shared_ptr<Unit>> _units;
	//std::unordered_map<uint16, std::shared_ptr<Unit>> _player2Units;

	Vector2 castle1Pos = Vector2(22, 72); 
	Vector2 castle2Pos = Vector2(221, 81);
	std::map<uint16, Castle*> _castles;
};

