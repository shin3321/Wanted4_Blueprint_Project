// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"
#include "DeadByDaylight/DeadByDaylight.h"
#include "DeadByDaylight/Header/MyPacketStructs.h"
#include "DeadByDaylight/Network/MyGameInstance.h"


void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	OwnerPlayer = GetPawn();
	UMyGameInstance* GameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	PlayerId = GameInstance->GetMyId();

	GetWorldTimerManager().SetTimer(SendPacketTimerHandle, this, &APlayerControllerBase::SendMovePacket, NetworkSendInterval, true);
}

void APlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayerReason)
{
}

void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	

}

void APlayerControllerBase::SendState(FString NewState)
{
	FC_ChangeStatePacket StatePacket;
	StatePacket.Header.PacketType = EPacketType::C_ChangeState;
	StatePacket.Header.PacketSize = sizeof(FC_ChangeStatePacket);
	StatePacket.PlayerId = PlayerId;

	FTCHARToUTF8 ConvertState(NewState);
	FCStringAnsi::Strncpy(StatePacket.State, (const char*)ConvertState.Get(), sizeof(StatePacket.State) - 1);
	StatePacket.State[sizeof(StatePacket.State) - 1] = '\0';

	SEND_PACKET(StatePacket);
}

void APlayerControllerBase::SendMovePacket()
{
	Rotation PlayerRotation (OwnerPlayer->GetActorRotation().Pitch, OwnerPlayer->GetActorRotation().Yaw, OwnerPlayer->GetActorRotation().Roll);
	Location PlayerLocation (OwnerPlayer->GetActorLocation().X, OwnerPlayer->GetActorLocation().Y, OwnerPlayer->GetActorLocation().Z);

	FC_MovePacket MovePacket;
	MovePacket.Header.PacketType = EPacketType::C_Move;
	MovePacket.Header.PacketSize = sizeof(FC_MovePacket);
	MovePacket.PlayerId = PlayerId;
	MovePacket.PlayerLocation = PlayerLocation;
	MovePacket.PlayerRotation = PlayerRotation;

	SEND_PACKET(MovePacket);
}
