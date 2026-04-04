// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNetworkSubsystem.h"
#include <Kismet/GameplayStatics.h>
#include "DeadByDaylight/Header/MyPacketStructs.h"
#include "DeadByDaylight/Network/MyGameInstance.h"
#include "DeadByDaylight/Network/PlayerManager.h"
#include "SocketSubsystem.h"
#include "EngineUtils.h"

void UMyNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UMyNetworkSubsystem::OnWorldInitialized);
}

void UMyNetworkSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMyNetworkSubsystem::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues)
{
	// 월드가 유효하고, 게임 월드일 때만 버퍼 리셋
	if (World && World->IsGameWorld())
	{
		ResetBuffer();
		UE_LOG(LogTemp, Warning, TEXT("Level Changed: Network Buffer Flushed."));
	}
}

void UMyNetworkSubsystem::ResetBuffer()
{
	ReceiverBuffer.Empty();
	UE_LOG(LogTemp, Warning, TEXT("Network Buffer Cleared for New Level"));
}

void UMyNetworkSubsystem::ReceiveData()
{
	if (!ClientSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Socket Is NULL"));
	}

	//소켓을 논블로킹 모드로 설정
	ClientSocket->SetNonBlocking(true);

	//받을 데이터 사이즈
	uint32 DataSize;

	//네트워크 버퍼에 데이터가 있는지 확인 -> HasPendingData true -> 몇 바이트 있느지 반환, 없으면 false 반환
	while (ClientSocket->HasPendingData(DataSize))
	{
		uint8 TempData[BUFSIZE];
		int32 BytesRead = 0;

		if (ReceiverBuffer.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Current Buffer Size: %d, First 2 Bytes: %02x %02x"),
				ReceiverBuffer.Num(), ReceiverBuffer[0], ReceiverBuffer[1]);
		}

		//현재 받은 데이터 크기와 버퍼 사이즈 중 더 작은 사이즈만큼 받음
		if (ClientSocket->Recv(TempData, FMath::Min(DataSize, (uint32)BUFSIZE), BytesRead))
		{
			{
				//받은 데이터를 받은 사이즈만큼 버퍼에 넣음
				ReceiverBuffer.Append(TempData, BytesRead);
				int32 ProcessedOffset = 0;

				//받은 크기가 헤더 사이즈를 넘을 때까지
				while (ReceiverBuffer.Num() - ProcessedOffset >= sizeof(FPacketHeader))
				{
					// 이 값이 서버/클라 모두 정확히 '4'가 나오는지 확인하세요.
					UE_LOG(LogTemp, Warning, TEXT("Header Size: %d"), sizeof(FPacketHeader));

					uint8* HeaderPtr = ReceiverBuffer.GetData() + ProcessedOffset;
					FPacketHeader Header;
					FMemory::Memcpy(&Header, HeaderPtr, sizeof(FPacketHeader));

					if (Header.PacketSize > MAX_PACKET_SIZE || Header.PacketSize <= 0)
					{
						UE_LOG(LogTemp, Error, TEXT("Invalid Packet Size: %d"), Header.PacketSize);
						return;

					}

					//받은 데이터가 패킷 사이즈보다 작으면 다시 Recv
					if (ReceiverBuffer.Num() - ProcessedOffset < Header.PacketSize)
						break;

					//정돈된 패킷 하나를 큐에 넣기
					TArray<uint8> OnePacket;
					OnePacket.Append(HeaderPtr, Header.PacketSize);
					ReceiveQueue.Enqueue(MoveTemp(OnePacket));

					ProcessedOffset += Header.PacketSize;
				}

				//버퍼를 처음부터 해석한 패킥 사이즈만큼 지우기
				if (ProcessedOffset > 0)
				{
					ReceiverBuffer.RemoveAt(0, ProcessedOffset, false);
				}
			}
		}
	}
}

void UMyNetworkSubsystem::ProcessQueuePackets()
{
	ReceiveData();
	GameInst = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());

	if (!GameInst) return;

	TArray<uint8> PacketData;

	while (true)
	{
		{
			FScopeLock Lock(&QueueLock);
			//패킷 데이터 큐 데이터 넣기
			if (!ReceiveQueue.Dequeue(PacketData))
				break;
		}
		if (PacketData.Num() < sizeof(FPacketHeader))
			continue;

		FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(PacketData.GetData());

		switch (Header->PacketType)
		{
		case EPacketType::S_LoginResult:
		{
			if (PacketData.Num() == sizeof(FS_LoginResultPacket))
			{
				UE_LOG(LogTemp, Display, TEXT("S_LoginResult"));
				FS_LoginResultPacket* Packet = reinterpret_cast<FS_LoginResultPacket*>(PacketData.GetData());
				GameInst->HandleLogin(Packet->BooldPoint, Packet->PlayerId);
			}
			break;
		}

		case EPacketType::S_Waiting:
		{
			if (PacketData.Num() == sizeof(FS_WaitingPacket))
			{
				UE_LOG(LogTemp, Display, TEXT("S_WaitingResult"));
				FS_WaitingPacket* Packet = reinterpret_cast<FS_WaitingPacket*>(PacketData.GetData());

				FPlayerInfo NewInfo;
				NewInfo.SetId(Packet->PlayerId);
				NewInfo.SetIsKiller(Packet->IsKiller);
				FVector Location(Packet->PlayerLocation.x, Packet->PlayerLocation.y, Packet->PlayerLocation.z);
				NewInfo.SetLocation(Location);

				GameInst->HandleWaiting(NewInfo);
			}
			break;
		}

		case EPacketType::S_WaitingResult:
		{
			if (PacketData.Num() == sizeof(FS_WaitingResultPacket))
			{
				UE_LOG(LogTemp, Display, TEXT("S_WaitingResult"));
				FS_WaitingResultPacket* Packet = reinterpret_cast<FS_WaitingResultPacket*>(PacketData.GetData());

				GameInst->HandleWaitResult();
			}
			break;
		}

		case EPacketType::S_WaitingPlayerInfo:
		{
			if (PacketData.Num() == sizeof(FS_WaitingPlayerInfoPacket))
			{
				UE_LOG(LogTemp, Display, TEXT("FS_WaitingPlayerInfoPacket"));
				FS_WaitingPlayerInfoPacket* Packet = reinterpret_cast<FS_WaitingPlayerInfoPacket*>(PacketData.GetData());

				FPlayerInfo Info;
				int32 PlayerId = Packet->PlayerId;
				Info.SetId(PlayerId);
				Info.SetIsKiller(Packet->IsKiller);
				FVector Location(Packet->Location.x, Packet->Location.y, Packet->Location.z);
				Info.SetLocation(Location);

				GameInst->AddPlayerInfo(PlayerId, Info);
			}
			break;
		}

		case EPacketType::S_Ready:
		{
			if (PacketData.Num() == sizeof(FS_ReadyPacket))
			{
				UE_LOG(LogTemp, Display, TEXT("FS_ReadyPacket"));
				FS_ReadyPacket* Packet = reinterpret_cast<FS_ReadyPacket*>(PacketData.GetData());

				FPlayerInfo Info;
				int32 PlayerId = Packet->PlayerId;
				Info.SetId(PlayerId);

				GameInst->OnReady.Broadcast();
			}
			break;
		}

		case EPacketType::S_GameStart:
		{
			if (PacketData.Num() == sizeof(FS_StartPacket))
			{
				UE_LOG(LogTemp, Display, TEXT("FS_StartPacket"));
				FS_StartPacket* Packet = reinterpret_cast<FS_StartPacket*>(PacketData.GetData());
				FVector StartLocation(Packet->StartLocation.x, Packet->StartLocation.y, Packet->StartLocation.z);

				GameInst->HandleGameStart(StartLocation, Packet->PlayerId);
			}
			break;
		}

		case EPacketType::S_Move:
		{
			if (PacketData.Num() == sizeof(FS_MovePacket))
			{
				UE_LOG(LogTemp, Display, TEXT("FS_MovePacket"));
				FS_MovePacket* Packet = reinterpret_cast<FS_MovePacket*>(PacketData.GetData());

				int32 PlayerId = Packet->PlayerId;
				FVector Location(Packet->PlayerLocation.x, Packet->PlayerLocation.y, Packet->PlayerLocation.z);
				FRotator Rotation(Packet->PlayerRotation.pitch, Packet->PlayerRotation.yaw, Packet->PlayerRotation.roll);
				if (PlayerMng)
					PlayerMng->HandleMove(PlayerId, Location, Rotation);
				else
				{
					UE_LOG(LogTemp, Error, TEXT("PlayerMng Is Null"));

				}
			}
			break;
		}

		case EPacketType::S_ChangeState:
		{
			UE_LOG(LogTemp, Display, TEXT("FS_ChangeStatePacket"));

			FS_ChangeStatePacket* Packet = reinterpret_cast<FS_ChangeStatePacket*>(PacketData.GetData());

			int32 PlayerId = Packet->PlayerId;
			uint16 Len = Packet->StateLen;

			if (PacketData.Num() < sizeof(FS_ChangeStatePacket) + Len)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid packet size"));
				break;
			}
			FString NewState = FString(
				Len,
				UTF8_TO_TCHAR(reinterpret_cast<const char*>(Packet->State))
			);
			if (PlayerMng)
				PlayerMng->HandleChangeState(PlayerId, NewState);

			break;
		}

		}
	}
}
