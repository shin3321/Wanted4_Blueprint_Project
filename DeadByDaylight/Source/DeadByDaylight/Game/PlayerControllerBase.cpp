// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"
#include "DeadByDaylight/DeadByDaylight.h"
#include "DeadByDaylight/Header/MyPacketStructs.h"


void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	OwnerPlayer = GetPawn();
	GameInst = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	if (GameInst)
	{
		PlayerId = GameInst->GetMyId();
	}

	GetWorldTimerManager().SetTimer(SendPacketTimerHandle, this, &APlayerControllerBase::SendMovePacket, NetworkSendInterval, true);
}

void APlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayerReason)
{
}

void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerControllerBase::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
}

void APlayerControllerBase::SendMovePacket()
{
	if (!GameInst || !GameInst->bGameStarted) return;
	APawn* CurrentPawn = GetPawn();
	if (!CurrentPawn) return;

	Rotation PlayerRotation(OwnerPlayer->GetActorRotation().Pitch, OwnerPlayer->GetActorRotation().Yaw, OwnerPlayer->GetActorRotation().Roll);
	Location PlayerLocation(OwnerPlayer->GetActorLocation().X, OwnerPlayer->GetActorLocation().Y, OwnerPlayer->GetActorLocation().Z);

	FC_MovePacket MovePacket;
	MovePacket.Header.PacketType = EPacketType::C_Move;
	MovePacket.Header.PacketSize = sizeof(FC_MovePacket);
	MovePacket.PlayerId = PlayerId;
	MovePacket.PlayerLocation = PlayerLocation;
	MovePacket.PlayerRotation = PlayerRotation;

	SEND_PACKET(MovePacket);
