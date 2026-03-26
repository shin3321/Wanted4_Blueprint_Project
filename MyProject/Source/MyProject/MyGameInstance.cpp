// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "UNetworkSubsystem.h" // 파일명이 U로 시작하는지 확인하세요 (보통 NetworkSubsystem.h)
#include "Sockets.h"
#include "Networking.h"
#include "SocketSubsystem.h"

// Slate 에러 방지용 헤더
//#include "Math/Transform2D.h"
#include "Rendering/SlateLayoutTransform.h"
#include "Rendering/SlateRenderTransform.h"
#include "Widgets/SWidget.h"

void UMyGameInstance::Init()
{
	Super::Init();

#if UE_BUILD_DEVELOPMENT
	TryAsyncConnect("127.0.0.1", 9999);
#endif
}

//네트워크 비동기 연결 함수
void UMyGameInstance::TryAsyncConnect(const FString& Ip, int32 Port)
{
	TWeakObjectPtr<UMyGameInstance>WeakThis(this);
	if (!WeakThis.IsValid()) return;
	Async(EAsyncExecution::Thread, [WeakThis, Ip, Port]()
		{
			if (WeakThis->ConnectToServer(Ip, Port))
			{
				AsyncTask(ENamedThreads::GameThread, [WeakThis]()

					{if (!WeakThis.IsValid()) return;
						WeakThis->UNetworkManager = WeakThis->GetSubsystem<UNetworkSubsystem>();
						if (WeakThis->UNetworkManager)
						{
							//UNetworkManager->SetSocket();
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("NetWorkManager is null"));
						}
					});
			}
		});
}

bool UMyGameInstance::ConnectToServer(const FString& Ip, int32 Port)
{
	//소켓 시스템 받아오기
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	
	//소켓 생성
	ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("GameClientSocket"));
	if(!ClientSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Socket Creation Falied"));
		return false;
	}

	//주소를 IPAddress에 파싱
	FIPv4Address IPAddress;
	if (!FIPv4Address::Parse(Ip, IPAddress))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid IP address format: %s"), *Ip);
		return false;
	}
	
	//주소를 저장하는 컨테이너 생성 후 주소와 포트 저장
	TSharedRef<FInternetAddr>Addr = SocketSubsystem->CreateInternetAddr();
	Addr->SetIp(IPAddress.Value);
	Addr->SetPort(Port);

	//저장된 주소로 연결 시도
	if (!ClientSocket->Connect(*Addr))
	{
		int32 ErrorCode = SocketSubsystem->GetLastErrorCode();
		FString ErrorMessage = SocketSubsystem->GetSocketError(static_cast<ESocketErrors>(ErrorCode));
		UE_LOG(LogTemp, Error, TEXT("Failed to connect: %s (Code %d)"), *ErrorMessage, ErrorCode);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Connected to server: %s:%d"), *Ip, Port);
	return true;
}
