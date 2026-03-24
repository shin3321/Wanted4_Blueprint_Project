#include "pch.h"
#include "Game.h"
#include "01.Game/Actor/GameSession.h"
#include "99.Header/PacketHandler.h"
#include "01.Game/Actor/Session.h"
#include "01.Game/Actor/Player.h"
#include "01.Game/Actor/Castle.h"
#include "01.Game/Actor/Unit.h"

#include <cmath>
#include <fstream>

Game* Game::_instance = nullptr;

Game::Game()
	:_iocpHandle(nullptr), _overlappedPool(nullptr),
	_mapBoundary(_width / 2, _height / 2, _width / 2, _height / 2),
	_tree(std::make_unique<QuadTree>(Rect(_width / 2, _height / 2, _width / 2, _height / 2), 4))
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

void Game::waitingRoom(uint16 id)
{
	if (players.size() == 2)
	{
		// 2명이면 3초 뒤에 게임 시작 모두에게 보냄
		Timer event{ id, std::chrono::system_clock::now() + 2s, TimerEvent::EV_GAME_START, 0 };

		//타이버 이벤트를 넣기 위한 뮤텍스 처리
		std::lock_guard<std::mutex> lock(timer_mutex);
		timer_queue.push(event);
		timer_cv.notify_one();
	}

	Packet waitingRoom;
	waitingRoom.write<uint16_t>(0);
	waitingRoom.write<uint16_t>(PK_SC_WAITING);
	waitingRoom.write<uint16_t>(id);
	uint16_t totalSize = static_cast<uint16_t>(waitingRoom.getBuffer().size());
	uint16_t networkSize = htons(totalSize);
	memcpy(&waitingRoom.getBuffer()[0], &networkSize, sizeof(uint16_t));

	std::shared_ptr<Session> session = sessions[id];
	session->doSend(waitingRoom.getBuffer().data(), totalSize);
}

void Game::loadMap()
{
	std::ifstream file("C:\\Wanted4_game\\Project\\Server\\Asset\\map.txt");
	std::string line;
	std::string validMapData = "";
	int rowCount = 0;

	while (std::getline(file, line)) {
		// 공백 제거
		line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
		line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

		if (line.empty()) continue;

		// 파일의 한 줄이 설정된 _width보다 길거나 짧을 경우에 대한 예외 처리
		if (line.size() > (size_t)_width) line = line.substr(0, _width);

		validMapData += line;
		rowCount++;
		if (rowCount >= _height) break;
	}
	_tiles.assign(_width * _height, 0);
	// 비트 패킹 로직 (기존과 동일하되 validMapData 사용)
	std::vector<uint8_t> packedData((_width * _height + 7) / 8, 0);
	for (int i = 0; i < _width * _height; ++i) {
		if (i < validMapData.size() && validMapData[i] == '1') {
			_tiles[i] = 1;
			int byteIdx = i / 8;
			int bitIdx = i % 8;
			packedData[byteIdx] |= (1 << bitIdx);
		}
	}
	Packet mapPacket;

	mapPacket.write<uint16_t>(0);
	mapPacket.write<uint16_t>(PK_SC_GAME_START);
	mapPacket.write<uint16_t>(_width);
	mapPacket.write<uint16_t>(_height);
	// 이후 packedData를 mapPacket에 write
	mapPacket.writeBuffer(packedData.data(), packedData.size());

	uint16_t totalSize = static_cast<uint16_t>(mapPacket.getBuffer().size());
	uint16_t networkSize = htons(totalSize);
	memcpy(&mapPacket.getBuffer()[0], &networkSize, sizeof(uint16_t));
	broadcast(mapPacket.getBuffer().data(), totalSize);

	sendCastle();
}

void Game::sendCastle()
{
	uint16 playerId = 0;
	Vector2 castlePos;

	for (auto player : players)
	{
		playerId = player.second->getId();
		if (playerId == 5)
		{
			castlePos = castle1Pos;
			//_tree->insert({ castlePos.x, castlePos.y, castle });
		}
		else
		{
			castlePos = castle2Pos;
		}

		_tiles[castlePos.y * _width + castlePos.x] = playerId;

		Castle* castle = new Castle(castlePos, playerId);
		_castles.try_emplace(playerId, castle);
		Packet unitPacket;
		unitPacket.write<uint16>(0);
		unitPacket.write<uint16>(PK_SC_CASTLE);
		unitPacket.write<uint16>(playerId);
		unitPacket.write<Vector2>(castlePos);
		uint16 packetSize = unitPacket.size();
		uint16 networkSize = htons(packetSize);
		memcpy(&unitPacket.getBuffer().data()[0], &networkSize, sizeof(uint16_t));

		broadcast(unitPacket.getBuffer().data(), packetSize);
	}
}

void Game::setUnitPosTiles(Vector2 newPos, Vector2 oldPos, uint16 unitId)
{
	_tileLock.lock();
	_tiles[oldPos.y * _width + oldPos.x] = 0;
	_tiles[newPos.y * _width + newPos.x] = unitId;
	_tileLock.unlock();
}

bool Game::isSpawnableUnit(uint16 playerId, Vector2 spawnPos)
{
	int tileX = (int)spawnPos.x;
	int tileY = (int)spawnPos.y;
	if (tileX < 0 || tileX > _width || tileY < 0 || tileY > _height) return false;

	//빈 공간만 허용
	if (_tiles[tileY * _width + tileX] != 0) return false;

	// 플레이어 1 → 왼쪽 절반
	if (playerId == 5)
	{
		if (spawnPos.x < _width / 2)
			return true;
		else
			return false;
	}
	// 플레이어 2 → 오른쪽 절반
	if (playerId == 6) {
		if (spawnPos.x >= _width / 2)
			return true;
		else
			return false;
	}
}

bool Game::isMovableUnit(uint16 playerId, Vector2 movePos)
{
	int tileX = (int)movePos.x;
	int tileY = (int)movePos.y;

	if (tileX < 0 || tileX > _width || tileY < 0 || tileY > _height) return false;

	return true;
}

void Game::spawnUnit(uint16 playerId, Vector2 spawnPos)
{
	if (!isSpawnableUnit(playerId, spawnPos))
		return;
	uint16 unitId = getUnittId();
	auto unit = std::make_shared<Unit>(spawnPos, unitId, playerId);
	addUnit(unit);
	auto player = players[playerId];
	if (!player->addUnit())
		return;

	unit->setMap(_tiles);

	{
		std::lock_guard<std::mutex>lock(_tileLock);
		_tiles[spawnPos.y * _width + spawnPos.x] = unitId;
	}

	Packet unitPacket;
	unitPacket.write<uint16>(0);
	unitPacket.write<uint16>(PK_SC_SPAWN_UNIT);
	unitPacket.write<uint16>(unitId);
	unitPacket.write<uint16>(playerId);
	unitPacket.write<Vector2>(spawnPos);
	uint16 packetSize = unitPacket.size();
	uint16 networkSize = htons(packetSize);
	memcpy(&unitPacket.getBuffer().data()[0], &networkSize, sizeof(uint16_t));

	broadcast(unitPacket.getBuffer().data(), packetSize);
}

void Game::addUnit(std::shared_ptr<Unit> unit)
{
	{
		std::lock_guard<std::mutex>lock(_unitsLock);
		_units.try_emplace(unit->getId(), unit);

		_tree->insert({ unit->_pos.x, unit->_pos.y, unit.get() });
	}
}

void Game::removeUnit(uint16 unitId, uint16 playerId)
{
	Packet despawnUnit;
	despawnUnit.write<uint16>(0);
	despawnUnit.write<uint16>(PK_SC_DESPAWN_UNIT);
	despawnUnit.write<uint16>(unitId);
	uint16 packetSize = despawnUnit.size();
	uint16 networkSize = htons(packetSize);
	memcpy(despawnUnit.getBuffer().data(), &networkSize, sizeof(uint16_t));
	broadcast(despawnUnit.getBuffer().data(), packetSize);

	{
		std::lock_guard<std::mutex> lock(_unitsLock);
		auto unit = findUnit(unitId);
		if (unit) {
			_tiles[unit->_pos.y * _width + unit->_pos.x] = 0;
		}
		_units.erase(unitId);
	}

	{
		std::lock_guard<std::mutex> lock(_treeLock);
		rebuild();
	}
	auto player = players[playerId];
	player->subUnit();
}

void Game::moveUnit(uint16 playerId, uint16 unitId, Vector2 movePos)
{
	if (!isMovableUnit(unitId, movePos))
	{
		return;
	}

	auto unit = findUnit(unitId);
	if (unit)
	{
		std::vector<Vector2> path = unit->moveUnit(movePos);

		Packet movePacket;
		movePacket.write<uint16>(0);
		movePacket.write<uint16>(PK_SC_MOVE_UNIT);
		movePacket.write<uint16>(unitId);
		movePacket.write<uint16>(playerId);

		//경로 개수 먼저 전송
		movePacket.write<uint16>(static_cast<uint16>(path.size()));
		for (const Vector2& pos : path)
		{
			movePacket.write<Vector2>(pos);
		}
		uint16 packetSize = movePacket.size();
		uint16 networkSize = htons(packetSize);
		memcpy(movePacket.getBuffer().data(), &networkSize, sizeof(uint16_t));
		broadcast(movePacket.getBuffer().data(), packetSize);

		unit->_pos = movePos;

		{
			std::lock_guard<std::mutex>lock(_treeLock);
			rebuild();
		}

		_moveUnits.insert(unitId);
	}
}

std::shared_ptr<Unit> Game::findUnit(uint16 unitId)
{
	auto it = _units.find(unitId);
	if (it == _units.end())return{};
	auto unit = it->second;
	if (!unit)return{};
	if (!unit->_isAlive) return {};
	return unit;
}

void Game::rebuild()
{
	_tree = std::make_unique<QuadTree>(_mapBoundary, _capacity);

	for (auto& [id, unit] : _units)
	{
		if (unit->_isAlive)
		{
			_tree->insert({ unit->_pos.x, unit->_pos.y, unit.get() });
		}
	}
}

void Game::attackUnit()
{
	std::set<std::pair<int, int>> combatPairs;
	//auto units = player->getUnits();

	for (auto& [id, unit] : _units)
	{
		std::vector<Unit*> nearby;
		_tree->queryCircle(unit->_pos.x, unit->_pos.y, unit->_range, nearby);

		for (Unit* other : nearby)
		{
			//같은 팀이면 넘김
			if (other->_ownerId == unit->_ownerId) continue;
			if (!other->_isAlive || !unit->_isAlive) continue;
			int dx = other->_pos.x - unit->_pos.x;
			int dy = other->_pos.y - unit->_pos.y;

			double dist = (dx * dx) + (dy * dy);
			if (dist <= unit->_range * unit->_range)
			{
				int a = min(unit->_id, other->_id);
				int b = max(unit->_id, other->_id);
				combatPairs.insert({ a, b });
			}
		}
	}

	std::set<uint16> attackedUnits;
	for (auto& [idA, idB] : combatPairs)
	{
		auto unitA = findUnit(idA);
		auto unitB = findUnit(idB);

		if (!unitA || !unitB) continue;

		double dx = unitA->_pos.x - unitB->_pos.x;
		double dy = unitA->_pos.y - unitB->_pos.y;
		double distSq = dx * dx + dy * dy; // sqrt 없이 제곱으로 비교 (더 빠름)

		{
			std::lock_guard<std::mutex> lock(_attackeUnitsLock);
			// 각자 range 따로 체크
			if (distSq <= unitA->_range * unitA->_range)
			{
				unitB->takeDamage(unitA->_attack);
				attackedUnits.insert(unitB->getId());
			}

			if (distSq <= unitB->_range * unitB->_range)
			{
				unitA->takeDamage(unitB->_attack);
				attackedUnits.insert(unitA->getId());
			}

			if (!attackedUnits.empty())
			{
				attackedUnit(attackedUnits);
			}
		}

		// 타이머 이벤트 등록 (공격 쿨타임 관리)
		{
			std::lock_guard<std::mutex> lock(timer_mutex);
			Timer event{ 0, std::chrono::system_clock::now() + 3s, TimerEvent::EV_UNIT_ATTACK, 0 };
			timer_queue.push(event);
			timer_cv.notify_one();
		}
	}

	Timer event{ 0, std::chrono::system_clock::now() + 3s, TimerEvent::EV_UNIT_ATTACK, 0 };

	//타이버 이벤트를 넣기 위한 뮤텍스 처리
	std::lock_guard<std::mutex> lock(timer_mutex);
	timer_queue.push(event);
	timer_cv.notify_one();
}

void Game::attackedUnit(std::set<uint16> attackedUnits)
{
	Packet attackedUnit;
	attackedUnit.write<uint16>(0);
	attackedUnit.write<uint16>(PK_SC_ATTACK_UNIT);
	attackedUnit.write<uint16>(attackedUnits.size());
	for (uint16 id : attackedUnits)
	{
		attackedUnit.write<uint16>(id);
	}
	uint16 packetSize = attackedUnit.size();
	uint16 networkSize = htons(packetSize);
	memcpy(attackedUnit.getBuffer().data(), &networkSize, sizeof(uint16_t));
	broadcast(attackedUnit.getBuffer().data(), packetSize);
}

void Game::attackCastle()
{
	for (auto& [id, castle] : _castles)
	{
		std::vector<Unit*> nearby;
		_tree->queryCircle(castle->_pos.x, castle->_pos.y, 3, nearby);

		std::erase_if(nearby, [&](Unit* other) {
			return other->_ownerId == castle->_ownerId;
			});

		uint16 attackAmount = nearby.size() * castle->_attack;
		castle->attacked(attackAmount);

		Packet castleAttacked;
		castleAttacked.write<uint16>(0);
		castleAttacked.write<uint16>(PK_SC_ATTACK_CASTLE);
		castleAttacked.write<uint16>(id); //성 아이디
		uint16 packetSize = castleAttacked.size();
		uint16 totalSize = htons(packetSize);
		memcpy(&castleAttacked.getBuffer()[0], &totalSize, sizeof(uint16));
		broadcast(castleAttacked.getBuffer().data(), packetSize);
	}
}

void Game::removeCastle(uint16 castleId, uint16 playerId)
{
	Packet despawnUnit;
	despawnUnit.write<uint16>(0);
	despawnUnit.write<uint16>(PK_SC_DESTROY_CASTLE);
	despawnUnit.write<uint16>(castleId);
	uint16 packetSize = despawnUnit.size();
	uint16 networkSize = htons(packetSize);
	memcpy(despawnUnit.getBuffer().data(), &networkSize, sizeof(uint16_t));
	broadcast(despawnUnit.getBuffer().data(), packetSize);

	//	auto player = players[playerId];
		//게임 판정
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