#pragma once
constexpr short GAME_PORT = 9999;
constexpr short BUFSIZE = 8192;

//constexpr int PK_SC_MAP_LOAD = 1;

#pragma pack(push, 1)

class Location
{
public:
	Location() {}
	Location(double x, double y, double z) : x(x), y(y), z(z) {}

public:
	double x = 0;
	double y = 0;
	double z = 0;
};

struct Rotation
{
public:
	double pitch;
	double yaw;
	double roll;

	Rotation() {}
	Rotation(double pitch, double yaw, double roll)
		:pitch(pitch), yaw(yaw), roll(roll) {
	}
};

enum class EPacketType : uint16_t
{
	C_Login = 1,
	S_LoginResult,
	C_Waiting,
	S_Waiting,
	S_WaitingResult,
	S_WaitingPlayerInfo,
	C_Ready,
	S_Ready, 
	S_GameStart,
	C_Move,
	S_Move,
	C_ChangeState,
	S_ChangeState,
};

struct PacketHeader
{
	uint16_t PacketSize;
	EPacketType PacketType;
};

struct C_LoginPacket
{
	PacketHeader Header;
	char Id[20];
	char Pw[20];
};

struct S_LoginResultPacket
{
	PacketHeader Header;
	int32_t bloodPoint;
	int32_t playerId;
};

struct C_WaitingPacket
{
	PacketHeader Header;
	int32_t PlayerId;
	bool IsKiller;
};

struct S_WaitingResultPacket
{
	PacketHeader Header; 
	int32_t PlayerId;
};

struct S_WaitingPacket
{
	PacketHeader Header;
	int32_t PlayerId;
	bool isKiller;
	Location location;
};

struct S_WaitingPlayerInfoPacket
{
	PacketHeader Header;
	int32_t PlayerId;
	bool isKiller;
	Location location;
};

struct C_ReadyPacket
{
	PacketHeader Header;
	int32_t PlayerId;
};

struct S_ReadyPacket
{
	PacketHeader Header;
	int32_t PlayerId;
};

struct S_StartPacket
{
	PacketHeader Header;
};

struct C_MovePacket
{
	PacketHeader Header;
	int32_t PlayerId;
	Location PlayerLocation;
	Rotation PlayerRotation;
};

struct S_MovePacket
{
	PacketHeader Header;
	int32_t PlayerId;
	Location PlayerLocation;
	Rotation PlayerRotation;
};

struct C_ChangeStatePacket
{
	PacketHeader Header;
	int32_t PlayerId;
}; 

struct S_ChangeStatePacket
{
	PacketHeader Header;
	int32_t PlayerId;
	char State[15];
};
#pragma pack(pop)