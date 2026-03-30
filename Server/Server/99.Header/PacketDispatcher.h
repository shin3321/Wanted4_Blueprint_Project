#pragma once
#include <functional>

#define REGISTER_HANDLER(PacketType, Handler)\
_handlers.emplace(PacketType, [this](char* buffer){Handler(buffer);});

using PacketHandler = std::function<void(char*)>;

class PacketDispatcher
{
public:
	PacketDispatcher()
	{
		REGISTER_HANDLER(EPacketType::C_Login, handleLogin);
	}
	void onReceive(char* buffer, uint16 packetSize);

	static PacketDispatcher& get()
	{
		static PacketDispatcher instance;
		return instance;

	}
private:
	void handleLogin(char* buffer);

private:
	std::map<EPacketType, PacketHandler> _handlers;
};

