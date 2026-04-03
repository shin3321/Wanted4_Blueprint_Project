#include "pch.h"
#include "Session.h"
#include "99.Header/Protocol.h"

Session::Session(OverlappedExPool& pool)
	: _overlappedPool(&pool), _recvBuf(BUFSIZE)
{
}

void Session::doSend(void* packet, uint16_t packetSize)
{
	// 1. void*를 헤더 포인터로 형변환 (Casting)
	PacketHeader* header = static_cast<PacketHeader*>(packet);

	// 2. 이제 원하는 정보를 읽을 수 있습니다.
	// 로그를 찍어서 값이 제대로 들어오는지 확인해보세요.
	if (header != nullptr)
	{
		uint16_t size = header->PacketSize;
		EPacketType id = header->PacketType;

		// 디버깅용 출력 (std::cout 혹은 사용하시는 로거 사용)
		 std::cout << ", Size: " << size << std::endl;
	}
	OverlappedEx* sendOver = _overlappedPool->allocOver();
	sendOver->reset(reinterpret_cast<char*>(packet), packetSize);
	sendOver->_wsaBuf.buf = sendOver->_buffer.data();
	sendOver->_wsaBuf.len = packetSize;
	sendOver->_type = OP_TYPE::SEND;

	unsigned char* p = (unsigned char*)sendOver->_wsaBuf.buf;
	printf("Send Bytes: %02x %02x %02x %02x\n", p[0], p[1], p[2], p[3]);

	int ret = WSASend(_socket, &sendOver->_wsaBuf, 1, nullptr, 0, &sendOver->_overlapped, nullptr);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			std::cerr << "WSASend Error: " << err << std::endl;
			_overlappedPool->freeOver(sendOver);
		}
	}
}

void Session::doRecv()
{
	DWORD recvFlag = 0;
	ZeroMemory(&_recvOver._overlapped, sizeof(_recvOver._overlapped));
	_recvOver._wsaBuf.buf = (char*)&_recvBuf._buffer[_recvBuf._tail];
	_recvOver._wsaBuf.len = _recvBuf.freeSize();
	_recvOver._type = OP_TYPE::RECV;

	//_recvOver의 버퍼를 io 요청해 두는 것
	int ret = ::WSARecv(_socket, &_recvOver._wsaBuf, 1, 0, &recvFlag, &_recvOver._overlapped, 0);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			std::cerr << "WSARecv Error: " << err << std::endl;
			return;
		}
	}
}
