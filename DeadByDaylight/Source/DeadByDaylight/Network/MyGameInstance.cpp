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
#include "Kismet/GameplayStatics.h"

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

void UMyGameInstance::SendLoginPacket(FString UserId, FString UserPw, bool IsKiller)
{
	UE_LOG(LogTemp, Display, TEXT("Send Login Packet: %s, %s"), *UserId, *UserPw);
	bIsKiller = IsKiller;

	FC_LoginPacket LoginPacket;
	LoginPacket.Header.PacketType = EPacketType::C_Login;
	LoginPacket.Header.PacketSize = sizeof(FC_LoginPacket);

	FTCHARToUTF8 ConvertId(*UserId);
	FCStringAnsi::Strncpy(LoginPacket.UserId, (const char*)ConvertId.Get(), sizeof(LoginPacket.UserId) - 1);
	LoginPacket.UserId[sizeof(LoginPacket.UserId) - 1] = '\0';

	FTCHARToUTF8 ConvertPw(*UserPw);
	FCStringAnsi::Strncpy(LoginPacket.UserPw, (const char*)ConvertPw.Get(), sizeof(LoginPacket.UserPw) - 1);
	LoginPacket.UserPw[sizeof(LoginPacket.UserPw) - 1] = '\0';

	SendPacket(&LoginPacket, sizeof(FC_LoginPacket));
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

void UMyGameInstance::HandleLogin(int32 BloodPoint, int32 PlayerId)
{
	UE_LOG(LogTemp, Display, TEXT("HandleLogin"));
	MyPlayerId = PlayerId;
	if (BloodPoint == -1)
	{
		OnShowErrorMessage.Broadcast();
	}
	else
	{
		MyBloodPoint = BloodPoint;
		UE_LOG(LogTemp, Log, TEXT("Login Success! ID: %d, BP: %d"), PlayerId, BloodPoint);
		OnLoginResultReceived.Broadcast(BloodPoint, PlayerId);
		//UGameplayStatics::OpenLevel(GetWorld(), "ReadyRoom", true);
	}
}

void UMyGameInstance::SendWaitingPacket(int32 PlayerId, bool IsKiller)
{
	UE_LOG(LogTemp, Display, TEXT("Waiting Send"));

	FC_WaitingPacket WaitingPacket;
	WaitingPacket.Header.PacketType = EPacketType::C_Waiting;
	WaitingPacket.Header.PacketSize = sizeof(FC_WaitingPacket);
	WaitingPacket.PlayerId = PlayerId;
	WaitingPacket.IsKiller = IsKiller;

	SendPacket(&WaitingPacket, sizeof(FC_WaitingPacket));
}

void UMyGameInstance::HandleWaitResult()
{
	OnWaitingRoom.Broadcast();
}


//원래 방에 있던 사람들에게 새로 들어온 사람 정보 보내기 + 나의 정보 
void UMyGameInstance::HandleWaiting(const FPlayerInfo& Info)
{
	//나의 정보라면 저장만 해 두고
	PlayerInfos.Add(Info.PlayerId, MakeUnique<FPlayerInfo>(Info));

	//나의 정보가 아니라면 대기방에 추가
	if (MyPlayerId != Info.PlayerId)
	{
		OnAddWaitingRoom.Broadcast(Info);
	}
}

bool UMyGameInstance::AddPlayerInfo(int32 PlayerId, const FPlayerInfo& Info)
{
	if (TUniquePtr<FPlayerInfo>* TargetInfo = PlayerInfos.Find(PlayerId))
	{
		*(*TargetInfo) = Info;
		return false;
	}
	else
	{
		PlayerInfos.Add(PlayerId, MakeUnique<FPlayerInfo>(Info));
		return true;
	}
}

void UMyGameInstance::SendReadyPacket(int32 PlayerId, bool IsKiller)
{
	UE_LOG(LogTemp, Display, TEXT("Ready Send"));

	FC_ReadyPacket ReadyPacket;
	ReadyPacket.Header.PacketType = EPacketType::C_Ready;
	ReadyPacket.Header.PacketSize = sizeof(FC_ReadyPacket);
	ReadyPacket.PlayerId = PlayerId;

	SendPacket(&ReadyPacket, sizeof(FC_ReadyPacket));
}

void UMyGameInstance::HandleGameStart(FS_StartPacket* Packet)
{
	if (Packet == nullptr) return;

	// 5명의 정보를 모두 순회합니다.
	for (int32 i = 0; i < 5; ++i)
	{
		int32 TargetId = Packet->StartInfo[i].PlayerId;

		// 유효한 ID인지 체크
		if (TargetId <= 0)
		{
			continue;
		}

		FVector SpawnLocation(
			Packet->StartInfo[i].StartLocation.x,
			Packet->StartInfo[i].StartLocation.y,
			Packet->StartInfo[i].StartLocation.z
		);

		if (auto InfoPtr = PlayerInfos.Find(TargetId))
		{
			auto InfoRef = InfoPtr->Get();
			InfoRef->SetLocation(SpawnLocation);

			InfoRef->SetIsKiller(Packet->StartInfo[i].Iskiller);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerInfos에 존재하지 않는 ID(%d)가 시작 패킷에 포함되었습니다."), TargetId);
		}
	}

	// 모든 유저의 위치 정보 세팅이 끝났으므로 레벨을 이동합니다.
	UGameplayStatics::OpenLevel(GetWorld(), "GameLevel", true);
}

FVector UMyGameInstance::GetMyPlayerLocation() const
{
	auto Info = PlayerInfos.Find(MyPlayerId);
	auto InfoRef = Info->Get();
	return InfoRef->Location;
}

TArray<FPlayerInfo> UMyGameInstance::GetAllInfo()
{
	TArray<FPlayerInfo> OutInfos;

	for (auto& [Id, Info] : PlayerInfos)
	{
		OutInfos.Add(*Info);
	}
	return OutInfos;
}

void UMyGameInstance::SetNetworkManagerOfPlayerManager(APlayerManager* PlayerManager)
{
	 UNetworkManager->SetPlayerManager(PlayerManager);
}
