#pragma once

#include <set>

class Player;
class Session;
class Room;

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
	void recv(int32 key, uint16_t numbytes);

	//클라이언트 아이디 생성
	int32 getClientId();
	int32 getRoomId();

	//로그인 정보 전달
	void sendLoginResult(int32 playerId, int bloodPoint);

	//게임 시작 시 방 미리 만들어 두기
	void makeRoom();
	
	void sendWaitResult(int32 playerId, bool isKiller);

	// 방 배정 함수
	void assignRoom(int32 playerId, bool isKiller);
	void readyRoom(C_ReadyPacket* readyPacket);

	//움직임 전달 함수
	void sendMove(C_MovePacket* movePacket);

	// 캐릭터 상태 전송 함수
	void recvState(C_ChangeStatePacket* statePacket);

	//closeSocket 함수 - 플레이어의 소켓을 닫는 함수
	void closeSocket(int sessioneId);
	void broadcast(const char* data, uint16 packetSize);

	//setter,getter
	//아이디에 맞는 세션을 반환	
	std::shared_ptr<Session> getSession(int32 id);
	std::shared_ptr<Player> getPlayer(uint32 playerId);

	std::shared_ptr<Room> getRoom(int32 sessionId);

	//싱글톤 함수
	static Game& get();

private:
	//싱글톤 정적 변수
	static Game* _instance;

	//클라이언트 아이디
	std::atomic<uint16> _clientId = 0;

	//unitId
	std::atomic<uint16> _roomId = 0;

	HANDLE _iocpHandle;
	OverlappedExPool* _overlappedPool;

private:
	std::unordered_map<uint16, std::shared_ptr<Session>> sessions;
	std::unordered_map<uint16, std::shared_ptr<Room>> rooms;
};

