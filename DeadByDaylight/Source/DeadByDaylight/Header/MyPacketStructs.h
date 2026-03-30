// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
constexpr int BUFSIZE = 8192;

enum class EPacketType : uint16_t
{
	C_Login = 1,
	S_LoginResult,
};

struct FPacketHeader
{
	uint16_t PacketSize;
	EPacketType PacketType;
};

#pragma pack(push, 1)
struct FCLoginPacket
{
	FPacketHeader Header;
	char UserId[20];
	char UserPw[20];
	bool IsKiller;
};

struct FSLoginResultPacket
{
	FPacketHeader Header;
	char UserId[20];
	char UserPw[20];
	bool IsKiller;
};
#pragma pack(pop)