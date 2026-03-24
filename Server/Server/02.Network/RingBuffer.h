#pragma once
#include <iostream>
#include <vector>

class RingBuffer
{
public:
	RingBuffer()
		:_buffer(BUFSIZE), _capacity(BUFSIZE)
	{
	}
	RingBuffer(size_t capacity)
		:_buffer(capacity), _capacity(capacity)
	{
	}

	bool empty() const 
	{
		return _head == _tail;
	}
	
	bool full() const
	{
		return (_tail + 1) % _capacity == _head;
	}

	size_t size()
	{
		if (_tail >= _head)
			return _tail - _head;
		else
			return _capacity - _head + _tail;
	}

	//버퍼의 남은 양 확인
	size_t freeSize()
	{
		return _capacity - size() - 1;
	}

	char* date() { return _buffer.data(); }
	size_t capacity() const { return _buffer.size(); }

	bool write(const char* data, size_t len)
	{
		//길이가 남은 양보다 많으면 실패 반환
		if (len > freeSize())
			return false;

		for (size_t i = 0; i < len; ++i)
		{
			_buffer[_tail] = data[i];
			_tail = (_tail + 1) % _capacity;
		}
		return true;
	}

	bool read(char* dest, size_t len)
	{
		//읽을 데이터가 버퍼의 사이즈보다 크면 실패 반환
		if (len > size())
			return false;

		//버퍼의 데이터를 dest에 넣기
		for (int i = 0; i < len; ++i)
		{
			dest[i] = _buffer[_head];
			_head = (_head + 1) % _capacity;
		}
		return true;
	}

	bool peek(char* dest, size_t len)
	{
		//읽을 데이터가 버퍼의 사이즈보다 크면 실패 반환
		if (len > size())
		{
			return false;
		}

		size_t temp = _head;
		for (int i = 0; i < len; ++i)
		{
			dest[i] = _buffer[temp];
			temp = (temp + 1) % _capacity;
		}
		return true;
	}

public:
	std::vector<char> _buffer;
	size_t _capacity;
	size_t _head = 0;
	size_t _tail = 0;
};

