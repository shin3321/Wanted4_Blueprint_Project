#include "pch.h"
#include "PacketHandler.h"
#include "01.Game/Game.h"
#include "01.Game/Actor/Player.h"
#include "Packet.h"


void PacketHandler::handlePacket(char* buffer, uint16_t id, uint16 packetSize)
{
	Packet packet;
	packet.getBuffer().assign(buffer, buffer + packetSize);

	uint16_t totalSize = packet.read<uint16_t>();   // 전체 크기
	uint16_t packetId = packet.read<uint16_t>();   // 패킷 타임

	switch (packetId)
	{
	case PK_CS_ID:
	{
		handleLogin(packet, id);
		break;
	}
	
	case PK_CS_SPAWN_UNIT:
	{
		handleUnitSpawn(packet, id);
		break;
	}
	case PK_CS_MOVE_UNIT:
	{
		handleUnitMove(packet, id);
		break;
	}

	}

}

void PacketHandler::handleLogin(Packet& loginPacket, uint16_t id)
{
	uint16_t offset = 0;
	//사이즈와 타입 더하기	
	std::string nickName(reinterpret_cast<char*>(&loginPacket), loginPacket.size());

	Game::get().setPlayer(std::make_shared<Player>(nickName), id);
	Game::get().waitingRoom(id);
}

void PacketHandler::handleUnitSpawn(Packet& unitPacket, uint16_t id)
{
	uint16_t playerId = unitPacket.read<uint16_t>();
	Vector2 spawnPos = unitPacket.read<Vector2>();

	Game::get().spawnUnit(playerId, spawnPos);
}

void PacketHandler::handleUnitMove(Packet& unitMovePacket, uint16_t id)
{
	uint16_t playerId = unitMovePacket.read<uint16_t>();
	Vector2 movePos = unitMovePacket.read<Vector2>();
	uint16_t unitCount = unitMovePacket.read<uint16_t>();

	for (int i = 0; i < unitCount; ++i)
	{
		uint16_t unitId = unitMovePacket.read<uint16_t>();

		Game::get().moveUnit(playerId, unitId, movePos);
	}
	Game::get().attackUnit();
}
