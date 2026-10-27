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

// Vision controls
void ToggleNightVision(bool enable);
bool IsNightVisionEnabled();

void ToggleThermalVision(bool enable);
bool IsThermalVisionEnabled();

// Weapon reload mechanics
void SetNoReload(bool enable);
bool IsNoReloadEnabled();

void ToggleSwitchReload(bool enable);
bool IsSwitchReloadEnabled();

void ToggleQuickReload(bool enable);
bool IsQuickReloadEnabled();

// Physics & movement controls
void ToggleInfiniteOxygen(bool enable);
bool IsInfiniteOxygenEnabled();

void SetPlayerPedAnims(uint16_t playerId, uint8_t mode);
uint8_t GetPlayerPedAnims(uint16_t playerId);

void TogglePlayerCursor(bool enable);
bool IsPlayerCursorEnabled();

// Plane radar altimeter color
void SetPlaneRadarColor(uint32_t color);
void SetPlaneRadarColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t GetPlaneRadarColor();
void GetPlaneRadarColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

// Sun color settings (hexadecimal RGBA)
void SetSunColor(uint32_t coreColor, uint32_t coronaColor);
void ResetSunColor();
bool IsSunColorSet();
uint32_t GetSunCoreColor();
uint32_t GetSunCoronaColor();

// Moon color settings (hexadecimal RGBA)
void SetMoonColor(uint32_t color);
void ResetMoonColor();
bool IsMoonColorSet();
uint32_t GetMoonColor();

// Vehicle speed blur motion toggle
void ToggleVehicleBlur(bool enable, uint8_t intensity = 36);
bool IsVehicleBlurEnabled();
uint8_t GetVehicleBlurIntensity();

// Pause menu tracking
bool IsLocalPlayerInPauseMenu();

// Reset when client disconnects
void ClearAllStates();

}
