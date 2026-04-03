// Fill out your copyright notice in the Description page of Project Settings.


#include "DeadByDaylight/Network/PlayerManager.h"
#include "DeadByDaylight/Network/MyGameInstance.h"

void APlayerManager::BeginPlay()
{
	this->Tags.Add("PlayerManager");
	UMyGameInstance* GameInst = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	MyPlayerId = GameInst->GetMyId();
}

void APlayerManager::HandleMove(int32 PlayerId, FVector PlayerLocation, FRotator PlayerRotator)
{
	if (PlayerId == MyPlayerId)
		return;

	if (AMyCharacterBase* Remote = Cast<AMyCharacterBase>(Players.FindRef(PlayerId)))
	{
		Remote->OnReceiveMovePacket(PlayerLocation, PlayerRotator);
	}
}

void APlayerManager::InsertPlayers(int32 PlayerId, AMyCharacterBase* Player)
{
	if (Player)
		Players.Add(PlayerId, Player);
}
