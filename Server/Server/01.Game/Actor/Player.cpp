#include "pch.h"
#include "Player.h"

Player::Player(std::string nickname)
{
	_nickname = nickname;
	_unitCount = 0;
}

bool Player::addUnit()
{
	if (_unitCount == 10)
		return false;

	_unitCount.fetch_add(1);
	return true;
}

void Player::subUnit()
{
	_unitCount.fetch_sub(1);
}

//void Player::addUnit(uint16 unitId, std::shared_ptr<Unit> unit)
//{
//	_units.try_emplace(unitId, unit);
//}
