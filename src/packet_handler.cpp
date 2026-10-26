#include "packet_handler.h"
#include "cj_manager.h"
#include "defs.h"
#include "game_extensions.h"
#include "textdraw_rotation.h"
#include "utils.h"
#include "water_zones.h"

#include <RakHook/rakhook.hpp>
#include <RakNet/BitStream.h>
#include <RakNet/NetworkTypes.h>
#include <RakNet/PacketEnumerations.h>

#include <game_sa/CKeyGen.h>
#include <game_sa/CPed.h>

#include <format>
#include <string>
#include <vector>

namespace packet_handler {

static bool g_bHooked = false;
static bool g_bHandshakeConfirmed = false;
static uint32_t s_lastAuthRetryTick = 0;
static uint32_t s_handshakeRetryCount = 0;

// Reads an 8-bit length-prefixed ASCII string
static bool ReadPacketString(RakNet::BitStream& bs, std::string& outStr, size_t maxLen = 64)
{
	outStr.clear();
	if (bs.GetNumberOfUnreadBits() < 8) return false;

	uint8_t len = 0;
	bs.Read(len);
	if (len == 0) return true;
	if (len > maxLen) return false;

	if (BITS_TO_BYTES(bs.GetNumberOfUnreadBits()) < len) {
		return false;
	}

	char buf[256];
	if (len >= sizeof(buf)) return false;
	bs.Read(buf, len);
	buf[len] = '\0';
	outStr.assign(buf, len);
	return true;
}

bool OnReceivePacket(Packet* p)
{
	if (!p || !p->data || p->length < 1) {
		return true;
	}

	unsigned char packetId = p->data[0];
	size_t headerOffset = 1;

	// Handle RakNet timestamped packets
	if (packetId == ID_TIMESTAMP) {
		if (p->length < sizeof(unsigned char) + sizeof(RakNetTime) + 1) {
			return true;
		}
		packetId = p->data[sizeof(unsigned char) + sizeof(RakNetTime)];
		headerOffset = sizeof(unsigned char) + sizeof(RakNetTime) + 1;
	}

	if (packetId == ID_CONNECTION_REQUEST_ACCEPTED) {
		ClientLog(LogLevel::Info, "Received ID_CONNECTION_REQUEST_ACCEPTED, announcing plugin...");
		s_handshakeRetryCount = 0;
		s_lastAuthRetryTick = GetTickCount();
		SendHandshakePong(0);
		return true;
	}

	// Client disconnection from server: clear all cached clothing and gameplay states
	if (packetId == ID_DISCONNECTION_NOTIFICATION || packetId == ID_CONNECTION_LOST) {
		ClientLog(LogLevel::Info, "Disconnected from server, clearing states...");
		g_bHandshakeConfirmed = false;
		s_handshakeRetryCount = 0;
		cj_manager::ClearAllCachedStates();
		game_extensions::ClearAllStates();
		textdraw_rotation::ClearAllStates();
		water_zones::ClearAllStates();
		return true;
	}

	// Only process our custom Packet ID 252
	if (packetId != EXTENDEDGAME_PACKET_ID) {
		return true;
	}

	if (!g_bHandshakeConfirmed) {
		g_bHandshakeConfirmed = true;
		s_handshakeRetryCount = 0;
		ClientLog(LogLevel::Info, "Packet 252 received from server, handshake confirmed!");
	}

	// Construct BitStream over packet data
	RakNet::BitStream bs(p->data, p->length, false);
	bs.IgnoreBits(static_cast<int>(headerOffset * 8));

	// Minimum payload needed: playerId (2 bytes) + actionId (1 byte) = 3 bytes
	if (bs.GetNumberOfUnreadBits() < 24) {
		return false; // Consume malformed packet
	}

	uint16_t playerId = 0;
	uint8_t actionId = 0;
	if (!bs.Read(playerId) || !bs.Read(actionId)) {
		return false;
	}

	ClientLog(LogLevel::Debug, std::format("Received Packet 252: playerId={}, actionId={}", playerId, static_cast<int>(actionId)));

	// Resolve target ped (may be nullptr if player is not streamed in yet)
	CPed* pPed = GetGamePed(playerId);

	switch (actionId) {
	case CJ_ACTION_SET_CLOTHES: {
		uint8_t slot = 0;
		std::string texture;
		std::string model;
		uint8_t bRebuild = 1;

		if (!bs.Read(slot) || slot >= CJ_SLOT_COUNT) return false;
		if (!ReadPacketString(bs, texture)) return false;
		if (!ReadPacketString(bs, model)) return false;
		if (bs.GetNumberOfUnreadBits() >= 8) {
			bs.Read(bRebuild);
		}

		bool isRemove = (texture.empty() || texture == "none" || texture == "0");
		uint32_t texHash = isRemove ? 0 : CKeyGen::GetUppercaseKey(texture.c_str());
		uint32_t mdlHash = (isRemove || model.empty() || model == "none" || model == "0") ? 0 : CKeyGen::GetUppercaseKey(model.c_str());

		ClientLog(LogLevel::Debug, std::format("CJ_ACTION_SET_CLOTHES: playerId={}, slot={}, tex='{}', mdl='{}', rebuild={}, pPed={:p}",
			playerId, slot, texture, model, bRebuild, static_cast<void*>(pPed)));

		cj_manager::CacheClothes(playerId, slot, texHash, mdlHash);

		if (pPed) {
			cj_manager::SetClothes(pPed, slot, texture, model, bRebuild != 0);
		}
		break;
	}

	case CJ_ACTION_REMOVE_CLOTHES: {
		uint8_t slot = 0;
		uint8_t bRebuild = 1;

		if (!bs.Read(slot) || slot >= CJ_SLOT_COUNT) return false;
		if (bs.GetNumberOfUnreadBits() >= 8) {
			bs.Read(bRebuild);
		}

		cj_manager::CacheClothes(playerId, slot, 0, 0);

		if (pPed) {
			cj_manager::RemoveClothes(pPed, slot, bRebuild != 0);
		}
		break;
	}

	case CJ_ACTION_SET_BODY_STATS: {
		float fat = 0.0f;
		float muscle = 0.0f;
		uint8_t bRebuild = 1;

		if (!bs.Read(fat) || !bs.Read(muscle)) return false;
		if (bs.GetNumberOfUnreadBits() >= 8) {
			bs.Read(bRebuild);
		}

		cj_manager::CacheBodyStats(playerId, fat, muscle);

		if (pPed) {
			cj_manager::SetBodyStats(pPed, fat, muscle, bRebuild != 0);
		}
		break;
	}

	case CJ_ACTION_RESET_CLOTHES: {
		uint8_t toDefault = 1;
		uint8_t bRebuild = 1;

		if (bs.GetNumberOfUnreadBits() >= 8) {
			bs.Read(toDefault);
		}
		if (bs.GetNumberOfUnreadBits() >= 8) {
			bs.Read(bRebuild);
		}

		ClientLog(LogLevel::Debug, std::format("CJ_ACTION_RESET_CLOTHES: playerId={}, toDefault={}, rebuild={}, pPed={:p}",
			playerId, toDefault, bRebuild, static_cast<void*>(pPed)));

		cj_manager::CacheReset(playerId, toDefault != 0);

		if (pPed) {
			cj_manager::ResetClothes(pPed, toDefault != 0, bRebuild != 0);
		}
		break;
	}

	case CJ_ACTION_REBUILD: {
		if (pPed) {
			cj_manager::RebuildCJ(pPed);
		}
		break;
	}

	case CJ_ACTION_SET_FULL_OUTFIT: {
		float fat = 0.0f;
		float muscle = 0.0f;
		uint8_t itemCount = 0;

		if (!bs.Read(fat) || !bs.Read(muscle) || !bs.Read(itemCount)) return false;
		if (itemCount > CJ_SLOT_COUNT) return false;

		// Reset cached state to empty first so no lingering items from prior outfit remain
		cj_manager::CacheReset(playerId, false);
		cj_manager::CacheBodyStats(playerId, fat, muscle);

		std::vector<CJSkinItem> items;
		items.reserve(itemCount);

		for (uint8_t i = 0; i < itemCount; ++i) {
			CJSkinItem item;
			if (!bs.Read(item.slot) || item.slot >= CJ_SLOT_COUNT) return false;
			if (!ReadPacketString(bs, item.textureName)) return false;
			if (!ReadPacketString(bs, item.modelName)) return false;

			bool isRemove = (item.textureName.empty() || item.textureName == "none" || item.textureName == "0");
			item.textureHash = isRemove ? 0 : CKeyGen::GetUppercaseKey(item.textureName.c_str());
			item.modelHash = (isRemove || item.modelName.empty() || item.modelName == "none" || item.modelName == "0") ? 0 : CKeyGen::GetUppercaseKey(item.modelName.c_str());

			cj_manager::CacheClothes(playerId, item.slot, item.textureHash, item.modelHash);
			items.push_back(std::move(item));
		}

		if (pPed) {
			cj_manager::ResetClothes(pPed, false, false);
			for (const auto& item : items) {
				cj_manager::SetClothes(pPed, item.slot, item.textureName, item.modelName, false);
			}
			cj_manager::SetBodyStats(pPed, fat, muscle, false);
			cj_manager::RebuildCJ(pPed);
		}
		break;
	}

	case CJ_ACTION_SET_CLOTHES_HASH: {
		uint8_t slot = 0;
		uint32_t textureHash = 0;
		uint32_t modelHash = 0;
		uint8_t bRebuild = 1;

		if (!bs.Read(slot) || slot >= CJ_SLOT_COUNT) return false;
		if (!bs.Read(textureHash) || !bs.Read(modelHash)) return false;
		if (bs.GetNumberOfUnreadBits() >= 8) {
			bs.Read(bRebuild);
		}

		cj_manager::CacheClothes(playerId, slot, textureHash, modelHash);

		if (pPed) {
			cj_manager::SetClothesHash(pPed, slot, textureHash, modelHash, bRebuild != 0);
		}
		break;
	}

	case CJ_ACTION_SET_TATTOO: {
		uint8_t tattooSlot = 0;
		std::string texture;
		uint8_t bRebuild = 1;

		if (!bs.Read(tattooSlot)) return false;
		if (!ReadPacketString(bs, texture)) return false;
		if (bs.GetNumberOfUnreadBits() >= 8) {
			bs.Read(bRebuild);
		}

		// Slot mapping: 0..8 maps to 4..12, or absolute 4..12 accepted
		if (tattooSlot <= 8) {
			tattooSlot += CJ_SLOT_TATTOO_LOWER_LEFT;
		} else if (tattooSlot < CJ_SLOT_TATTOO_LOWER_LEFT || tattooSlot > CJ_SLOT_TATTOO_LOWER_BACK) {
			return false;
		}

		bool isRemove = (texture.empty() || texture == "none" || texture == "0");
		uint32_t texHash = isRemove ? 0 : CKeyGen::GetUppercaseKey(texture.c_str());
		cj_manager::CacheClothes(playerId, tattooSlot, texHash, 0);

		if (pPed) {
			cj_manager::SetClothes(pPed, tattooSlot, texture, "", bRebuild != 0);
		}
		break;
	}

	case CJ_ACTION_CLEAR_CACHE: {
		cj_manager::RemoveCachedState(playerId);
		break;
	}

	case CJ_ACTION_HANDSHAKE_PING: {
		uint32_t challenge = 0;
		if (bs.GetNumberOfUnreadBits() >= 32) {
			bs.Read(challenge);
		}
		SendHandshakePong(challenge);
		break;
	}

	case CJ_ACTION_HANDSHAKE_ACK: {
		g_bHandshakeConfirmed = true;
		s_handshakeRetryCount = 0;
		ClientLog(LogLevel::Info, "Handshake ACK received from server, communication channel established!");
		break;
	}

	case EXTENDEDGAME_ACTION_SET_AIRCRAFT_HEIGHT_LIMIT: {
		float limit = 800.0f;
		if (bs.Read(limit)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_SET_AIRCRAFT_HEIGHT_LIMIT: limit={:.1f}", limit));
			game_extensions::SetAircraftHeightLimit(limit);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_JETPACK_HEIGHT_LIMIT: {
		float limit = 100.0f;
		if (bs.Read(limit)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_SET_JETPACK_HEIGHT_LIMIT: limit={:.1f}", limit));
			game_extensions::SetJetpackHeightLimit(limit);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_BIKE_KNOCK_OFF: {
		uint8_t knockState = BIKE_KNOCK_OFF_DEFAULT;
		if (bs.Read(knockState)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_SET_BIKE_KNOCK_OFF: playerId={}, knockState={}", playerId, knockState));
			game_extensions::SetPlayerKnockedOffBikeState(playerId, knockState);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_FIRE_PROOF: {
		uint8_t enable = 0;
		if (bs.Read(enable)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_SET_FIRE_PROOF: playerId={}, enable={}", playerId, enable));
			game_extensions::SetPlayerFireProof(playerId, enable != 0);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_INFINITE_SPRINT: {
		uint8_t enable = 0;
		if (bs.Read(enable)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_SET_INFINITE_SPRINT: enable={}", enable));
			game_extensions::TogglePlayerInfiniteSprint(enable != 0);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_SUN: {
		uint8_t enable = 1;
		if (bs.Read(enable)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_SET_SUN: enable={}", enable));
			game_extensions::TogglePlayerSun(enable != 0);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_TOGGLE_MOON: {
		uint8_t enable = 1;
		if (bs.Read(enable)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_TOGGLE_MOON: enable={}", enable));
			game_extensions::TogglePlayerMoon(enable != 0);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_TOGGLE_STARS: {
		uint8_t enable = 1;
		if (bs.Read(enable)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_TOGGLE_STARS: enable={}", enable));
			game_extensions::TogglePlayerStars(enable != 0);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_TOGGLE_LOW_CLOUDS: {
		uint8_t enable = 1;
		if (bs.Read(enable)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_TOGGLE_LOW_CLOUDS: enable={}", enable));
			game_extensions::TogglePlayerLowClouds(enable != 0);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_TOGGLE_FLUFFY_CLOUDS: {
		uint8_t enable = 1;
		if (bs.Read(enable)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_TOGGLE_FLUFFY_CLOUDS: enable={}", enable));
			game_extensions::TogglePlayerFluffyClouds(enable != 0);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_TOGGLE_RAINBOW: {
		uint8_t enable = 1;
		if (bs.Read(enable)) {
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_TOGGLE_RAINBOW: enable={}", enable));
			game_extensions::TogglePlayerRainbow(enable != 0);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_TOGGLE_HUD_COMPONENTS: {
		uint32_t hudComponents = HUD_COMPONENT_ALL;
		if (bs.Read(hudComponents)) {
			game_extensions::TogglePlayerHUDComponents(hudComponents);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_HUD_COMPONENTS_COLOUR: {
		uint32_t hudComponents = 0;
		uint32_t intColour = 0;
		if (bs.Read(hudComponents) && bs.Read(intColour)) {
			game_extensions::SetPlayerHUDComponentsColour(hudComponents, intColour);
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_WATER_LEVEL: {
		float level = 0.0f;
		uint8_t waterParts = 0;
		if (bs.Read(level) && bs.Read(waterParts)) {
			if ((waterParts & ~0x07) != 0 ||
				!game_extensions::SetWaterLevel(
					level,
					(waterParts & 0x01) != 0,
					(waterParts & 0x02) != 0,
					(waterParts & 0x04) != 0
				)) {
				ClientLog(LogLevel::Warning, std::format("Rejected invalid water level: {:.3f}", level));
			}
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_WATER_WAVE_HEIGHT: {
		float height = 0.0f;
		if (bs.Read(height)) {
			if (!game_extensions::SetWaterWaveHeight(height)) {
				ClientLog(LogLevel::Warning, std::format("Rejected invalid water wave height: {:.3f}", height));
			}
		}
		break;
	}

	case EXTENDEDGAME_ACTION_RESET_WATER_LEVEL: {
		if (!game_extensions::ResetWaterLevel()) {
			ClientLog(LogLevel::Warning, "Could not reset water level before water data initialization");
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_TEXTDRAW_ROTATION: {
		uint16_t textDrawId = 0;
		uint8_t isPlayerTextDraw = 0;
		float angle = 0.0f;
		float pivotX = -1.0f;
		float pivotY = -1.0f;
		float spinSpeed = 0.0f;
		if (bs.Read(textDrawId) && bs.Read(isPlayerTextDraw) &&
			bs.Read(angle) && bs.Read(pivotX) && bs.Read(pivotY) && bs.Read(spinSpeed)) {
			uint16_t slot = isPlayerTextDraw ? static_cast<uint16_t>(2048 + textDrawId) : textDrawId;
			textdraw_rotation::Set(slot, angle, pivotX, pivotY, spinSpeed);
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_SET_TEXTDRAW_ROTATION: id={}, isPlayer={}, angle={:.1f}, pivot=({:.1f}, {:.1f}), spin={:.1f}",
				textDrawId, isPlayerTextDraw, angle, pivotX, pivotY, spinSpeed));
		}
		break;
	}

	case EXTENDEDGAME_ACTION_RESET_TEXTDRAW_ROTATION: {
		uint16_t textDrawId = 0;
		uint8_t isPlayerTextDraw = 0;
		if (bs.Read(textDrawId) && bs.Read(isPlayerTextDraw)) {
			uint16_t slot = isPlayerTextDraw ? static_cast<uint16_t>(2048 + textDrawId) : textDrawId;
			textdraw_rotation::Reset(slot);
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_RESET_TEXTDRAW_ROTATION: id={}, isPlayer={}",
				textDrawId, isPlayerTextDraw));
		}
		break;
	}

	case EXTENDEDGAME_ACTION_CREATE_WATER_ZONE: {
		uint16_t zoneId = 0;
		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;
		float level = 0.0f;
		float bottomZ = -100.0f;
		if (bs.Read(zoneId) && bs.Read(minX) && bs.Read(minY) &&
			bs.Read(maxX) && bs.Read(maxY) && bs.Read(level) && bs.Read(bottomZ)) {
			water_zones::Create(zoneId, minX, minY, maxX, maxY, level, bottomZ);
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_CREATE_WATER_ZONE: id={}, bounds=[{:.1f}, {:.1f} to {:.1f}, {:.1f}], level={:.2f}, bottom={:.2f}",
				zoneId, minX, minY, maxX, maxY, level, bottomZ));
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_WATER_ZONE_LEVEL: {
		uint16_t zoneId = 0;
		float level = 0.0f;
		if (bs.Read(zoneId) && bs.Read(level)) {
			water_zones::SetLevel(zoneId, level);
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_SET_WATER_ZONE_LEVEL: id={}, level={:.2f}",
				zoneId, level));
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_WATER_ZONE_COORDS: {
		uint16_t zoneId = 0;
		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;
		float level = 0.0f;
		float bottomZ = -100.0f;
		if (bs.Read(zoneId) && bs.Read(minX) && bs.Read(minY) &&
			bs.Read(maxX) && bs.Read(maxY) && bs.Read(level) && bs.Read(bottomZ)) {
			water_zones::SetCoords(zoneId, minX, minY, maxX, maxY, level, bottomZ);
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_SET_WATER_ZONE_COORDS: id={}, bounds=[{:.1f}, {:.1f} to {:.1f}, {:.1f}], level={:.2f}",
				zoneId, minX, minY, maxX, maxY, level));
		}
		break;
	}

	case EXTENDEDGAME_ACTION_DESTROY_WATER_ZONE: {
		uint16_t zoneId = 0;
		if (bs.Read(zoneId)) {
			water_zones::Destroy(zoneId);
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_DESTROY_WATER_ZONE: id={}", zoneId));
		}
		break;
	}

	case EXTENDEDGAME_ACTION_SET_JETPACK_OFFSET: {
		float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
		float rotAngle = 90.0f, rotAxisX = 0.0f, rotAxisY = 1.0f, rotAxisZ = 0.0f;
		if (bs.Read(posX) && bs.Read(posY) && bs.Read(posZ) &&
			bs.Read(rotAngle) && bs.Read(rotAxisX) && bs.Read(rotAxisY) && bs.Read(rotAxisZ)) {
			game_extensions::SetJetpackOffset(posX, posY, posZ);
			game_extensions::SetJetpackRotation(rotAngle, rotAxisX, rotAxisY, rotAxisZ);
			ClientLog(LogLevel::Info, std::format("EXTENDEDGAME_ACTION_SET_JETPACK_OFFSET: pos=({:.3f}, {:.3f}, {:.3f}), rot=({:.1f} deg, axis: {:.1f}, {:.1f}, {:.1f})",
				posX, posY, posZ, rotAngle, rotAxisX, rotAxisY, rotAxisZ));
		}
		break;
	}

	case EXTENDEDGAME_ACTION_RESET_JETPACK_OFFSET: {
		game_extensions::ResetJetpackOffset();
		ClientLog(LogLevel::Info, "EXTENDEDGAME_ACTION_RESET_JETPACK_OFFSET received");
		break;
	}

	default:
		break;
	}

	// Return false so RakNet/SA-MP deallocates packet and doesn't complain about unknown packet ID 252
	return false;
}

bool OnReceiveRpc(unsigned char& id, RakNet::BitStream* bs)
{
	if (!g_bHooked) return true;

	if (id == RPC_InitGame) {
		ClientLog(LogLevel::Info, "Received RPC_InitGame (139), session initializing; sending handshake pong...");
		g_bHandshakeConfirmed = false;
		s_handshakeRetryCount = 0;
		s_lastAuthRetryTick = GetTickCount();
		cj_manager::ClearAllCachedStates();
		game_extensions::ClearAllStates();
		textdraw_rotation::ClearAllStates();
		water_zones::ClearAllStates();
		game_extensions::EnsureJetpackLoaded();
		SendHandshakePong(0);
		return true;
	}

	if (id == RPC_GameModeRestart) {
		ClientLog(LogLevel::Info, "Received RPC_GameModeRestart (142), session resetting; sending handshake pong...");
		g_bHandshakeConfirmed = false;
		s_handshakeRetryCount = 0;
		s_lastAuthRetryTick = GetTickCount();
		cj_manager::ClearAllCachedStates();
		game_extensions::ClearAllStates();
		textdraw_rotation::ClearAllStates();
		water_zones::ClearAllStates();
		game_extensions::EnsureJetpackLoaded();
		SendHandshakePong(0);
		return true;
	}

	// When a remote player quits, clean up allocated player data and remove cached skin state
	if ((id == RPC_WorldPlayerRemove || id == 163) && bs) {
		int readOffset = bs->GetReadOffset();
		uint16_t playerId = 0;
		if (bs->Read(playerId)) {
			CPed* pPed = GetGamePed(playerId);
			if (pPed && !IsLocalPlayerPed(pPed) && pPed->m_pPlayerData) {
				if (pPed->m_pPlayerData->m_pPedClothesDesc) {
					delete pPed->m_pPlayerData->m_pPedClothesDesc;
					pPed->m_pPlayerData->m_pPedClothesDesc = nullptr;
				}
				delete pPed->m_pPlayerData;
				pPed->m_pPlayerData = nullptr;
			}
			cj_manager::RemoveCachedState(playerId);
		}
		bs->SetReadOffset(readOffset);
	}
	return true;
}

bool OnSendRpc(int& id, RakNet::BitStream* bs, PacketPriority& priority, PacketReliability& reliability, char& ord_channel, bool& sh_timestamp)
{
	if (!g_bHooked) return true;

	if (id == RPC_RequestClass || id == RPC_RequestSpawn) {
		if (!g_bHandshakeConfirmed && rakhook::orig && rakhook::orig->IsConnected()) {
			ClientLog(LogLevel::Info, std::format("RPC {} sent, announcing presence with Handshake Pong...", id));
			SendHandshakePong(0);
		}
	}
	return true;
}

void RequestPlayerSync(uint16_t targetPlayerId)
{
	if (!g_bHooked || !rakhook::initialized) return;

	RakNet::BitStream bs;
	uint8_t packetId = EXTENDEDGAME_PACKET_ID;
	uint8_t actionId = CJ_ACTION_REQUEST_SYNC;

	bs.Write(packetId);
	bs.Write(targetPlayerId);
	bs.Write(actionId);

	rakhook::send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0);
}

void SendHandshakePong(uint32_t challenge)
{
	if (!g_bHooked || !rakhook::initialized || !rakhook::orig) return;

	if (!rakhook::orig->IsConnected()) {
		ClientLog(LogLevel::Info, "SendHandshakePong skipped: rakclient is not connected to server yet");
		return;
	}

	RakNet::BitStream bs;
	uint8_t packetId = EXTENDEDGAME_PACKET_ID;
	uint16_t targetId = CJ_LOCAL_PLAYER_ID;
	uint8_t actionId = CJ_ACTION_HANDSHAKE_PONG;
	uint32_t magic = EXTENDEDGAME_HANDSHAKE_MAGIC;
	uint16_t version = EXTENDEDGAME_PLUGIN_VERSION;

	bs.Write(packetId);
	bs.Write(targetId);
	bs.Write(actionId);
	bs.Write(magic);
	bs.Write(version);
	bs.Write(challenge);

	bool bSuccess = rakhook::send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0);
	ClientLog(LogLevel::Info, std::format("SendHandshakePong: packet sent (challenge=0x{:X}, version=0x{:X}, success={})", challenge, version, bSuccess ? "true" : "false"));
}

void Process()
{
	if (!g_bHooked || !rakhook::initialized) return;

	if (!g_bHandshakeConfirmed && rakhook::orig && rakhook::orig->IsConnected()) {
		uint32_t now = GetTickCount();
		if (now - s_lastAuthRetryTick >= 2500) {
			s_lastAuthRetryTick = now;
			if (s_handshakeRetryCount < 6) {
				s_handshakeRetryCount++;
				ClientLog(LogLevel::Debug, std::format("Process(): Retrying Handshake Pong to server (attempt {}/6, awaiting confirmation)...", s_handshakeRetryCount));
				SendHandshakePong(0);
			}
		}
	}
}

bool Initialize()
{
	if (g_bHooked) return true;

	rakhook::on_receive_packet += OnReceivePacket;
	rakhook::on_receive_rpc += OnReceiveRpc;
	rakhook::on_send_rpc += OnSendRpc;
	g_bHooked = true;

	ClientLog(LogLevel::Info, "packet_handler::Initialize: hooks registered");

	// If connected to a server already, announce ASI presence immediately
	if (rakhook::orig && rakhook::orig->IsConnected()) {
		SendHandshakePong(0);
	}
	return true;
}

void Shutdown()
{
	g_bHooked = false;
	g_bHandshakeConfirmed = false;
	ClientLog(LogLevel::Info, "packet_handler::Shutdown: unhooked");
}

}
