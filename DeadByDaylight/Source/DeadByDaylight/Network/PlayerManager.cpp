// Fill out your copyright notice in the Description page of Project Settings.


#include "DeadByDaylight/Network/PlayerManager.h"
#include "DeadByDaylight/Network/MyGameInstance.h"

void APlayerManager::BeginPlay()
{
	this->Tags.Add("PlayerManager");
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

void APlayerManager::HandleChangeState(int32 PlayerId, const FString NewState)
{
	if (PlayerId == MyPlayerId)
		return;

	if (AMyCharacterBase* Remote = Cast<AMyCharacterBase>(Players.FindRef(PlayerId)))
	{
		Remote->OnReceiveState(NewState);
	}
}

void APlayerManager::InsertPlayers(int32 PlayerId, AMyCharacterBase* Player)
{
	if (Player)
		Players.Add(PlayerId, Player);
}

void APlayerManager::SetPlayerId()
{
	UMyGameInstance* GameInst = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	if (GameInst)
		MyPlayerId = GameInst->GetMyId();
	else
		UE_LOG(LogTemp, Display, TEXT("GameInstance Is Null"))
}
