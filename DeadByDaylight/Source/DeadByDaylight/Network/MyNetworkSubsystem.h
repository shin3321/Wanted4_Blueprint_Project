// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
//#include <Sockets.h>
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

	void SetSocket(FSocket* Socket) { ClientSocket = Socket; }

private:
	FSocket* ClientSocket;
};
