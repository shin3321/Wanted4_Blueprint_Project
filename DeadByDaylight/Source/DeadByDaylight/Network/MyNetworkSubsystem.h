// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DeadByDaylight/Network/PlayerManager.h"
#include <Sockets.h>
#include "MyNetworkSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class DEADBYDAYLIGHT_API UMyNetworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues);

	void SetSocket(FSocket* Socket) { ClientSocket = Socket; }
	
public:
	//Recv 함수
	void ReceiveData();
	void ResetBuffer();
	//Recv한 패킷을 처리하는 함수
	void ProcessQueuePackets();

private:
	FSocket* ClientSocket;
	
	//받은 패킷을 저장하는 큐
	TQueue<TArray<uint8>, EQueueMode::Spsc> ReceiveQueue;
	FCriticalSection QueueLock;
	
	TArray<uint8> ReceiverBuffer;
	
	class UMyGameInstance* GameInst;
	APlayerManager* PlayerMng; 
	const int32 MAX_PACKET_SIZE = 65535;
};
