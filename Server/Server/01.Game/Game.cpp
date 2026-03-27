#include "pch.h"
#include "Game.h"
#include "01.Game/Actor/GameSession.h"
#include "99.Header/PacketHandler.h"
#include "01.Game/Actor/Session.h"
#include "01.Game/Actor/Player.h"

#include <cmath>
#include <fstream>

Game* Game::_instance = nullptr;

Game::Game()
	:_iocpHandle(nullptr), _overlappedPool(nullptr)
{
	_instance = this;
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

void Game::recv(uint16_t key, uint16_t numbytes)
{
	//받은 세션 아이디 확인
	std::shared_ptr<Session> session = sessions[key];
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
		packetSize = ntohs(packetSize);
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
		PacketHandler::handlePacket(packet.data(), key, packetSize);
	}
	session->doRecv();
}

uint16 Game::getClientId()
{
	_clientId.fetch_add(1);
	return _clientId;
}

uint16 Game::getUnittId()
{
	_unitId.fetch_add(1);
	return _unitId;
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
		session.second->doSend(data, packetSize);
	}
}

void Game::setPlayer(std::shared_ptr<Player> player, uint16_t playerId)
{
	players.try_emplace(playerId, player);
	player->setId(playerId);
}

std::shared_ptr<Session> Game::getSession(uint16_t id)
{
	std::shared_ptr<Session> session = sessions[id];
	return session;
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