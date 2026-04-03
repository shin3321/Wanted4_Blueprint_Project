// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
constexpr int BUFSIZE = 8192;

#pragma pack(push, 1)
struct Location
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
		:pitch(pitch), yaw(yaw), roll(roll) {}
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

struct FPacketHeader
{
	uint16 PacketSize;
	EPacketType PacketType;
};

struct FC_LoginPacket
{
	FPacketHeader Header;
	char UserId[20];
	char UserPw[20];
	bool IsKiller;
};

struct FS_LoginResultPacket
{
	FPacketHeader Header;
	int32 BooldPoint;
	int32 PlayerId;
};

struct FC_WaitingPacket
{
	FPacketHeader Header;
	int32 PlayerId;
	bool IsKiller;
};

struct FS_WaitingResultPacket
{
	FPacketHeader Header;
	int32 PlayerId;
};

struct FS_WaitingPacket
{
	FPacketHeader Header;
	int32_t PlayerId;
	bool IsKiller;
	Location PlayerLocation;
};

struct FS_WaitingPlayerInfoPacket
{
	FPacketHeader Header;
	int32 PlayerId;
	bool IsKiller;
	Location Location;
};

struct FC_ReadyPacket
{
	FPacketHeader Header;
	int32 PlayerId;
};

struct FS_ReadyPacket
{
	FPacketHeader Header;
	int32 PlayerId;
};

struct FS_StartPacket
{
	FPacketHeader Header;
};

struct FC_MovePacket
{
	FPacketHeader Header;
	int32 PlayerId;
	Location PlayerLocation;
	Rotation PlayerRotation;
};

struct FS_MovePacket
{
	FPacketHeader Header;
	int32_t PlayerId;
	Location PlayerLocation;
	Rotation PlayerRotation;
};

struct FC_ChangeStatePacket
{
	FPacketHeader Header;
	int32 PlayerId;
	char State[15];
};

struct FS_ChangeStatePacket
{
	FPacketHeader Header;
	int32_t PlayerId;
	char State[15];
};
#pragma pack(pop)