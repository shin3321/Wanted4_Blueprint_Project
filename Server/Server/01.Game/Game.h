#pragma once

#include <set>
class Player;
class Session;

//class Unit;


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

};

