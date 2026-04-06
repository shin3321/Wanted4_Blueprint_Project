// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/TimerHandle.h"
#include "DeadByDaylight/Network/MyGameInstance.h"
#include "PlayerControllerBase.generated.h"

UCLASS(Blueprintable)
class DEADBYDAYLIGHT_API APlayerControllerBase : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayerReason) override;
	virtual void Tick(float DeltaTime) override;
protected:
	// 빙의될 때 호출되는 엔진 함수 오버라이드
	virtual void OnPossess(APawn* aPawn) override;

	UFUNCTION(BlueprintPure, Category = "Network")
	int32 GetPlayerId() const { return PlayerId; }

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SetId(int32 Id) { PlayerId = Id; }

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendMovePacket();

public:
	UMyGameInstance* GameInst;

protected:
	FTimerHandle SendPacketTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Network")
	float NetworkSendInterval = 0.01f;

private:
	APawn* OwnerPlayer;
	FString State;
	int32 PlayerId;

};
