#include "pch.h"
#include "PacketDispatcher.h"
#include "01.Game/Game.h"
#include "01.Game/Actor/Player.h"

void PacketDispatcher::onReceive(char* buffer, uint16 packetSize)
{
	EPacketType* type = reinterpret_cast<EPacketType*>(buffer);

	if (_handlers.contains(*type))
	{
		_handlers[*type](buffer);
	}
}

void PacketDispatcher::handleLogin(char* buffer)
{

}
