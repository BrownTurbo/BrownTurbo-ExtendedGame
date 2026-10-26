#pragma once

#include <cstdint>
#include <string>

namespace game_extensions {

// Lifecycle
bool Initialize();
void Process();
void Shutdown();

// Jetpack asset management
void EnsureJetpackLoaded();

// Gameplay controls
void SetAircraftHeightLimit(float limit);
float GetAircraftHeightLimit();

void SetJetpackHeightLimit(float limit);
float GetJetpackHeightLimit();

// Jetpack clump placement & orientation
void SetJetpackOffset(float x, float y, float z);
void GetJetpackOffset(float& x, float& y, float& z);

void SetJetpackRotation(float angle, float axisX, float axisY, float axisZ);
void GetJetpackRotation(float& angle, float& axisX, float& axisY, float& axisZ);

void ResetJetpackOffset();

void SetPlayerKnockedOffBikeState(uint16_t playerId, uint8_t knockState);
uint8_t GetPlayerKnockedOffBikeState(uint16_t playerId);

void SetPlayerFireProof(uint16_t playerId, bool enable);
bool IsPlayerFireProof(uint16_t playerId);

void TogglePlayerInfiniteSprint(bool enable);
bool IsInfiniteSprintEnabled();

// Environmental visual controls
void TogglePlayerSun(bool enable);
bool IsSunEnabled();

// Sky elements controls (UG:MP Parity)
void TogglePlayerMoon(bool enable);
bool IsMoonEnabled();

void TogglePlayerStars(bool enable);
bool IsStarsEnabled();

void TogglePlayerLowClouds(bool enable);
bool IsLowCloudsEnabled();

void TogglePlayerFluffyClouds(bool enable);
bool IsFluffyCloudsEnabled();

void TogglePlayerRainbow(bool enable);
bool IsRainbowEnabled();

// Water controls
bool SetWaterLevel(float level, bool includeWorldNonSeaLevel, bool includeWorldSeaLevel, bool includeOutsideWorldLevel);
bool ResetWaterLevel();
float GetWaterLevel();

bool SetWaterWaveHeight(float height);
float GetWaterWaveHeight();

// HUD Components controls (UG:MP Parity)
void TogglePlayerHUDComponents(uint32_t hudComponents);
uint32_t GetPlayerHUDComponents();

void SetPlayerHUDComponentsColour(uint32_t hudComponents, uint32_t intColour);
uint32_t GetPlayerHUDComponentColour(uint32_t hudComponent);

// Reset when client disconnects
void ClearAllStates();

}
