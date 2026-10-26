#pragma once

#include <windows.h>
#include <cstdint>

struct Packet;
namespace RakNet {
    class BitStream;
}
enum PacketPriority;
enum PacketReliability;

namespace packet_handler {

bool Initialize();
void Shutdown();
void Process();

// Send sync request to server for a target player
void RequestPlayerSync(uint16_t targetPlayerId);

// Send handshake response to server proving plugin is installed
void SendHandshakePong(uint32_t challenge = 0);

// Callbacks for Rakhook
bool OnReceivePacket(Packet* p);
bool OnReceiveRpc(unsigned char& id, RakNet::BitStream* bs);
bool OnSendRpc(int& id, RakNet::BitStream* bs, PacketPriority& priority, PacketReliability& reliability, char& ord_channel, bool& sh_timestamp);

}
