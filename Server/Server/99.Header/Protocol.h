#pragma once
constexpr short GAME_PORT = 9999;
constexpr short BUFSIZE = 8192;

//constexpr int PK_SC_MAP_LOAD = 1;

#pragma pack(push, 1)

enum class EPacketType : uint16_t
{
	C_Login = 1,
	S_LoginResult,
};

struct PacketHeader
{
	uint16_t PacketSize;
	EPacketType PacketType;
};

struct C_LoginPacket
{
	EPacketType Header;
	char Id[20];
	char Pw[20];
};
