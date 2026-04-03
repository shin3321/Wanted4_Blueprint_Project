// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeadByDaylight/MyCharacterBase.h"
#include "PlayerManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMovePlayer, int32, PlayerId, FVector, PlayerLocation, FRotator, PlayerRotator);

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsKiller = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

public:
	void SetId(int32 Id) { PlayerId = Id; }
	void SetIsKiller(bool IsKiller) { bIsKiller = IsKiller; }
	void SetName(FString InName) { Name = InName; }
	void SetLocation(FVector InLocation) { Location = InLocation; }
};


UCLASS()
class DEADBYDAYLIGHT_API APlayerManager : public AActor
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	void HandleMove(int32 PlayerId, FVector PlayerLocation, FRotator PlayerRotator);

	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnMovePlayer OnMovePlayer;

	UFUNCTION(BlueprintCallable, Category = "Network")
	void InsertPlayers(int32 PlayerId, AMyCharacterBase* Player);

private:
	TMap<int32, TUniquePtr<FPlayerInfo>> PlayerInfos;

	UPROPERTY()
	TMap<int32, AMyCharacterBase*> Players;
	int32 MyPlayerId;
};
