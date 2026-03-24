#pragma once

class ServerCore
{

public:
	ServerCore();
	~ServerCore();
	//iocp 객체 생성
	void init();
	void registerHandle(HANDLE handle);

	//getqueuecompletionstatus
	void runWorkThread();

	//timer 이벤트 함수
	void doTimer();

	//데이터 베이스 함수
	void dbThread();

	//core 개수
	int getCore();
	
	//getter
	HANDLE getHandle() { return _iocpHandle; }

private:


private:
	//서버 연결 소켓
	SOCKET _listenSocket;
	SOCKET _clientSocket;

	//accpet용 overlapped 구조체
	OverlappedEx _acceptOverlapped;
	OverlappedExPool _overlappedPool;

	//타이머 이벤트용 overlapped구조체
	//OverlappedEx* _timerOver;

	HANDLE _iocpHandle;

	std::atomic<bool> _running = true;
};

