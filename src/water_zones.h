#pragma once

#include <cstdint>
#include <CVector.h>

namespace WaterZones {

constexpr uint16_t MAX_WATER_ZONES = 128;
constexpr uint16_t INVALID_WATER_ZONE_ID = 0xFFFF;

bool Initialize();
void Process();
void Shutdown();

bool Create(uint16_t zoneId, float minX, float minY, float maxX, float maxY, float level, float bottomZ = -100.0f);
bool SetLevel(uint16_t zoneId, float level);
bool SetCoords(uint16_t zoneId, float minX, float minY, float maxX, float maxY, float level, float bottomZ = -100.0f);
bool Destroy(uint16_t zoneId);
void ClearAllStates();

bool HasActiveZones();
bool GetWaterLevel(float x, float y, float z, float& outLevel);
bool TestLine(float startX, float startY, float startZ, float endX, float endY, float endZ, CVector* pCol);
void RenderAll();

}
