#pragma once
#include <Windows.h>

class Vector2
{
public:
	Vector2() {}
	Vector2(int x, int y) : x(x), y(y) {}
	~Vector2()
	{
		if (string)
		{
			delete[] string;
			string = nullptr;
		}
	}

	// 문자열로 변환해서 반환하는 함수.
	inline const char* ToString()
	{
		if (string)
		{
			delete[] string;
			string = nullptr;
		}
		string = new char[128];
		memset(string, 0, sizeof(char) * 128);
		sprintf_s(string, 128, "(%d, %d)", x, y);
		return string;
	}

	// 연산자 오버로딩.
	inline Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
	inline Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
	inline bool operator==(const Vector2& other) const { return (x == other.x) && (y == other.y); }
	inline bool operator!=(const Vector2& other) const { return !(*this == other); }

	// 형변환 연산자 오버로딩.
	inline operator COORD() const
	{
		COORD coord = {};
		coord.X = static_cast<short>(x);
		coord.Y = static_cast<short>(y);
		return coord;
	}

	// 벡터 기본 값.
	static Vector2 Zero;
	static Vector2 One;
	static Vector2 Up;
	static Vector2 Right;

public:
	int x = 0;
	int y = 0;

private:
	char* string = nullptr;
};

inline Vector2 Vector2::Zero(0, 0);
inline Vector2 Vector2::One(1, 1);
inline Vector2 Vector2::Up(0, 1);
inline Vector2 Vector2::Right(1, 0);