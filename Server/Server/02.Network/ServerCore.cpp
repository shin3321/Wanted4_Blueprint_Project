#include "pch.h"
#include "ServerCore.h"
#include "01.Game/Game.h"

ServerCore::ServerCore()
	:_overlappedPool(10)
{

}

ServerCore::~ServerCore()
{

}

void ServerCore::init()
{
	WSADATA wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return;
	}
	_listenSocket = ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	//bind
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(GAME_PORT);
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	if (::bind(_listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		return;
	}

	//listen
	if (::listen(_listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		return;
	}

	// iocp 커널 객체 생성
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	//iocp와 소켓 연결
	::CreateIoCompletionPort(reinterpret_cast<HANDLE>(_listenSocket), _iocpHandle, 9990, 0);

	::SOCKADDR_IN clientAddr;
	int addrSize = sizeof(clientAddr);

	//클라이언트 소켓 만들기
	_clientSocket = ::WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	//accept
	_acceptOverlapped._type = OP_TYPE::ACCEPT;

	//최소 16바이트 이상이어야 해서 16을 더해줌
	::AcceptEx(_listenSocket, _clientSocket, _acceptOverlapped._acceptBuf, 0,
		addrSize + 16, addrSize + 16, 0, &_acceptOverlapped._overlapped);

	std::cout << "Waiting for Client Connection\n";

	Game::get().init(_iocpHandle, _overlappedPool);

	int MaxThreadNum = getCore();
	std::vector<std::thread> workThreads;
	for (int i = 0; i < MaxThreadNum; ++i)
	{
		workThreads.emplace_back([this]() {
			this->runWorkThread();
			});
	}

	std::thread timerThread{ &ServerCore::doTimer, this };
	std::thread dbThread{ &ServerCore::dbThread , this };
	std::thread inputThread([this, MaxThreadNum]()
		{
			while (true)
			{
				if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
				{
					std::cout << "Server Shutting Down...\n";
					_running = false;  // 종료 플래그
					timer_cv.notify_all();
					// IOCP 워커 스레드 깨우기
					for (int i = 0; i < MaxThreadNum; ++i)
						PostQueuedCompletionStatus(_iocpHandle, 0, 0, nullptr);
					break;
				}
				Sleep(100);
			}
		});

	inputThread.join();

	if (timerThread.joinable()) timerThread.join();

	for (auto& thread : workThreads)
	{
		if (thread.joinable()) thread.join();
	}

	closesocket(_listenSocket);
	WSACleanup();
}

void ServerCore::registerHandle(HANDLE handle)
{

}

void ServerCore::runWorkThread()
{
	while (_running)
	{
		//받은 크기
		DWORD numbytes;

		//구분할 키 값
		ULONG_PTR key;

		//반환 받을 Overlapped 구조체
		WSAOVERLAPPED* over = nullptr;

		//완료된 io 요청 받기 
		BOOL ret = GetQueuedCompletionStatus(_iocpHandle, &numbytes, &key, &over, INFINITE);

		OverlappedEx* ex_over = reinterpret_cast<OverlappedEx*>(over);
		if (over == nullptr)
		{
			std::cout << "Worker Thread Exit\n";
			return;
		}

		if (FALSE == ret)
		{
			if (ex_over->_type == OP_TYPE::ACCEPT)
				std::cout << "accept Error\n";
			else
			{
				std::cout << "GQCS Error on client[" << key << "]\n";
				if (ex_over->_type == OP_TYPE::SEND) _overlappedPool.freeOver(ex_over);
				continue;
			}
		}
		if ((numbytes == 0) && ((ex_over->_type == OP_TYPE::RECV) || (ex_over->_type == OP_TYPE::SEND)))
		{
			if (ex_over->_type == OP_TYPE::SEND) _overlappedPool.freeOver(ex_over);
			continue;
		}

		switch (ex_over->_type)
		{
		case OP_TYPE::SEND:
		{
			_overlappedPool.freeOver(ex_over);

			break;
		}
		case OP_TYPE::ACCEPT:
		{
			Game::get().accept(_clientSocket);

			//다른 클라이언트를 받기 위해 accept 열어 두기
			_clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			ZeroMemory(&_acceptOverlapped, sizeof(_acceptOverlapped._overlapped));
			int addrSize = sizeof(SOCKADDR_IN);
			//accept
			_acceptOverlapped._type = OP_TYPE::ACCEPT;
			AcceptEx(_listenSocket, _clientSocket, _acceptOverlapped._acceptBuf, 0, addrSize + 16, addrSize + 16, 0, &_acceptOverlapped._overlapped);
			break;
		}
		case OP_TYPE::RECV:
		{
			//받은ㅇ 데이터가 없으면 소켓 닫음
			if (numbytes == 0)
			{
				std::cout << "Recv Buffer Overflow. Dropping Session\n";
				Game::get().closeSocket(key);
			}

			numbytes = static_cast<uint16_t>(numbytes);
			Game::get().recv(key, numbytes);
			break;
		}

		}

	}
}

void ServerCore::doTimer()
{
	while (_running)
	{
		std::unique_lock<std::mutex> lock(timer_mutex);
		if (timer_queue.empty())
		{
			timer_cv.wait(lock);
			continue;
		}

		auto now = std::chrono::system_clock::now();
		auto nextEvent = timer_queue.top();

		//다음 이벤트 시작 시간이 남았으면 기다림
		if (nextEvent.wakeUpTime > now)
		{
			timer_cv.wait_until(lock, nextEvent.wakeUpTime);
			continue;
		}

		timer_queue.pop();
		lock.unlock();

		auto timerOver = _overlappedPool.allocOver();

		switch (nextEvent._eventType)
		{
		case TimerEvent::EV_GAME_START:
		{
			timerOver->_type = OP_TYPE::GAME_START;
			timerOver->_targetId = nextEvent.playerId;
			PostQueuedCompletionStatus(_iocpHandle, 1, nextEvent.playerId, &timerOver->_overlapped);
			break;
		}
		}

	}
}

void ServerCore::dbThread()
{
}

int ServerCore::getCore()
{
	DWORD bufferSize = 0;

	//필요한 버퍼 크기 확인
	//논리 프로세서 코어 정보 가져오기
	GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bufferSize);

	//논리 프로세서 코어 크기만큼 벡터 생성
	std::vector<BYTE> buffer(bufferSize);
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX pInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer.data();

	int pCoreLogicalCount = 0;

	//실제 데이터 가져오기
	if (GetLogicalProcessorInformationEx(RelationProcessorCore, pInfo, &bufferSize))
	{
		DWORD offset = 0;
		while (offset < bufferSize)
		{
			pInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)(buffer.data() + offset);
			if (pInfo->Relationship == RelationProcessorCore)
			{
				// EfficiencyClass가 0보다 크면 P-Core로 판단
				if (pInfo->Processor.EfficiencyClass > 0) {
					// 해당 물리 코어에 속한 논리 프로세서(하이퍼스레딩 포함) 개수 합산
					for (WORD i = 0; i < pInfo->Processor.GroupCount; ++i) {
						// 하위 비트 중 1의 개수를 세어 논리 프로세서 수를 구함
						unsigned __int64 mask = pInfo->Processor.GroupMask[i].Mask;
						while (mask) {
							if (mask & 1) pCoreLogicalCount++;
							mask >>= 1;
						}
					}
				}
			}
			offset += pInfo->Size;
		}
	}
	// 정보를 가져오지 못한 경우 안전하게 기본값 반환
	return pCoreLogicalCount > 0 ? pCoreLogicalCount : std::thread::hardware_concurrency();
}