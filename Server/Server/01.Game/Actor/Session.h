#pragma once
//===============================
//			Session
// 오직 네트워크 기능만 하는 클래스
//===============================
#include "01.Game/Actor/Player.h"

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session()
		:_recvBuf(0)
	{
		_id = -1;
		_socket = INVALID_SOCKET;
	}

	Session(OverlappedExPool& pool);	

	void setPlayer(std::shared_ptr<Player> player)
	{
		_player = player;
		_player->setSession(shared_from_this());
	}

	void setRoomId(uint16 roomId) { _roomId = roomId; }

	void doSend(void* packet, uint16_t packetSize);
	void doRecv();


public:
	//리시브를 받는 오버랩드 구조체
	OverlappedEx _recvOver;

	//받은 파일의 바이트 수 저장
	uint16_t _recvBytes = 0;

	//1:1 소켓
	SOCKET _socket;

	//세션 아이디
	int _id;

	uint16 _roomId;


	//recv를 받을 링버퍼
	RingBuffer _recvBuf;


private:
	std::shared_ptr<Player> _player;
	OverlappedExPool* _overlappedPool;

};