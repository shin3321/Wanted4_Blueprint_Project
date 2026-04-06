// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "DeadByDaylight/Game/PlayerControllerBase.h"
#include "DeadByDaylight/Network/MyGameInstance.h"
#include "DeadByDaylight/Network/MyNetworkSubsystem.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

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

void AMyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
    if (!GI) return;

    bool bIsKiller = GI->GetMyIsKiller(); 

    //  컨트롤러 교체 로직이 필요한 경우 여기서 처리할 수 있지만, 
    // 우선은 이미 생성된 컨트롤러에 알맞은 Pawn을 연결해주는 것이 중요합니다.
    // SpawnPawnAndPossess(NewPlayer);
    }

    void AMyGameModeBase::PossessAllPlayers()
    {
    TArray<AActor*> FoundCharacters;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMyCharacterBase::StaticClass(), FoundCharacters);

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerControllerBase* PC = Cast<APlayerControllerBase>(It->Get());
        if (!PC) continue;

        int32 PC_Id = PC->GetPlayerId();

        for (AActor* Actor : FoundCharacters)
        {
            AMyCharacterBase* Character = Cast<AMyCharacterBase>(Actor);
            if (Character && !Character->GetController())
            {
                if (Character->GetPlayerId() == PC_Id)
                {
                    PC->Possess(Character);
                    UE_LOG(LogTemp, Display, TEXT("Possessed Player %d"), PC_Id);
                    break;
                }
            }
        }
    }
    }

    void AMyGameModeBase::SpawnPawnAndPossess(APlayerController* NewPlayer)
{
    if (!NewPlayer) return;

    UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
    if (!GI) return;

    bool bIsKiller = GI->GetMyIsKiller();

    TSubclassOf<APawn> PawnClass = SurvivorPawnClass;
    
    if (PawnClass)
    {
        AActor* StartSpot = FindPlayerStart(NewPlayer);
        FVector Location = StartSpot ? StartSpot->GetActorLocation() : FVector::ZeroVector;
        FRotator Rotation = StartSpot ? StartSpot->GetActorRotation() : FRotator::ZeroRotator;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = NewPlayer;
        SpawnParams.Instigator = GetInstigator();

        APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PawnClass, Location, Rotation, SpawnParams);
        if (NewPawn)
        {
            NewPlayer->Possess(NewPawn);
        }
    }
}

void AMyGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    // 주기적으로(예: 0.5초마다) 빙의되지 않은 캐릭터가 있는지 확인하고 자동 빙의 시도
    static float Accumulator = 0.0f;
    Accumulator += DeltaTime;
    if (Accumulator >= 0.5f)
    {
        PossessAllPlayers();
        Accumulator = 0.0f;
    }

	auto NetworkManager = GetGameInstance()->GetSubsystem<UMyNetworkSubsystem>();
	if (NetworkManager)
	{
		NetworkManager->ProcessQueuePackets();
	}
}

APlayerController* AMyGameModeBase::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());

	return Super::SpawnPlayerController(InRemoteRole, Options);
}
