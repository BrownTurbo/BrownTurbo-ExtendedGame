#pragma once

#include <windows.h>
#include <cstdint>

#include <RakHook/rakhook.hpp>
#include <RakNet/BitStream.h>
#include <RakNet/NetworkTypes.h>
#include <RakNet/PacketEnumerations.h>

struct Packet;

namespace PacketHandler {

bool Initialize();
void Shutdown();
void Process();

// Send sync request to server for a target player
void RequestPlayerSync(uint16_t targetPlayerId);

// Send handshake response to server proving plugin is installed
void SendHandshakePong(uint32_t challenge = 0);

// Send pause menu event to server
bool SendPauseMenuEvent(uint8_t eventType, uint8_t fromPage, uint8_t toPage);

// Callbacks for Rakhook
bool OnReceivePacket(Packet* p);
bool OnReceiveRpc(unsigned char& id, RakNet::BitStream* bs);
bool OnSendRpc(int& id, RakNet::BitStream* bs, PacketPriority& priority, PacketReliability& reliability, char& ord_channel, bool& sh_timestamp);

}
