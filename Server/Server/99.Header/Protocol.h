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

	friend std::ostream& operator<< (std::ostream& os, const Location& loc)
	{
		return os << "(" << loc.x << ", " << loc.y << ", " << loc.z << ")\n";
	}

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

struct PlayerStartInfo
{
	int32_t PlayerId;
	Location StartLocation;
	bool Iskiller;
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
	C_ProjectileAxe,
	S_ProjectileAxe
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
	int32_t PlayerId;
	PlayerStartInfo StartInfo[5];
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
	uint8_t StateLen;
	char State[1];
};

struct S_ChangeStatePacket
{
	PacketHeader Header;
	int32_t PlayerId;
	uint8_t StateLen;
	char State[1];
};

struct C_ProjectileAxe
{
	PacketHeader Header;
	Location AxeStartLoc;
};

struct S_ProjectileAxe
{
	PacketHeader Header;
	int32_t AxeId;
	Location AxeStartLoc;
};

#pragma pack(pop)