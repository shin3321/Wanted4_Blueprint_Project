// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DeadByDaylight/Network/MyGameInstance.h"


#define SEND_PACKET(Packet)\
UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());\
if(!GameInstance)return;\
GameInstance->SendPacket(&Packet, sizeof(Packet));