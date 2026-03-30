// Fill out your copyright notice in the Description page of Project Settings.


#include "DeadByDaylight/Network/PacketDispatcher.h"

FPacketDispatcher::FPacketDispatcher()
{
	REGISTER_HANDLER(EPacketType::S_LoginResult, HandleLogin);
}

FPacketDispatcher::~FPacketDispatcher()
{
}

void FPacketDispatcher::HandleLogin(uint8* buffer)
{

}
