#include "pch.h"
#include "Game.h"
#include "01.Game/Actor/GameSession.h"
#include "99.Header/PacketDispatcher.h"
#include "01.Game/Actor/Session.h"
#include "01.Game/Actor/Player.h"
#include "01.Game/Room.h"
#include <cmath>
#include <fstream>

Game* Game::_instance = nullptr;

Game::Game()
	:_iocpHandle(nullptr), _overlappedPool(nullptr)
{
	_instance = this;
	makeRoom();
}

Game::~Game()
{
}

void Game::init(HANDLE iocpHandle, OverlappedExPool& overlappedPool)
{
	_iocpHandle = iocpHandle;
	_overlappedPool = &overlappedPool;
}

void Game::accept(SOCKET socket)
{
	int newId = getClientId();
	auto newSession = std::make_shared<Session>(*_overlappedPool);
	newSession->_socket = socket;
	newSession->_id = newId;
	sessions.try_emplace(newId, newSession);

	//세션에 넣은 소켓 io 등록
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(newSession->_socket), _iocpHandle, newSession->_id, 0);

	//recv 등록
	newSession->doRecv();
	std::cout << "new Session " << newSession->_id << "\n";
}

void Game::recv(int32 key, uint16_t numbytes)
{
	//받은 세션 아이디 확인
	auto session = getSession(key);
	if (!session)
	{
		std::cout << "Invalid Session In Recv. Id: " << key << "\n";
		return;
	}

	if (!session->_recvBuf.write(session->_recvOver._wsaBuf.buf, numbytes))
	{
		//Todo: Recv Error
	}

	while (true)
	{
		//recv 버퍼에 패킷 사이즈가 없으면 실패
		uint16_t packetSize = 0;
		if (!session->_recvBuf.peek(reinterpret_cast<char*>(&packetSize), sizeof(uint16_t)))
			break;

		if (packetSize == 0) break;

		//받은 리시브 버퍼의 크기가 패킷 사이즈보다 작으면 실패
		if (session->_recvBuf.size() < packetSize)
			break;

		//받은 패킷을 받을 벡터 만들기
		std::vector<char> packet(packetSize);

		if (!session->_recvBuf.read(packet.data(), packetSize))
		{
			std::cerr << " Ring Buffer Read Failed";
			break;
		}

		//프로세스 패킷
		PacketDispatcher::get().onReceive(packet.data(), packetSize, key);
	}
	session->doRecv();
}

int32 Game::getClientId()
{
	_clientId.fetch_add(1);
	return _clientId;
}

int32 Game::getRoomId()
{
	_roomId.fetch_add(1);
	return _roomId;
}

void Game::sendLoginResult(int32 playerId, int bloodPoint)
{
	S_LoginResultPacket result;
	result.Header.PacketType = EPacketType::S_LoginResult;
	result.Header.PacketSize = sizeof(S_LoginResultPacket);
	result.bloodPoint = bloodPoint;
	result.playerId = playerId;

	auto session = getSession(playerId);
	session->doSend(&result, sizeof(S_LoginResultPacket));
}

void Game::makeRoom()
{
	if (rooms.empty())
	{
		uint16 roomId = getRoomId();
		rooms.emplace(roomId, std::make_shared<Room>(roomId));
	}

}

void Game::sendWaitResult(int32 playerId, bool isKiller)
{
	auto session = getSession(playerId);
	S_WaitingResultPacket packet;
	packet.Header.PacketType = EPacketType::S_WaitingResult;
	packet.Header.PacketSize = sizeof(S_WaitingResultPacket);
	session->doSend(&packet, sizeof(S_WaitingPacket));
}

void Game::assignRoom(int32 playerId, bool isKiller)
{
	auto session = getSession(playerId);
	bool isAssign = false;
	for (auto& [id, room] : rooms)
	{
		if (room->_isStarted)
			continue;

		if (isKiller)
		{
			if (room->addKiller(session, playerId))
			{
				isAssign = true;
				break;
			}
		}
		else
		{
			if (room->addSurvivor(session, playerId))
			{
				isAssign = true;
				break;
			}
		}
	}
	
	if (!isAssign)
	{
		uint16 newRoomId = getRoomId();
		auto newRoom = std::make_shared<Room>(newRoomId);
		rooms.emplace(newRoomId, newRoom);

		if (isKiller)
		{
			newRoom->addKiller(session, playerId);
		}
		else
		{
			newRoom->addSurvivor(session, playerId);
		}
	}
}		

void Game::readyRoom(C_ReadyPacket* readyPacket)
{
	int32 playerId = readyPacket->PlayerId;
	auto session = getSession(playerId);

	auto room = getRoom(session->_roomId);
	room->sendReadyPacket(readyPacket);
}

void Game::sendMove(C_MovePacket* movePacket)
{
	int32 playerId = movePacket->PlayerId;
	auto session = getSession(playerId);

	auto room = getRoom(session->_roomId);
	room->sendMovePacket(*movePacket);
}

void Game::recvState(C_ChangeStatePacket* statePacket)
{
	int32 playerId = statePacket->PlayerId;
	auto session = getSession(playerId);

	auto room = getRoom(session->_roomId);
	room->sendState(*statePacket);
}


void Game::sendAxe(int id)
{
	int32 playerId = id;
	auto session = getSession(id);

	auto room = getRoom(session->_roomId);

}

void Game::closeSocket(int sessioneId)
{
	std::shared_ptr<Session> session = sessions[sessioneId];
	std::cout << "Invalid Session. Id: " << sessioneId << "\n";

	if (!session)
	{
		std::cout << "Invalid Session In Recv. Id: " << sessioneId << "\n";
		return;
	}
	closesocket(session->_socket);
}

void Game::broadcast(const char* data, uint16 packetSize)
{
	for (const auto session : sessions)
	{
		session.second->doSend(&data, packetSize);
	}
}

std::shared_ptr<Session> Game::getSession(int32 id)
{
	auto it = sessions.find(id);
	if (it == sessions.end())
	{
		return {};
	}
	std::shared_ptr<Session> session = it->second;
	if (!session)
		return {};
	return session;
}

std::shared_ptr<Player> Game::getPlayer(uint32 playerId)
{
	return std::shared_ptr<Player>();
}

std::shared_ptr<Room> Game::getRoom(int32 sessionId)
{
	auto it = rooms.find(sessionId);
	if (it != rooms.end())
	{
		std::shared_ptr<Room> room = it->second;
		if (room)
			return room;
	}
	return std::shared_ptr<Room>();
}

Game& Game::get()
{
	if (!_instance)
	{
		std::cout << "instance is null";
		__debugbreak();
	}
	return *_instance;
}