// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

//네트워킹 
#include <Networking.h>
#include <Sockets.h>
#include "Common/TcpSocketBuilder.h"
#include "Serialization/ArrayWriter.h"
#include "SocketSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "DeadByDaylight/Header/MyPacketStructs.h"

#include "MyNetworkSubsystem.h"



void UMyGameInstance::Init()
{
	Super::Init();

#if UE_BUILD_DEVELOPMENT
	TryAsyncConnect("127.0.0.1", 9999);
#endif

}

void UMyGameInstance::TryAsyncConnect(const FString& Ip, int32 Port)
{
	//게임인스턴스 함수를 weak_ptr로 받음
	TWeakObjectPtr<UMyGameInstance>WeakInstPtr(this);
	if (!WeakInstPtr.IsValid())return;
	Async(EAsyncExecution::Thread, [WeakInstPtr, Ip, Port]()
		{
			if (WeakInstPtr->ConnectToServer(Ip, Port))
			{
				AsyncTask(ENamedThreads::GameThread, [WeakInstPtr]()
					{
						if (!WeakInstPtr.IsValid()) return;
						//네트워크 매니저 설정
						WeakInstPtr->UNetworkManager = WeakInstPtr->GetSubsystem<UMyNetworkSubsystem>();
						if (WeakInstPtr->UNetworkManager)
						{
							//소켓 넘겨 주기
							WeakInstPtr->UNetworkManager->SetSocket(WeakInstPtr->ClientSocket);
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("NetWorkManager is null"));
						}
					});
			}
			else
			{
				AsyncTask(ENamedThreads::GameThread, []()
					{
					});
			}
		});
}

bool UMyGameInstance::ConnectToServer(const FString& Ip, int32 Port)
{
	if (ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		UE_LOG(LogTemp, Error, TEXT("Already Connect To Server"));
		return true;
	}

	//소켓 시스템 연결
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	ClientSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("GameClientSocket"));

	if (!ClientSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Socket Creation Failed"));
		return false;
	}

	//주소 컨테이너에 Ip 주소 연결
	FIPv4Address IpAddress;
	if (!FIPv4Address::Parse(Ip, IpAddress))
	{
		UE_LOG(LogTemp, Error, TEXT("Invaild Ip Address format: %s"), *Ip);
		return false;
	}
	TSharedRef<FInternetAddr>Addr = SocketSubsystem->CreateInternetAddr();
	Addr->SetIp(IpAddress.Value);
	Addr->SetPort(Port);

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

bool UMyGameInstance::IsServerConnected()
{
	if (ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		return true;
	}
	else
		return false;
}

void UMyGameInstance::SendLoginPacket(FString UserId, FString UserPw, bool IsKller)
{
	UE_LOG(LogTemp, Display, TEXT("Send Login Packet: %s, %s"), *UserId, *UserPw);

	FCLoginPacket LoginPacket;
	LoginPacket.Header.PacketType = EPacketType::C_Login;
	LoginPacket.Header.PacketSize = sizeof(FCLoginPacket);
	
	FTCHARToUTF8 ConvertId(*UserId);
	FCStringAnsi::Strncpy(LoginPacket.UserId, (const char*)ConvertId.Get(), sizeof(LoginPacket.UserId) - 1);
	LoginPacket.UserId[sizeof(LoginPacket.UserId) - 1] = '\0';

	FTCHARToUTF8 ConvertPw(*UserPw);
	FCStringAnsi::Strncpy(LoginPacket.UserPw, (const char*)ConvertPw.Get(), sizeof(LoginPacket.UserPw) - 1);
	LoginPacket.UserPw[sizeof(LoginPacket.UserPw) - 1] = '\0';

	SendPacket(&LoginPacket, sizeof(FCLoginPacket));
}



void UMyGameInstance::SendPacket(void* Packet, int32 PacketSize)
{
	int32 bytesSent = 0;

	if (!ClientSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Invaild Socket"));
		return;
	}
	if (!ClientSocket->Send(reinterpret_cast<uint8*>(Packet), PacketSize, bytesSent))
	{
		UE_LOG(LogTemp, Error, TEXT("Packet Send Fail"));
	}
	else
	{

	}
}
