// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"
#include "DeadByDaylight/DeadByDaylight.h"

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerControllerBase::SendLoginPacket(FString UserId, FString UserPw, bool IsKller)
{

}
