#pragma once
class PacketHandler
{
public:
	//패킷 처리 함수
	static void handlePacket(char* buffer, uint16_t id, uint16 packetSize);

private:
	static void handleLogin(Packet& loginPacket, uint16_t id);
	static void handleUnitSpawn(Packet& unitPacket, uint16_t id);
	static void handleUnitMove(Packet& unitPacket, uint16_t id);
};

