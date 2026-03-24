#pragma once
//===============================
//			Session
// 오직 네트워크 기능만 하는 클래스
//===============================
class Player;

class Session
{
public:
	Session()
		:_recvBuf(0)
	{
		_id = -1;
		_socket = INVALID_SOCKET;
	}

	Session(OverlappedExPool& pool);
	

	void doSend(const char* packet, uint16_t packetSize);
	void doRecv();

public:
	//리시브를 받는 오버랩드 구조체
	OverlappedEx _recvOver;

	//send하는 오버랩드 구조제
	OverlappedEx* _sendOver;

	//받은 파일의 바이트 수 저장
	uint16_t _recvBytes = 0;

	//1:1 소켓
	SOCKET _socket;

	//세션 아이디
	int _id;

	//recv를 받을 링버퍼
	RingBuffer _recvBuf;


private:
	std::weak_ptr<Player> _player;
	OverlappedExPool* _overlappedPool;

};