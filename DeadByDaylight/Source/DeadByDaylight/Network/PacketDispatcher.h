// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "DeadByDaylight/Header/MyPacketStructs.h"

#define REGISTER_HANDLER(PacketType, HandlerPacket)\
Handler.Add(PacketType, [this](uint8* buffer){HandlerPacket(buffer);});

using HandlePacket = TFunction<void(uint8*)>;

class DEADBYDAYLIGHT_API FPacketDispatcher
{
public:
	FPacketDispatcher();
	~FPacketDispatcher();

	void HandleLogin(uint8* buffer);
private:
	TMap<EPacketType, HandlePacket> Handler;
};
