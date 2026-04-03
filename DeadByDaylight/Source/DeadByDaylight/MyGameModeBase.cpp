// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "DeadByDaylight/Game/PlayerControllerBase.h"
#include "DeadByDaylight/Network/MyGameInstance.h"
#include "DeadByDaylight/Network/MyNetworkSubsystem.h"

AMyGameModeBase::AMyGameModeBase()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = APlayerController::StaticClass();

	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AMyGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void AMyGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto NetworkManager = GetGameInstance()->GetSubsystem<UMyNetworkSubsystem>();
	if (NetworkManager)
	{
		NetworkManager->ProcessQueuePackets();
	}
}

APlayerController* AMyGameModeBase::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
	if (GI&&GI->bGameStarted)
	{
		PlayerControllerClass = GI->bIsKiller ? KillerControllerClass : SurvivorControllerClass;
	}

	return Super::SpawnPlayerController(InRemoteRole, Options);
}
