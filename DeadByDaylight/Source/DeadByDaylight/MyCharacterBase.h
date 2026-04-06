// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecvStateChange, const FString, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateAxe);

class UMyGameInstance;

UCLASS()
class DEADBYDAYLIGHT_API AMyCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void OnReceiveMovePacket(FVector NewLocation, FRotator NewRotation);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendState(FString NewState);
	
	void OnReceiveState(const FString NewState);

	UPROPERTY(BlueprintAssignable, Category = "State")
	FOnRecvStateChange OnRecvStateChange;

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendAxe();

	void OnReceiveAxe();
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnCreateAxe OnCreateAxe;

public:
	UPROPERTY(BlueprintReadWrite, Category = "Network")
	FRotator NewTargetRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Network")
	FVector NewTargetLocation;

	UFUNCTION(BlueprintPure, Category = "Network")
	int32 GetPlayerId() const { return PlayerId; }

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SetId(int32 Id) { PlayerId = Id; }

public:
	bool bIsGameStarted = false;

	UMyGameInstance* GameInst;

private:
	FString State;
	int32 PlayerId;
};
