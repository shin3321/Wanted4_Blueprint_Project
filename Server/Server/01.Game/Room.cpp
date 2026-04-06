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

	slots[0]._loc = Location(50.0, -60.0, 0.0);
	slots[1]._loc = Location(-20.0, -30.0, 0.0);
	slots[2]._loc = Location(-120.0, -80.0, 0.0);
	slots[3]._loc = Location(-230.0, -140.0, 0.0);

	enterLocations[0].startLocation = Location(3560.0, 3170.0, 100.0);
	enterLocations[1].startLocation = Location(2130.0, -770.0, 100.0);
	enterLocations[2].startLocation = Location(2230.0, -2950.0, 100.0);
	enterLocations[3].startLocation = Location(-2430.0, -3170.0, 100.0);
	enterLocations[4].startLocation = Location(-2170.0, 3240.0, 100.0);
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
	std::lock_guard<std::mutex>lock(_slotLock);
	for (auto& slot : slots)
	{
		if (slot._isOccupied == false)
		{
			slot._isOccupied = true;
			return slot._loc;
		}
	}
	return {};
}

Location Room::startingPlayerLocation()
{
	std::lock_guard<std::mutex>lock(_enterLock);
	for (auto& loc : enterLocations)
	{
		if (loc._isOccupied == false)
		{
			loc._isOccupied = true;
			return loc.startLocation;
		}
	}
	return {};
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
	//Todo
	//들어오기 전에 미리 레디 한 사람들은 카운트 안 됨
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
	memset(&packet, 0, sizeof(S_StartPacket));
	packet.Header.PacketType = EPacketType::S_GameStart;
	packet.Header.PacketSize = sizeof(S_StartPacket);
	int infoIndex = 0; // StartInfo 배열에 접근하기 위한 별도의 인덱스

	{
		std::lock_guard<std::mutex> lock(_survivorLock);

		for (const auto& pair : _survivors)
		{
			if (infoIndex >= 5) break;

			std::shared_ptr<Survivor> survivor = pair.second;

			packet.StartInfo[infoIndex].PlayerId = survivor->getId();
			packet.StartInfo[infoIndex].StartLocation = startingPlayerLocation();
			packet.StartInfo[infoIndex].Iskiller = false;

			infoIndex++;
		}
	}

	if (_killer)
	{
		if (infoIndex < 5) // 배열 크기 체크
		{
			packet.StartInfo[infoIndex].PlayerId = _killer->getId();
			packet.StartInfo[infoIndex].StartLocation = startingPlayerLocation();
			packet.StartInfo[infoIndex].Iskiller = true;

			std::cout << "Killer Starting Location: " << packet.StartInfo[infoIndex].StartLocation << "\n";
			infoIndex++;
		}
	}
	roomBroadcast(&packet, sizeof(packet));
}


void Room::sendState(const C_ChangeStatePacket& statePacket)
{
	int32 Len = statePacket.StateLen;

	int32 PacketSize = sizeof(S_ChangeStatePacket) + Len;

	S_ChangeStatePacket* packet = (S_ChangeStatePacket*)malloc(PacketSize);

	packet->Header.PacketType = EPacketType::S_ChangeState;
	packet->Header.PacketSize = PacketSize;
	packet->PlayerId = statePacket.PlayerId;
	packet->StateLen = Len;

	memcpy(packet->State, statePacket.State, Len);

	roomBroadcastExcludeMe(statePacket.PlayerId, packet, PacketSize);

	free(packet);
}

void Room::sendAxe(int32 playerId)
{
	S_ProjectileAxe packet;
	packet.Header.PacketType = EPacketType::S_ProjectileAxe;
	packet.Header.PacketSize = sizeof(S_ProjectileAxe);
	packet.AxeId = playerId;

	roomBroadcast(&packet, sizeof(S_ProjectileAxe));
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

void Room::roomBroadcastExcludeMe(int32 playerId, void* packet, uint16_t size)
{
	std::lock_guard<std::mutex>lock(_survivorLock);
	for (auto& [id, player] : _survivors)
	{
		if (playerId == id) continue;
		player->send(packet, size);
	}
	if (_killer && (_killer->getId() != playerId))
		_killer->send(packet, size);
}

void Room::sendMovePacket(const C_MovePacket& movePacket)
{
	S_MovePacket packet;
	packet.Header.PacketSize = sizeof(S_MovePacket);
	packet.Header.PacketType = EPacketType::S_Move;
	packet.PlayerLocation = movePacket.PlayerLocation;
	packet.PlayerRotation = movePacket.PlayerRotation;
	packet.PlayerId = movePacket.PlayerId;
	roomBroadcast(&packet, sizeof(packet));
}
