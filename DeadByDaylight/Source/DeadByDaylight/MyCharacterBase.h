// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyCharacterBase.generated.h"

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

public:
	UPROPERTY(BlueprintReadWrite, Category = "Network")
	FRotator NewTargetRotation;

	UPROPERTY(BlueprintReadWrite, Category = "Network")
	FVector NewTargetLocation;

};
