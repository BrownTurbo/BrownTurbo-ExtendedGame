#include "water_zones.h"

#include "utils.h"

#include <MinHook.h>
#include <plugin.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <algorithm>

namespace WaterZones {
namespace {

struct WaterZone {
    bool enabled = false;
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float level = 0.0f;
    float bottomZ = -100.0f;
};

struct WaterRenPar {
    float z = 0.0f;
    float bigWaves = 0.2f;
    float smallWaves = 0.1f;
    char flowX = 0;
    char flowY = 0;
    char pad[2] = {0, 0};
};
static_assert(sizeof(WaterRenPar) == 0x10, "WaterRenPar must be 16 bytes");

using RenderWaterRectangle_t = void(__cdecl*)(
    int32_t minX, int32_t maxX, int32_t Y1, int32_t Y2,
    WaterRenPar P1, WaterRenPar P2, WaterRenPar P3, WaterRenPar P4
);

using GetWaterLevel_t = bool(__cdecl*)(float x, float y, float z, float* pOutWaterLevel, uint8_t bTouchingWater, CVector* pVecNormals);
using GetWaterLevelNoWaves_t = bool(__cdecl*)(float x, float y, float z, float* pOutWaterLevel, float* pSmallWaves, float* pBigWaves);
using TestLineAgainstWater_t = bool(__cdecl*)(float startX, float startY, float startZ, float endX, float endY, float endZ, CVector* pCol);
using CanSeeWaterFromCurrArea_t = bool(__cdecl*)();

std::array<WaterZone, MAX_WATER_ZONES> g_Zones{};
std::mutex g_Mutex;

GetWaterLevel_t g_origGetWaterLevel = nullptr;
GetWaterLevelNoWaves_t g_origGetWaterLevelNoWaves = nullptr;
TestLineAgainstWater_t g_origTestLineAgainstWater = nullptr;
CanSeeWaterFromCurrArea_t g_origCanSeeWaterFromCurrArea = nullptr;

static uint8_t s_origFlushCheckBytes[6] = { 0x39, 0x1D, 0x50, 0xB9, 0xC4, 0x00 };
static bool s_flushHookInstalled = false;

bool g_Initialized = false;
bool g_HooksInstalled = false;

void __declspec(naked) Hook_RenderWater_Zones() {
    __asm {
        pushad
        call WaterZones::RenderAll
        popad
        // Replicate original instruction: cmp dword ptr [0xC4B950], ebx (where ebx is 0)
        cmp dword ptr ds:[0xC4B950], 0
        ret
    }
}

bool __cdecl Hooked_GetWaterLevel(float x, float y, float z, float* pOutWaterLevel, uint8_t bTouchingWater, CVector* pVecNormals) {
    float customLevel = 0.0f;
    if (GetWaterLevel(x, y, z, customLevel)) {
        if (pOutWaterLevel) {
            *pOutWaterLevel = customLevel;
        }
        if (pVecNormals) {
            pVecNormals->x = 0.0f;
            pVecNormals->y = 0.0f;
            pVecNormals->z = 1.0f;
        }
        return true;
    }

    if (g_origGetWaterLevel) {
        return g_origGetWaterLevel(x, y, z, pOutWaterLevel, bTouchingWater, pVecNormals);
    }
    return false;
}

bool __cdecl Hooked_GetWaterLevelNoWaves(float x, float y, float z, float* pOutWaterLevel, float* pSmallWaves, float* pBigWaves) {
    float customLevel = 0.0f;
    if (GetWaterLevel(x, y, z, customLevel)) {
        if (pOutWaterLevel) {
            *pOutWaterLevel = customLevel;
        }
        if (pSmallWaves) {
            *pSmallWaves = 0.1f;
        }
        if (pBigWaves) {
            *pBigWaves = 0.2f;
        }
        return true;
    }

    if (g_origGetWaterLevelNoWaves) {
        return g_origGetWaterLevelNoWaves(x, y, z, pOutWaterLevel, pSmallWaves, pBigWaves);
    }
    return false;
}

bool __cdecl Hooked_TestLineAgainstWater(float startX, float startY, float startZ, float endX, float endY, float endZ, CVector* pCol) {
    if (TestLine(startX, startY, startZ, endX, endY, endZ, pCol)) {
        return true;
    }

    if (g_origTestLineAgainstWater) {
        return g_origTestLineAgainstWater(startX, startY, startZ, endX, endY, endZ, pCol);
    }
    return false;
}

bool __cdecl Hooked_CanSeeWaterFromCurrArea() {
    if (HasActiveZones()) {
        return true;
    }

    if (g_origCanSeeWaterFromCurrArea) {
        return g_origCanSeeWaterFromCurrArea();
    }
    return false;
}

bool InstallHooks() {
    if (g_HooksInstalled) {
        return true;
    }

    // 1. Hook CWaterLevel::GetWaterLevel (0x6EB690)
    if (MH_CreateHook(reinterpret_cast<void*>(0x6EB690), reinterpret_cast<void*>(&Hooked_GetWaterLevel), reinterpret_cast<void**>(&g_origGetWaterLevel)) == MH_OK) {
        MH_EnableHook(reinterpret_cast<void*>(0x6EB690));
    }

    // 2. Hook CWaterLevel::GetWaterLevelNoWaves (0x6E8580)
    if (MH_CreateHook(reinterpret_cast<void*>(0x6E8580), reinterpret_cast<void*>(&Hooked_GetWaterLevelNoWaves), reinterpret_cast<void**>(&g_origGetWaterLevelNoWaves)) == MH_OK) {
        MH_EnableHook(reinterpret_cast<void*>(0x6E8580));
    }

    // 3. Hook CWaterLevel::TestLineAgainstWater (0x6E61B0)
    if (MH_CreateHook(reinterpret_cast<void*>(0x6E61B0), reinterpret_cast<void*>(&Hooked_TestLineAgainstWater), reinterpret_cast<void**>(&g_origTestLineAgainstWater)) == MH_OK) {
        MH_EnableHook(reinterpret_cast<void*>(0x6E61B0));
    }

    // 4. Hook CGame::CanSeeWaterFromCurrArea (0x53C4B0) to enable water rendering in any interior
    if (MH_CreateHook(reinterpret_cast<void*>(0x53C4B0), reinterpret_cast<void*>(&Hooked_CanSeeWaterFromCurrArea), reinterpret_cast<void**>(&g_origCanSeeWaterFromCurrArea)) == MH_OK) {
        MH_EnableHook(reinterpret_cast<void*>(0x53C4B0));
    }

    // 5. Hook RenderWater right before buffer flush (0x6F0064)
    // Overwrite: 0x6F0064 (6 bytes: 39 1D 50 B9 C4 00) with call Hook_RenderWater_Zones + nop
    plugin::patch::RedirectCall(0x6F0064, Hook_RenderWater_Zones);
    plugin::patch::SetUChar(0x6F0069, 0x90);
    s_flushHookInstalled = true;

    g_HooksInstalled = true;
    ClientLog(LogLevel::Info, "Installed Custom Water Zone hooks");
    return true;
}

void RemoveHooks() {
    if (!g_HooksInstalled) {
        return;
    }

    MH_DisableHook(reinterpret_cast<void*>(0x6EB690));
    MH_RemoveHook(reinterpret_cast<void*>(0x6EB690));
    g_origGetWaterLevel = nullptr;

    MH_DisableHook(reinterpret_cast<void*>(0x6E8580));
    MH_RemoveHook(reinterpret_cast<void*>(0x6E8580));
    g_origGetWaterLevelNoWaves = nullptr;

    MH_DisableHook(reinterpret_cast<void*>(0x6E61B0));
    MH_RemoveHook(reinterpret_cast<void*>(0x6E61B0));
    g_origTestLineAgainstWater = nullptr;

    MH_DisableHook(reinterpret_cast<void*>(0x53C4B0));
    MH_RemoveHook(reinterpret_cast<void*>(0x53C4B0));
    g_origCanSeeWaterFromCurrArea = nullptr;

    if (s_flushHookInstalled) {
        plugin::patch::SetRaw(0x6F0064, s_origFlushCheckBytes, sizeof(s_origFlushCheckBytes));
        s_flushHookInstalled = false;
    }

    g_HooksInstalled = false;
}

} // namespace

bool Initialize() {
    g_Initialized = true;
    InstallHooks();
    return true;
}

void Process() {
    if (!g_Initialized) {
        return;
    }

    if (!g_HooksInstalled) {
        InstallHooks();
    }
}

void Shutdown() {
    RemoveHooks();
    ClearAllStates();
    g_Initialized = false;
}

bool Create(uint16_t zoneId, float minX, float minY, float maxX, float maxY, float level, float bottomZ) {
    if (zoneId >= MAX_WATER_ZONES ||
        !std::isfinite(minX) || !std::isfinite(minY) ||
        !std::isfinite(maxX) || !std::isfinite(maxY) ||
        !std::isfinite(level) || !std::isfinite(bottomZ)) {
        return false;
    }

    const std::lock_guard<std::mutex> lock(g_Mutex);
    auto& zone = g_Zones[zoneId];
    zone.enabled = true;
    zone.minX = std::min(minX, maxX);
    zone.maxX = std::max(minX, maxX);
    zone.minY = std::min(minY, maxY);
    zone.maxY = std::max(minY, maxY);
    zone.level = level;
    zone.bottomZ = std::min(bottomZ, level);
    return true;
}

bool SetLevel(uint16_t zoneId, float level) {
    if (zoneId >= MAX_WATER_ZONES || !std::isfinite(level)) {
        return false;
    }

    const std::lock_guard<std::mutex> lock(g_Mutex);
    auto& zone = g_Zones[zoneId];
    if (!zone.enabled) {
        return false;
    }
    zone.level = level;
    if (zone.bottomZ > level) {
        zone.bottomZ = level - 5.0f;
    }
    return true;
}

bool SetCoords(uint16_t zoneId, float minX, float minY, float maxX, float maxY, float level, float bottomZ) {
    return Create(zoneId, minX, minY, maxX, maxY, level, bottomZ);
}

bool Destroy(uint16_t zoneId) {
    if (zoneId >= MAX_WATER_ZONES) {
        return false;
    }

    const std::lock_guard<std::mutex> lock(g_Mutex);
    g_Zones[zoneId] = {};
    return true;
}

void ClearAllStates() {
    const std::lock_guard<std::mutex> lock(g_Mutex);
    g_Zones.fill({});
}

bool HasActiveZones() {
    const std::lock_guard<std::mutex> lock(g_Mutex);
    for (const auto& zone : g_Zones) {
        if (zone.enabled) return true;
    }
    return false;
}

bool GetWaterLevel(float x, float y, float z, float& outLevel) {
    const std::lock_guard<std::mutex> lock(g_Mutex);
    for (const auto& zone : g_Zones) {
        if (!zone.enabled) continue;
        if (x >= zone.minX && x <= zone.maxX && y >= zone.minY && y <= zone.maxY) {
            // Player or vehicle can enter water if up to 20m above water surface and down to bottomZ
            if (z <= (zone.level + 20.0f) && z >= (zone.bottomZ - 1.0f)) {
                outLevel = zone.level;
                return true;
            }
        }
    }
    return false;
}

bool TestLine(float startX, float startY, float startZ, float endX, float endY, float endZ, CVector* pCol) {
    const std::lock_guard<std::mutex> lock(g_Mutex);
    for (const auto& zone : g_Zones) {
        if (!zone.enabled) continue;

        const float zPlane = zone.level;
        if ((startZ >= zPlane && endZ <= zPlane) || (startZ <= zPlane && endZ >= zPlane)) {
            const float denom = endZ - startZ;
            if (std::fabs(denom) > 0.0001f) {
                const float t = (zPlane - startZ) / denom;
                if (t >= 0.0f && t <= 1.0f) {
                    const float ix = startX + t * (endX - startX);
                    const float iy = startY + t * (endY - startY);
                    if (ix >= zone.minX && ix <= zone.maxX && iy >= zone.minY && iy <= zone.maxY) {
                        if (pCol) {
                            pCol->x = ix;
                            pCol->y = iy;
                            pCol->z = zPlane;
                        }
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void RenderAll() {
    auto pRenderRect = reinterpret_cast<RenderWaterRectangle_t>(0x6EC5D0);
    if (!pRenderRect) return;

    std::array<WaterZone, MAX_WATER_ZONES> zonesCopy;
    {
        const std::lock_guard<std::mutex> lock(g_Mutex);
        zonesCopy = g_Zones;
    }

    for (const auto& zone : zonesCopy) {
        if (!zone.enabled) continue;

        const int32_t rMinX = static_cast<int32_t>(std::floor(zone.minX));
        const int32_t rMaxX = static_cast<int32_t>(std::ceil(zone.maxX));
        const int32_t rMinY = static_cast<int32_t>(std::floor(zone.minY));
        const int32_t rMaxY = static_cast<int32_t>(std::ceil(zone.maxY));

        if (rMinX >= rMaxX || rMinY >= rMaxY) continue;

        WaterRenPar p;
        p.z = zone.level;
        p.bigWaves = 0.2f;
        p.smallWaves = 0.1f;
        p.flowX = 0;
        p.flowY = 0;

        // Subdivide into chunks of at most 64m for smooth wave tesselation and camera culling
        constexpr int32_t CHUNK_SIZE = 64;
        for (int32_t x = rMinX; x < rMaxX; x += CHUNK_SIZE) {
            const int32_t nextX = std::min(x + CHUNK_SIZE, rMaxX);
            for (int32_t y = rMinY; y < rMaxY; y += CHUNK_SIZE) {
                const int32_t nextY = std::min(y + CHUNK_SIZE, rMaxY);
                pRenderRect(x, nextX, y, nextY, p, p, p, p);
            }
        }
    }
}

}
