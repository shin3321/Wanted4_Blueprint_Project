// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

/**
 *
 */
UCLASS()
class DEADBYDAYLIGHT_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyGameModeBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	// 컨트롤러를 생성할 때 호출되는 엔진 함수
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<APlayerController> KillerControllerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<APlayerController> SurvivorControllerClass;
};
