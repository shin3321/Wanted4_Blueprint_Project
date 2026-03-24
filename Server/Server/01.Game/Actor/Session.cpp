#include "pch.h"
#include "Session.h"

Session::Session(OverlappedExPool& pool)
	: _overlappedPool(&pool), _recvBuf(BUFSIZE)
{
}

void Session::doSend(const char* packet, uint16_t packetSize)
{
	_sendOver = _overlappedPool->allocOver();
	_sendOver->reset(packet, packetSize);
	_sendOver->_type = OP_TYPE::SEND;

	int ret = WSASend(_socket, &_sendOver->_wsaBuf, 1, nullptr, 0,&_sendOver->_overlapped, nullptr);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
		{
			std::cerr << "WSASend Error: " << err << std::endl;
			_overlappedPool->freeOver(_sendOver);
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
