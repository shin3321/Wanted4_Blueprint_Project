#include "pch.h"
#include "Room.h"
#include "01.Game/Actor/Survivor.h"
#include "01.Game/Actor/Killer.h"
#include "01.Game/Actor/Session.h"
#include "01.Game/Actor/Player.h"
#include "02.Network/TimerEvent.h"

Room::Room(uint16 id)
{
	_roomId = id;

	slots[0]._loc = Location(50.0f, -60.0f, 0.0f);
	slots[1]._loc = Location(-20.0f, -30.0f, 0.0f);
	slots[2]._loc = Location(-120.0f, -80.0f, 0.0f);
	slots[3]._loc = Location(-230.0f, -140.0f, 0.0f);

}

bool Room::addKiller(std::shared_ptr<Session> session, int32 id)
{
	if (_killer) return false;

	std::shared_ptr<Killer> killer = std::make_shared<Killer>(id);
	_killer = killer;

	session->setPlayer(killer);
	session->setRoomId(_roomId);

	killer->setRoomId(_roomId);
	killer->setLocation(killerLocation);
	sendWaitingRoom(killer, id, true);
	return true;
}

bool Room::addSurvivor(std::shared_ptr<Session> session, int32 id)
{
	if (_survivors.size() > 4)
		return false;

	std::shared_ptr<Survivor> survivor = std::make_shared<Survivor>(id);
	{
		std::lock_guard<std::mutex>lock(_survivorLock);
		_survivors.emplace(id, survivor);
	}

	session->setPlayer(survivor);
	session->setRoomId(_roomId);

	survivor->setRoomId(_roomId);
	survivor->setLocation(WaitingPlayerLocation());
	sendWaitingRoom(survivor, id, false);
	return true;
}

Location Room::WaitingPlayerLocation()
{
	for (auto slot : slots)
	{
		if (slot._isOccupied == false)
		{
			slot._isOccupied = true;
			return slot._loc;
		}
	}
}

void Room::sendWaitingRoom(std::shared_ptr<Player> player, int32 playerId, bool isKiller)
{
	//원래 방에 있던 사람들에게 새로 들어온 사람 정보 보내기 + 나에게 나의 정보 보내기
	S_WaitingPacket waiting;
	waiting.Header.PacketType = EPacketType::S_Waiting;
	waiting.Header.PacketSize = sizeof(S_WaitingPacket);
	waiting.PlayerId = playerId;
	waiting.isKiller = isKiller;
	waiting.location = player->getLocation();

	if (!_survivors.empty())
	{
		std::lock_guard<std::mutex>lock(_survivorLock);
		for (auto& [id, survivor] : _survivors)
		{
			survivor->send(&waiting, sizeof(S_WaitingPacket));
		}
	}

	if (_killer)
	{
		_killer->send(&waiting, sizeof(S_WaitingPacket));
	}

	//새로 들어온 사람에게 원래 방에 있던 사람들 보내기
	if (!_survivors.empty())
	{
		std::lock_guard<std::mutex>lock(_survivorLock);
		for (auto& [id, survivor] : _survivors)
		{
			if (id == playerId) continue;
			S_WaitingPlayerInfoPacket info;
			info.Header.PacketType = EPacketType::S_WaitingPlayerInfo;
			info.Header.PacketSize = sizeof(S_WaitingPlayerInfoPacket);
			info.PlayerId = id;
			info.isKiller = false;
			info.location = survivor->getLocation();
			player->send(&info, sizeof(S_WaitingPlayerInfoPacket));
		}
	}

	if (_killer)
	{
		if (_killer->getId() != playerId)
		{
			S_WaitingPlayerInfoPacket info;
			info.Header.PacketType = EPacketType::S_WaitingPlayerInfo;
			info.Header.PacketSize = sizeof(S_WaitingPlayerInfoPacket);
			info.PlayerId = _killer->getId();
			info.isKiller = true;
			info.location = _killer->getLocation();
			player->send(&info, sizeof(S_WaitingPlayerInfoPacket));
		}
	}

	{
		Timer event{ playerId, std::chrono::system_clock::now() + 3s, TimerEvent::EV_GAME_WAIT, playerId, isKiller };
		std::lock_guard<std::mutex>lock(timer_mutex);
		timer_queue.push(event);
		timer_cv.notify_one();
	}

}

void Room::sendReadyPacket(C_ReadyPacket* readyPacket)
{
	++readyCount;
	int32 PlayerId = readyPacket->PlayerId;
	if (readyCount >= startCount)
	{
		Timer event{ PlayerId, std::chrono::system_clock::now() + 5s, TimerEvent::EV_GAME_START ,_roomId };
		std::lock_guard<std::mutex>lock(timer_mutex);
		timer_queue.push(event);
		timer_cv.notify_one();

	}
	S_ReadyPacket packet;
	packet.Header.PacketType = EPacketType::S_Ready;
	packet.Header.PacketSize = sizeof(S_ReadyPacket);
	packet.PlayerId = readyPacket->PlayerId;
	roomBroadcast(&packet, sizeof(packet));
}

void Room::sendGameStart()
{
	S_StartPacket packet;
	packet.Header.PacketType = EPacketType::S_GameStart;
	packet.Header.PacketSize = sizeof(S_StartPacket);
	roomBroadcast(&packet, sizeof(packet));
}

void Room::roomBroadcast(void* packet, uint16_t size)
{
	std::lock_guard<std::mutex>lock(_survivorLock);
	for (auto& [id, player] : _survivors)
	{
		player->send(packet, size);
	}
	if (_killer)
		_killer->send(packet, size);
}

void Room::sendMovePacket(const C_MovePacket& movePacket)
{
	S_MovePacket packet;
	packet.Header.PacketSize = sizeof(EPacketType::S_Move);
	packet.Header.PacketType = EPacketType::S_Move;
	packet.PlayerLocation = movePacket.PlayerLocation;
	packet.PlayerRotation = movePacket.PlayerRotation;
	packet.PlayerId = movePacket.PlayerId;
	roomBroadcast(&packet, sizeof(packet));
}
