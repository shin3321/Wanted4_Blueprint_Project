#include "pch.h"
#include "Player.h"
#include "Session.h"

Player::Player(int32 id)
{ 
	_id = id;
}


void Player::send(void* packet, uint16_t packetSize)
{
	if (auto s = _session.lock()) {
		s->doSend(packet, packetSize);
	}
}
