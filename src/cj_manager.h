#pragma once

#include "types.h"
#include <cstdint>
#include <string>
#include <unordered_map>

class CPed;
class CPlayerPed;
class CPedClothesDesc;

namespace CJManager {

// Initialize CJ system & hooks
bool Initialize();
void Shutdown();

// Ensure ped has valid CPlayerData and CPedClothesDesc
void EnsureClothesSystemInitialized();
bool EnsurePedIsCJ(CPed* pPed);

// Clothes & stats manipulation
bool SetClothes(CPed* pPed, uint8_t slot, const std::string& texture, const std::string& model, bool bRebuild);
bool SetClothesHash(CPed* pPed, uint8_t slot, uint32_t textureHash, uint32_t modelHash, bool bRebuild);
bool RemoveClothes(CPed* pPed, uint8_t slot, bool bRebuild);
bool SetBodyStats(CPed* pPed, float fFat, float fMuscle, bool bRebuild);
bool ResetClothes(CPed* pPed, bool toDefault, bool bRebuild);
bool RebuildCJ(CPed* pPed);

// State caching for persistence across streaming/respawns
void CacheClothes(uint16_t playerId, uint8_t slot, uint32_t textureHash, uint32_t modelHash);
void CacheBodyStats(uint16_t playerId, float fat, float muscle);
void CacheReset(uint16_t playerId, bool toDefault);
bool HasCachedState(uint16_t playerId);
void ApplyCachedState(uint16_t playerId, CPed* pPed);
void RemoveCachedState(uint16_t playerId);
void ClearAllCachedStates();

// Per-frame stream-in & sync check for cached CJ peds
void Process();

}
