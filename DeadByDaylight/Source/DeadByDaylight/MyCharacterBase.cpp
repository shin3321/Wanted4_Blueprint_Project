// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterBase.h"
#include "DeadByDaylight/DeadByDaylight.h"
#include "DeadByDaylight/Header/MyPacketStructs.h"
#include "DeadByDaylight/Network/MyGameInstance.h"

// Sets default values
AMyCharacterBase::AMyCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	GameInst = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
	PlayerId = GameInst->GetMyId();

}

// Called every frame
void AMyCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMyCharacterBase::SendState(FString NewState)
{
	FTCHARToUTF8 ConvertState(NewState);
	int32 Len = ConvertState.Length();

	int32 PacketSize = sizeof(FC_ChangeStatePacket) + Len;

	FC_ChangeStatePacket* Packet = (FC_ChangeStatePacket*)FMemory::Malloc(PacketSize);

	Packet->Header.PacketType = EPacketType::C_ChangeState;
	Packet->Header.PacketSize = PacketSize;
	Packet->PlayerId = PlayerId;
	Packet->StateLen = Len;

	memcpy(Packet->State, ConvertState.Get(), Len);

	SEND_PACKET_RAW(Packet, PacketSize);
	FMemory::Free(Packet);
}

void AMyCharacterBase::OnReceiveState(const FString NewState)
{
	OnRecvStateChange.Broadcast(NewState);
}

void AMyCharacterBase::OnReceiveMovePacket(FVector NewLocation, FRotator NewRotation)
{
	NewTargetLocation = NewLocation;
	NewTargetRotation = NewRotation;
}