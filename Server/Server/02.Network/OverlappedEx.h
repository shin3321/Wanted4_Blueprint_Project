#pragma once

enum class OP_TYPE
{
	ACCEPT,
	RECV,
	SEND,
	GAME_START,
	UNIT_ATTACK,

};

//OVERLAPPED 확장 구조체
struct OverlappedEx
{
	WSAOVERLAPPED _overlapped;
	// 데이터 버퍼
	WSABUF _wsaBuf;
	char _acceptBuf[BUFSIZE];
	std::vector<char>  _buffer;
	OP_TYPE _type;
	uint16_t _targetId;

	//recv용 생성자
	OverlappedEx()
	{
		_wsaBuf.len = BUFSIZE;
		_wsaBuf.buf = _buffer.data();
		ZeroMemory(&_overlapped, sizeof(_overlapped));
		ZeroMemory(_acceptBuf, sizeof(_acceptBuf));
		//_type = OP_TYPE::RECV;
	}

	//send용 생성자
	OverlappedEx(char* packet, size_t packetSize)
	{
		_buffer.assign(packet, packet + packetSize);        
		_wsaBuf.len = static_cast<ULONG>(packetSize);
		_wsaBuf.buf = _buffer.data();
		ZeroMemory(&_overlapped, sizeof(_overlapped));
		_type = OP_TYPE::SEND;
	}    
	
	// 재사용 시 초기화
	void reset(const char* packet, size_t packetSize)
	{
		_buffer.assign(packet, packet + packetSize);
		_wsaBuf.len = static_cast<ULONG>(packetSize);
		_wsaBuf.buf = _buffer.data();
		ZeroMemory(&_overlapped, sizeof(_overlapped));
	}
};

class OverlappedExPool
{
public:
	//생성
	OverlappedExPool(int size)
	{
		for (int i = 0; i < size; ++i)
		{
			_pool.push(new OverlappedEx());
		}
	}

	//할당
	OverlappedEx* allocOver()
	{
		//pool이 비어 있으면 재할당
		if (_pool.empty())
		{
			return new OverlappedEx();
		}

		OverlappedEx* overlapped = _pool.front();
		_pool.pop();
		return overlapped;
	}

	//반납
	void freeOver(OverlappedEx* overlapped)
	{
		if (overlapped)
		{
			overlapped->_buffer.resize(BUFSIZE);
			overlapped->_wsaBuf.len = BUFSIZE;
			overlapped->_wsaBuf.buf = overlapped->_buffer.data();
			ZeroMemory(&overlapped->_overlapped, sizeof(WSAOVERLAPPED));

			std::lock_guard<std::mutex> lock(_poolMutex);
			_pool.push(overlapped);
		}
	}

private:
	std::mutex _poolMutex;
	std::queue<OverlappedEx*> _pool;
};