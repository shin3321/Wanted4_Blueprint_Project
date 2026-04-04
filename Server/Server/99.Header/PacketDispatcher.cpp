#include "pch.h"
#include "PacketDispatcher.h"
#include "01.Game/Game.h"
#include "01.Game/Actor/Player.h"
#include "02.Network/DBThread.h"

void PacketDispatcher::onReceive(char* buffer, uint16 packetSize, int32 playerId)
{
	if (buffer == nullptr || packetSize < sizeof(PacketHeader))
		return;

	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

	EPacketType type = static_cast<EPacketType>(header->PacketType);

	auto it = _handlers.find(type);
	if (it != _handlers.end())
	{
		it->second(buffer, playerId);
	}
}

void PacketDispatcher::handleLogin(char* buffer, int32 playerId)
{
	std::cout << "Login Packet Handler\n";
	C_LoginPacket* packet = reinterpret_cast<C_LoginPacket*>(buffer);
	
	DBTask info = { DB_TASK_TYPE::Player_Load, playerId, packet->Id, packet->Pw };
	DBThread::get().pushTask(info);

}


void PacketDispatcher::handleWaiting(char* buffer, int32 playerId)
{
	std::cout << "Waiting Packet Handler\n";
	C_WaitingPacket* packet = reinterpret_cast<C_WaitingPacket*>(buffer);

	Game::get().assignRoom(packet->PlayerId, packet->IsKiller);
	//Game::get().sendWaitResult(packet->PlayerId, packet->IsKiller);
}

void PacketDispatcher::handleReady(char* buffer, int32 playerId)
{
	std::cout << "Ready Packet Handler\n";
	C_ReadyPacket* packet = reinterpret_cast<C_ReadyPacket*>(buffer);
	Game::get().readyRoom(packet);
}

void PacketDispatcher::handleMove(char* buffer, int32 playerId)
{
	C_MovePacket* packet = reinterpret_cast<C_MovePacket*>(buffer);
	Game::get().sendMove(packet);
}

void PacketDispatcher::handleChangeState(char* buffer, int32 playerId)
{
	auto header = reinterpret_cast<PacketHeader*>(buffer);

	int32 packetSize = header->PacketSize;

	if (packetSize < sizeof(C_ChangeStatePacket))
		return;

	C_ChangeStatePacket* packet = reinterpret_cast<C_ChangeStatePacket*>(buffer);

	if (packet->StateLen > packetSize)
		return;

	Game::get().recvState(packet);
}