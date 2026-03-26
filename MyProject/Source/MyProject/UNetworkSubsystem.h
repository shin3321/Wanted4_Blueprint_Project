// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <Sockets.h>
#include <Networking.h>
#include "UNetworkSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UUNetworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize() override;

	void SetSocket(FSocket* Socket);
};
