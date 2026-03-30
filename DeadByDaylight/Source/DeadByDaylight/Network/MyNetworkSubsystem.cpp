// Fill out your copyright notice in the Description page of Project Settings.


#include "MyNetworkSubsystem.h"
#include "DeadByDaylight/Header/MyPacketStructs.h"

void UMyNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

void UMyNetworkSubsystem::Deinitialize()
{
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

		//현재 받은 데이터 크기와 버퍼 사이즈 중 더 작은 사이즈만큼 받음
		if (ClientSocket->Recv(TempData, FMath::Min(DataSize, (uint32)BUFSIZE), BytesRead))
		{
			{
				//받은 데이터를 받은 사이즈만큼 버퍼에 넣음
				ReceiverBuffer.Append(TempData, BytesRead);

				//받은 크기가 헤더 사이즈를 넘을 때까지
				while (ReceiverBuffer.Num() >= sizeof(FPacketHeader))
				{
					FPacketHeader* Header = reinterpret_cast<FPacketHeader*>(ReceiverBuffer.GetData());

					//받은 데이터가 패킷 사이즈보다 작으면 다시 Recv
					if (ReceiverBuffer.Num() < Header->PacketSize)
						break;

					//정돈된 패킷 하나를 큐에 넣기
					TArray<uint8> OnePacket;
					OnePacket.Append(ReceiverBuffer.GetData(), Header->PacketSize);

					//복사가 아닌 소유권 이전으로 복사 줄임
					ReceiveQueue.Enqueue(MoveTemp(OnePacket));

					//버퍼를 처음부터 해석한 패킥 사이즈만큼 지우기
					ReceiverBuffer.RemoveAt(0, Header->PacketSize, false);
				}
			}
		}
	}
}

void UMyNetworkSubsystem::ProcessQueuePackets()
{
}
