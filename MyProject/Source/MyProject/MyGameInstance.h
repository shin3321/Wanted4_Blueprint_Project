// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

class UNetworkSubsystem;
/**
 *
 */
UCLASS()
class MYPROJECT_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void TryAsyncConnect(const FString& Ip, int32 Port);

	UFUNCTION(BlueprintCallable)
	bool ConnectToServer(const FString& Ip, int32 port);

private:
	//패킷을 받을 네트워크 매니저
	UNetworkSubsystem* UNetworkManager;

	//소켓을 담을 변수
	FSocket* ClientSocket;
};
