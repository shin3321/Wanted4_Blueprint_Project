#pragma once

constexpr uint16 PK_SC_MAP_LOAD = 1;
constexpr uint16 PK_CS_ID = 2;
constexpr uint16 PK_SC_WAITING = 3;
constexpr uint16 PK_SC_GAME_START = 4;
constexpr uint16 PK_SC_CASTLE = 11;

class Packet
{
public:
	Packet() :_readPos(0)
	{
		_buffer.reserve(BUFSIZE);
	}
template<typename T>
	//패킷에 데이터 쓰기
	void write(T value)
	{
		//값의 크기
		size_t size = sizeof(T);
		size_t oldSize = _buffer.size();

		//이미 넣어 놨던 것에 지금 새로 들어온 크기를 더함)
		_buffer.resize(oldSize + size);

		//새로운 값을 버퍼에 넣음
		std::memcpy(&_buffer[oldSize], &value, size);
	}

	// 가공된 버퍼나 배열을 한 번에 밀어넣는 기능
	void writeBuffer(const void* src, size_t size)
	{
		size_t oldSize = _buffer.size();
		_buffer.resize(oldSize + size);
		std::memcpy(&_buffer[oldSize], src, size);
	}

	inline void writeString(const std::string& str)
	{
		uint16_t len = static_cast<uint16_t>(str.size());
		write(len);
		size_t oldSize = _buffer.size();
		_buffer.resize(oldSize + len);
		std::memcpy(&_buffer[oldSize], str.data(), len);
	}

	template<typename T>
	T read(T value)
	{
		T value;
		//읽을 사이즈가 버퍼 크기를 벗어나면 실패
		if (_readPos + sizeof(T) > _buffer.size())
		{
			__debugbreak();
		}
		std::memcpy(&value, &_buffer[_readPos], sizeof(T));
		_readPos += sizeof(T);
		return value;
	}

	size_t size()
	{
		return _buffer.size();
	}

	template<typename T>
	T read()
	{
		T value;
		//읽을 사이즈가 버퍼 크기를 벗어나면 실패
		if (_readPos + sizeof(T) > _buffer.size())
		{
			__debugbreak();
		}
		std::memcpy(&value, &_buffer[_readPos], sizeof(T));
		_readPos += sizeof(T);
		return value;
	}

	std::vector<char>& getBuffer() { return _buffer; }

private:
	std::vector<char> _buffer;
	size_t _readPos;

};

