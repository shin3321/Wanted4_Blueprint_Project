// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerManager.h"
#include "Engine/GameInstance.h"
#include "MyNetworkSubsystem.h"
#include "MyGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoginResultReceived, int32, BloodPoint, int32, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWaitingRoom);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddWaitingRoom, const FPlayerInfo&, NewPlayerInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStart);


UCLASS()
class DEADBYDAYLIGHT_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void TryAsyncConnect(const FString& Ip, int32 Port);

	UFUNCTION(BlueprintCallable)
	bool ConnectToServer(const FString& Ip, int32 port);

	UFUNCTION(BlueprintCallable)
	bool IsServerConnected();

	//로그인 전송 패킷--------------------------------
	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendLoginPacket(FString UserId, FString UserPw, bool IsKiller);

	void SendPacket(void* Packet, int32 PacketSize);

	UPROPERTY(BlueprintAssignable, Category = "Error")
	FOnShowErrorMessage OnShowErrorMessage;

	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnLoginResultReceived OnLoginResultReceived;

	void HandleLogin(int32 BloodPoint, int32 PlayerId);

	//대기방 전송 패킷--------------------------------
	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendWaitingPacket(int32 PlayerId, bool IsKiller);
	void HandleWaitResult();

	void HandleWaiting(const FPlayerInfo& Info);
	bool AddPlayerInfo(int32 TargetId, const FPlayerInfo& Info);

	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnWaitingRoom OnWaitingRoom;

	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnAddWaitingRoom OnAddWaitingRoom;

	//Ready--------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendReadyPacket(int32 PlayerId, bool IsKiller);

	UPROPERTY(BlueprintAssignable, Category = "Ready")
	FOnReady OnReady;

	UPROPERTY(BlueprintAssignable, Category = "Ready")
	FOnStart OnStart;


	//Start --------------------------------
	void HandleGameStart(FVector StartLocation, int32 PlayerId);

	//PlayerInfo---------------------------------------
	UFUNCTION(BlueprintPure, Category = "GameInfo")
	int32 GetMyId() const { return MyPlayerId; }

	UFUNCTION(BlueprintPure, Category = "GameInfo")
	FVector GetMyPlayerLocation() const;

	UFUNCTION(BlueprintPure, Category = "GameInfo")
	bool GetMyIsKiller() const { return bIsKiller; }

	UFUNCTION(BlueprintPure, Category = "GameInfo")
	TArray<FPlayerInfo> GetAllInfo();

	UFUNCTION(BlueprintPure, Category = "GameInfo")
	int32 GetInfoSize() { return PlayerInfos.Num(); }

public:
	UPROPERTY(BlueprintReadWrite, Category = "PlayerInfo")
	int32 MyPlayerId;

	UPROPERTY(BlueprintReadWrite, Category = "PlayerInfo")
	int32 MyBloodPoint;

	UPROPERTY(BlueprintReadWrite, Category = "PlayerInfo")
	bool bIsKiller = false;

	UPROPERTY(BlueprintReadWrite, Category = "Game")
	bool bGameStarted = false;

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SetNetworkManagerOfPlayerManager(APlayerManager* PlayerManager);

private:
	//패킷을 받을 네트워크 매니저
	class UMyNetworkSubsystem* UNetworkManager;

	//소켓을 담을 변수
	FSocket* ClientSocket;

	//대기 방의 플레이어 저장 함수
	TMap<int32, TUniquePtr<FPlayerInfo>> PlayerInfos;
};
