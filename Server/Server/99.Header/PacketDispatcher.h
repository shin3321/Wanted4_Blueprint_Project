#pragma once
#include <functional>
#include "99.Header/Protocol.h"

#define REGISTER_HANDLER(PacketType, Handler)\
    _handlers.emplace(PacketType, [this](char* buffer, int32 playerId)\
		{ Handler(buffer, playerId); });

using PacketHandler = std::function<void(char*, uint16)>;

class PacketDispatcher
{
public:
	PacketDispatcher()
	{
		REGISTER_HANDLER(EPacketType::C_Login, handleLogin);
		REGISTER_HANDLER(EPacketType::C_Waiting, handleWaiting);
		REGISTER_HANDLER(EPacketType::C_Ready, handleReady);
		REGISTER_HANDLER(EPacketType::C_Move, handleMove);
		REGISTER_HANDLER(EPacketType::C_ChangeState, handleChangeState);
		REGISTER_HANDLER(EPacketType::C_ProjectileAxe, handleProjectile);
	}
	void onReceive(char* buffer, uint16 packetSize, int32 playerId);

	static PacketDispatcher& get()
	{
		static PacketDispatcher instance;
		return instance;

	}
private:
	void handleLogin(char* buffer, int32 playerId);
	void handleWaitingResult(char* buffer, int32 playerId);
	void handleWaiting(char* buffer, int32 playerId);
	void handleReady(char* buffer, int32 playerId);
	void handleMove(char* buffer, int32 playerId);
	void handleChangeState(char* buffer, int32 playerId);
	void handleProjectile(char* buffer, int32 playerId);
		
private:
	std::map<EPacketType, PacketHandler> _handlers;
};

