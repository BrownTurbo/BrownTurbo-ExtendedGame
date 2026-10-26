#include "game_extensions.h"
#include "defs.h"
#include "utils.h"

#include <MinHook.h>
#include <plugin.h>
#include <game_sa/CPed.h>
#include <game_sa/CPlayerPed.h>
#include <game_sa/CVehicle.h>
#include <game_sa/CCoronas.h>
#include <game_sa/CPlantMgr.h>
#include <game_sa/CHud.h>
#include <game_sa/CHudColours.h>
#include <game_sa/CClouds.h>
#include <game_sa/CWaterLevel.h>
#include <game_sa/CWeapon.h>
#include <game_sa/CPad.h>
#include <game_sa/CStreaming.h>
#include <game_sa/CTxdStore.h>
#include <game_sa/CModelInfo.h>
#include <game_sa/CFileLoader.h>
#include <game_sa/CTaskSimpleJetpack.h>
#include <game_sa/CCamera.h>
#include <game_sa/CWorld.h>
#include <RenderWare.h>

#include <array>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include <windows.h>

namespace game_extensions {

// State variables
static float g_fAircraftHeightLimit = 800.0f; // Default GTA:SA limit is 800.0f
static float g_fAircraftHeightLimitForGame = 800.0f;
static float g_fJetpackHeightLimit  = 100.0f; // Default GTA:SA limit is 100.0f

// Jetpack clump placement & orientation state
// Vanilla default offset: { 0.1f, 0.08f, 0.0f }, axis { 0.0f, 1.0f, 0.0f }, angle 90.0f
// For the custom umbrella jetpack (Y center in DFF is shifted by +0.462m):
// Compensating Y by -0.462m (0.08 - 0.462 = -0.38m) aligns the umbrella shaft snugly onto CJ's spine.
static CVector g_vecJetpackOffset  = { 0.10f, -0.38f, 0.00f };
static CVector g_vecJetpackRotAxis = { 0.00f, 1.00f, 0.00f };
static float   g_fJetpackRotAngle  = 90.0f;

// Original opcode at 0x67F26A in CTaskSimpleJetPack::ProcessThrust:
// 8B 47 14 (mov eax, [edi + 0x14])
// When height limit is active, jump straight to 0x67F2A9 (fld dword ptr [0x858B50] -> loads 0.0f):
// Target offset: 0x67F2A9 - (0x67F26A + 2) = 0x3D
// Patch: { 0xEB, 0x3D, 0x90 } (jmp short +0x3D, nop)
static const uint8_t s_origThrustDampingOpcode[3] = { 0x8B, 0x47, 0x14 };
static const uint8_t s_patchThrustZeroOpcode[3]    = { 0xEB, 0x3D, 0x90 };
static bool s_bJetpackThrustPatched = false;
static bool g_bInfiniteSprint       = false;
static bool g_bSunEnabled           = true;

static bool g_bMoonEnabled          = true;
static bool g_bStarsEnabled         = true;
static bool g_bLowCloudsEnabled     = true;
static bool g_bFluffyCloudsEnabled  = true;
static bool g_bRainbowEnabled       = true;
static float g_fWaterLevel          = 0.0f;
static float g_fWaterWaveHeight     = 0.0f;
static float g_fRequestedWaterWaveHeight = 0.0f;
static constexpr float WATER_WAVE_PERCENT_SCALE = 0.01f;
static bool g_bRestoreNativeWaterWaves = false;
static bool g_bWaterLevelOverrideEnabled = false;
static bool g_bIncludeWorldNonSeaWater = false;
static bool g_bIncludeWorldSeaWater = false;
static bool g_bIncludeOutsideWorldWater = false;
static bool g_bWaterRenderOrderEnabled = false;
static constexpr size_t WATER_WORLD_VERTEX_COUNT = 0x3FD;
static std::array<float, WATER_WORLD_VERTEX_COUNT> g_WorldWaterDefaultLevels{};
static bool g_bWorldWaterDefaultsCaptured = false;
static constexpr std::array<uintptr_t, 2> AIRCRAFT_HEIGHT_LIMIT_OPERANDS{{0x6D2614, 0x6D2625}};
static std::array<uint32_t, AIRCRAFT_HEIGHT_LIMIT_OPERANDS.size()> g_AircraftHeightLimitOriginalOperands{};
static bool g_bAircraftHeightLimitOperandsCaptured = false;
static constexpr uintptr_t SUN_RENDER_FUNCTION = 0x6FC5A0;
static constexpr uintptr_t SUN_REFLECTION_FUNCTION = 0x6FBAA0;
static std::array<uint8_t, 2> g_SunRenderOriginalOpcodes{};
static bool g_bSunRenderOriginalOpcodesCaptured = false;

struct WaterRenderOrderPatch {
    uintptr_t address;
    size_t size;
    std::array<uint8_t, 6> originalBytes{};
};

static std::array<WaterRenderOrderPatch, 8> g_WaterRenderOrderPatches{{
    {0x53DFF5, 1},
    {0x53E133, 2},
    {0x53E132, 1},
    {0x53E156, 3},
    {0x53DF4B, 4},
    {0x53E159, 5},
    {0x5DBC4C, 6},
    {0x53E103, 5}
}};
static bool g_bWaterRenderOrderBytesCaptured = false;

static uint32_t g_nHUDComponents    = HUD_COMPONENT_ALL; // 511

struct HUDColoursOverride {
    uint32_t ammoColor       = 0;
    uint32_t weaponIconColor = 0;
    uint32_t healthColor     = 0;
    uint32_t oxygenColor     = 0;
    uint32_t armourColor     = 0;
    uint32_t radarColor      = 0;
    uint32_t crosshairColor  = 0;
    uint32_t moneyColor      = 0;
    uint32_t laserDotColor   = 0;
};
static HUDColoursOverride g_HUDColours;

static std::unordered_map<uint16_t, uint8_t> g_PlayerKnockOffBike;
static std::unordered_map<uint16_t, bool>    g_PlayerFireProof;

// Function pointers for original hooked routines
using VolumetricCloudsRender_t = void(__cdecl*)();
static VolumetricCloudsRender_t g_origVolumetricCloudsRender = nullptr;

using WaterLevelInitialise_t = void(__cdecl*)();
static WaterLevelInitialise_t g_origWaterLevelInitialise = nullptr;

using PreRenderWater_t = void(__cdecl*)();
static PreRenderWater_t g_origPreRenderWater = nullptr;

using DrawAmmo_t = void(__cdecl*)(CPed*, int, int, float);
static DrawAmmo_t g_origDrawAmmo = nullptr;

using DrawWeaponIcon_t = void(__cdecl*)(CPed*, int, int, float);
static DrawWeaponIcon_t g_origDrawWeaponIcon = nullptr;

using RenderHealthBar_t = void(__cdecl*)(int, int, int);
static RenderHealthBar_t g_origRenderHealthBar = nullptr;

using RenderBreathBar_t = void(__cdecl*)(int, int, int);
static RenderBreathBar_t g_origRenderBreathBar = nullptr;

using RenderArmorBar_t = void(__cdecl*)(int, int, int);
static RenderArmorBar_t g_origRenderArmorBar = nullptr;

using DrawRadar_t = void(__cdecl*)();
static DrawRadar_t g_origDrawRadar = nullptr;

using DrawCrossHairs_t = void(__cdecl*)();
static DrawCrossHairs_t g_origDrawCrossHairs = nullptr;

static bool g_bInitialized = false;

// Memory Patch Helpers
static void ApplyMoonPatch(bool enable) {
    if (enable) {
        // Original bytes at 0x713A05 in CClouds::Render: jge 0x713D2A
        uint8_t origBytes[6] = { 0x0F, 0x8D, 0x1F, 0x03, 0x00, 0x00 };
        plugin::patch::SetRaw(0x713A05, origBytes, 6);
    } else {
        // Unconditionally jump past moon rendering to 0x713D2A
        uint8_t patchBytes[6] = { 0xE9, 0x20, 0x03, 0x00, 0x00, 0x90 };
        plugin::patch::SetRaw(0x713A05, patchBytes, 6);
    }
}

static void ApplyStarsPatch(bool enable) {
    if (enable) {
        // Original bytes at 0x713D2A in CClouds::Render: cmp bl, 0x16; jae 0x713D3B
        uint8_t origBytes[5] = { 0x80, 0xFB, 0x16, 0x73, 0x0C };
        plugin::patch::SetRaw(0x713D2A, origBytes, 5);
    } else {
        // Unconditionally jump past stars rendering to 0x71401E
        uint8_t patchBytes[5] = { 0xE9, 0xEF, 0x02, 0x00, 0x00 };
        plugin::patch::SetRaw(0x713D2A, patchBytes, 5);
    }
}

static void ApplyLowCloudsPatch(bool enable) {
    if (enable) {
        uint8_t origBytes[5] = { 0x33, 0xF6, 0xEB, 0x06, 0x8B };
        plugin::patch::SetRaw(0x71413B, origBytes, 5);
    } else {
        uint8_t patchBytes[5] = { 0xE9, 0xEA, 0x00, 0x00, 0x00 };
        plugin::patch::SetRaw(0x71413B, patchBytes, 5);
    }
}

static void ApplyRainbowPatch(bool enable) {
    if (enable) {
        uint8_t origBytes[6] = { 0x0F, 0x8B, 0x46, 0x01, 0x00, 0x00 };
        plugin::patch::SetRaw(0x714240, origBytes, 6);
    } else {
        uint8_t patchBytes[6] = { 0xE9, 0x47, 0x01, 0x00, 0x00, 0x90 };
        plugin::patch::SetRaw(0x714240, patchBytes, 6);
    }
}

static void ApplyMoneyPatch(bool enable) {
    if (enable) {
        uint8_t origBytes[6] = { 0x0F, 0x84, 0x95, 0x01, 0x00, 0x00 }; // je 0x58F618
        plugin::patch::SetRaw(0x58F47D, origBytes, 6);
    } else {
        uint8_t patchBytes[6] = { 0xE9, 0x96, 0x01, 0x00, 0x00, 0x90 }; // jmp 0x58F618; nop
        plugin::patch::SetRaw(0x58F47D, patchBytes, 6);
    }
}

static void ApplyLaserDotColorPatch(uint32_t col) {
    if (col != 0) {
        plugin::patch::SetUChar(0x73AA83, static_cast<uint8_t>(col & 0xFF));         // Alpha
        plugin::patch::SetUChar(0x73AA88, static_cast<uint8_t>((col >> 8) & 0xFF));  // Blue
        plugin::patch::SetUChar(0x73AA8A, static_cast<uint8_t>((col >> 16) & 0xFF)); // Green
        plugin::patch::SetUChar(0x73AA8C, static_cast<uint8_t>((col >> 24) & 0xFF)); // Red
    } else {
        plugin::patch::SetUChar(0x73AA83, 0xFF);
        plugin::patch::SetUChar(0x73AA88, 0x00);
        plugin::patch::SetUChar(0x73AA8A, 0x00);
        plugin::patch::SetUChar(0x73AA8C, 0x80);
    }
}

static void ApplyAircraftHeightLimitOperandPointers() {
    if (!g_bAircraftHeightLimitOperandsCaptured) {
        return;
    }

    auto limitAddress = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(&g_fAircraftHeightLimitForGame)
    );
    for (const uintptr_t operandAddress : AIRCRAFT_HEIGHT_LIMIT_OPERANDS) {
        plugin::patch::SetRaw(operandAddress, &limitAddress, sizeof(limitAddress));
    }
}

static void ApplySunEnabled(bool enable) {
    if (!g_bSunRenderOriginalOpcodesCaptured) {
        return;
    }

    const std::array<uintptr_t, 2> functionAddresses{{
        SUN_RENDER_FUNCTION,
        SUN_REFLECTION_FUNCTION
    }};
    for (size_t i = 0; i < functionAddresses.size(); ++i) {
        plugin::patch::SetUChar(
            functionAddresses[i],
            enable ? g_SunRenderOriginalOpcodes[i] : 0xC3
        );
    }
}

// 4. Hook for Volumetric / Fluffy Clouds (CClouds::VolumetricCloudsRender at 0x716380)
static void Hooked_VolumetricCloudsRender() {
    if (!g_bFluffyCloudsEnabled) {
        return; // Suppress fluffy volumetric clouds
    }
    if (g_origVolumetricCloudsRender) {
        g_origVolumetricCloudsRender();
    }
}

static bool CaptureWorldWaterDefaults() {
    if (g_bWorldWaterDefaultsCaptured) {
        return true;
    }

    if (CWaterLevel::m_nNumOfWaterVertices < static_cast<int>(WATER_WORLD_VERTEX_COUNT) ||
        CWaterLevel::m_nNumOfWaterVertices > static_cast<int>(CWaterLevel::m_aVertices.size())) {
        return false;
    }

    for (size_t i = 0; i < WATER_WORLD_VERTEX_COUNT; ++i) {
        g_WorldWaterDefaultLevels[i] = CWaterLevel::m_aVertices[i].rp.z;
    }
    g_bWorldWaterDefaultsCaptured = true;
    return true;
}

static void SetOutsideWorldWaterLevel(float level) {
    plugin::patch::SetFloat(0x6EFECC, level);
    plugin::patch::SetFloat(0x6EFF0C, level);
    plugin::patch::SetFloat(0x6EFF4A, level);
    plugin::patch::SetFloat(0x6EFFA6, level);
    plugin::patch::SetFloat(0x6E873F, level);
    plugin::patch::SetFloat(0x6EA238, level);
}

static void __declspec(naked) HookPlantsBeforeWater() {
    __asm {
        push 1
        movzx eax, bl
        push eax
        mov eax, 0x5DBAE0
        call eax
        add esp, 8
        ret
    }
}

static void __declspec(naked) HookPlantsAfterWater() {
    __asm {
        push 0
        movzx eax, bl
        push eax
        mov eax, 0x5DBAE0
        call eax
        add esp, 8

        pop ebx
        add esp, 8
        mov eax, 0x7113B0
        jmp eax
    }
}

static void __declspec(naked) HookPlantRenderContinuation() {
    __asm {
        mov ax, word ptr [esi - 10h]
        mov edx, edi
        mov eax, 0x5DBC52
        jmp eax
    }
}

static void WriteRelativeBranch(uintptr_t address, uint8_t opcode, const void* destination) {
    std::array<uint8_t, 5> branchBytes{opcode, 0, 0, 0, 0};
    const auto displacement = static_cast<int32_t>(
        reinterpret_cast<intptr_t>(destination) - static_cast<intptr_t>(address + branchBytes.size())
    );
    std::memcpy(branchBytes.data() + 1, &displacement, sizeof(displacement));
    plugin::patch::SetRaw(address, branchBytes.data(), branchBytes.size());
}

static void SetWaterRenderOrderEnabled(bool enable) {
    if (enable == g_bWaterRenderOrderEnabled) {
        return;
    }

    if (enable) {
        if (!g_bWaterRenderOrderBytesCaptured) {
            for (auto& patch : g_WaterRenderOrderPatches) {
                std::memcpy(patch.originalBytes.data(), reinterpret_cast<const void*>(patch.address), patch.size);
            }
            g_bWaterRenderOrderBytesCaptured = true;
        }

        plugin::patch::SetUChar(0x53DFF5, 0xEB);
        uint8_t skipWaterBytes[] = {0x90, 0x90};
        plugin::patch::SetRaw(0x53E133, skipWaterBytes, sizeof(skipWaterBytes));
        plugin::patch::SetUChar(0x53E132, 0x90);
        uint8_t stackBytes[] = {0x90, 0x90, 0x90};
        plugin::patch::SetRaw(0x53E156, stackBytes, sizeof(stackBytes));
        uint8_t underwaterThresholdBytes[] = {0x8C, 0x8B, 0x85, 0x00};
        plugin::patch::SetRaw(0x53DF4B, underwaterThresholdBytes, sizeof(underwaterThresholdBytes));

        WriteRelativeBranch(0x53E159, 0xE9, reinterpret_cast<const void*>(&HookPlantsAfterWater));
        WriteRelativeBranch(0x5DBC4C, 0xE9, reinterpret_cast<const void*>(&HookPlantRenderContinuation));
        WriteRelativeBranch(0x53E103, 0xE8, reinterpret_cast<const void*>(&HookPlantsBeforeWater));
        g_bWaterRenderOrderEnabled = true;
        return;
    }

    for (auto patch = g_WaterRenderOrderPatches.rbegin(); patch != g_WaterRenderOrderPatches.rend(); ++patch) {
        plugin::patch::SetRaw(patch->address, patch->originalBytes.data(), patch->size);
    }
    g_bWaterRenderOrderEnabled = false;
    g_bWaterRenderOrderBytesCaptured = false;
}

static void ApplyWorldWaterLevelOverride(bool restoreUnselectedVertices) {
    if (!g_bWaterLevelOverrideEnabled || !g_bWorldWaterDefaultsCaptured) {
        return;
    }

    for (size_t i = 0; i < WATER_WORLD_VERTEX_COUNT; ++i) {
        const float defaultLevel = g_WorldWaterDefaultLevels[i];
        const bool isWorldNonSeaLevel = defaultLevel != 0.0f;
        const bool shouldOverride =
            (isWorldNonSeaLevel && g_bIncludeWorldNonSeaWater) ||
            (!isWorldNonSeaLevel && g_bIncludeWorldSeaWater);
        if (shouldOverride) {
            CWaterLevel::m_aVertices[i].rp.z = g_fWaterLevel;
        } else if (restoreUnselectedVertices) {
            CWaterLevel::m_aVertices[i].rp.z = defaultLevel;
        }
    }
}

static void ApplyWaterLevelOverride() {
    if (!g_bWaterLevelOverrideEnabled) {
        if (g_bWorldWaterDefaultsCaptured) {
            for (size_t i = 0; i < WATER_WORLD_VERTEX_COUNT; ++i) {
                CWaterLevel::m_aVertices[i].rp.z = g_WorldWaterDefaultLevels[i];
            }
        }
        SetOutsideWorldWaterLevel(0.0f);
        SetWaterRenderOrderEnabled(false);
        return;
    }

    ApplyWorldWaterLevelOverride(true);
    if (g_bIncludeOutsideWorldWater) {
        SetOutsideWorldWaterLevel(g_fWaterLevel);
    } else {
        SetOutsideWorldWaterLevel(0.0f);
    }
    SetWaterRenderOrderEnabled(true);
}

static void RestoreWaterWaveHeight() {
    plugin::patch::SetUChar(0x72C665, 0xD9);
    plugin::patch::SetUChar(0x72C666, 0x1D);
    plugin::patch::SetUChar(0x72C667, 0xE8);
    plugin::patch::SetUChar(0x72C668, 0x12);
    plugin::patch::SetUChar(0x72C669, 0xC8);
    plugin::patch::SetUChar(0x72C66A, 0x00);

    plugin::patch::SetUChar(0x72C659, 0xC7);
    plugin::patch::SetUChar(0x72C65A, 0x05);
    plugin::patch::SetUChar(0x72C65B, 0xE8);
    plugin::patch::SetUChar(0x72C65C, 0x12);
    plugin::patch::SetUChar(0x72C65D, 0xC8);
    plugin::patch::SetUChar(0x72C65E, 0x00);
    plugin::patch::SetUChar(0x72C65F, 0x00);
    plugin::patch::SetUChar(0x72C660, 0x00);
    plugin::patch::SetUChar(0x72C661, 0x80);
    plugin::patch::SetUChar(0x72C662, 0x3F);
    plugin::patch::SetFloat(0xC812E8, 0.6f);
    g_fWaterWaveHeight = 0.6f;
}

static void ApplyWaterWaveHeight(float height) {
    plugin::patch::SetUChar(0x72C665, 0xDD);
    plugin::patch::SetUChar(0x72C666, 0xD8);
    plugin::patch::SetUChar(0x72C667, 0x90);
    plugin::patch::SetUChar(0x72C668, 0x90);
    plugin::patch::SetUChar(0x72C669, 0x90);
    plugin::patch::SetUChar(0x72C66A, 0x90);
    for (uintptr_t address = 0x72C659; address <= 0x72C662; ++address) {
        plugin::patch::SetUChar(address, 0x90);
    }
    plugin::patch::SetFloat(0xC812E8, height);
    g_fWaterWaveHeight = height;
}

static void __cdecl Hooked_WaterLevelInitialise() {
    if (g_origWaterLevelInitialise) {
        g_origWaterLevelInitialise();
    }

    g_bWorldWaterDefaultsCaptured = false;
    if (!CaptureWorldWaterDefaults()) {
        ClientLog(LogLevel::Warning, "Water initialization completed, but world-water defaults could not be captured");
    }
    ApplyWaterLevelOverride();

    if (g_bRestoreNativeWaterWaves) {
        RestoreWaterWaveHeight();
    } else {
        ApplyWaterWaveHeight(g_fRequestedWaterWaveHeight * WATER_WAVE_PERCENT_SCALE);
    }
}

static void __cdecl Hooked_PreRenderWater() {
    if (!g_bWorldWaterDefaultsCaptured &&
        (g_bIncludeWorldNonSeaWater || g_bIncludeWorldSeaWater)) {
        CaptureWorldWaterDefaults();
    }
    ApplyWorldWaterLevelOverride(false);

    if (g_origPreRenderWater) {
        g_origPreRenderWater();
    }
}

// 5. Hook for Ammo Display (CHud::DrawAmmo at 0x5893B0)
static void Hooked_DrawAmmo(CPed* ped, int x, int y, float alpha) {
    if (!(g_nHUDComponents & HUD_COMPONENT_AMMO)) {
        return; // Suppress ammo rendering
    }

    CHudColour oldColor = HudColour.m_aColours[HUD_COLOUR_BLUELIGHT];
    if (g_HUDColours.ammoColor != 0) {
        uint32_t col = g_HUDColours.ammoColor;
        HudColour.m_aColours[HUD_COLOUR_BLUELIGHT].red   = static_cast<uint8_t>((col >> 24) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_BLUELIGHT].green = static_cast<uint8_t>((col >> 16) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_BLUELIGHT].blue  = static_cast<uint8_t>((col >> 8) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_BLUELIGHT].alpha = static_cast<uint8_t>(col & 0xFF);
    }

    if (g_origDrawAmmo) {
        g_origDrawAmmo(ped, x, y, alpha);
    }

    if (g_HUDColours.ammoColor != 0) {
        HudColour.m_aColours[HUD_COLOUR_BLUELIGHT] = oldColor;
    }
}

// 6. Hook for Weapon Icon (CHud::DrawWeaponIcon at 0x58D7D0)
static void Hooked_DrawWeaponIcon(CPed* ped, int x, int y, float alpha) {
    if (!(g_nHUDComponents & HUD_COMPONENT_WEAPONICON)) {
        return; // Suppress weapon icon rendering
    }

    if (g_origDrawWeaponIcon) {
        g_origDrawWeaponIcon(ped, x, y, alpha);
    }
}

// 7. Hook for Health Bar (CHud::RenderHealthBar at 0x589270)
static void Hooked_RenderHealthBar(int playerId, int x, int y) {
    if (!(g_nHUDComponents & HUD_COMPONENT_HEALTH)) {
        return; // Suppress health bar rendering
    }

    CHudColour oldColor = HudColour.m_aColours[HUD_COLOUR_RED];
    if (g_HUDColours.healthColor != 0) {
        uint32_t col = g_HUDColours.healthColor;
        HudColour.m_aColours[HUD_COLOUR_RED].red   = static_cast<uint8_t>((col >> 24) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_RED].green = static_cast<uint8_t>((col >> 16) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_RED].blue  = static_cast<uint8_t>((col >> 8) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_RED].alpha = static_cast<uint8_t>(col & 0xFF);
    }

    if (g_origRenderHealthBar) {
        g_origRenderHealthBar(playerId, x, y);
    }

    if (g_HUDColours.healthColor != 0) {
        HudColour.m_aColours[HUD_COLOUR_RED] = oldColor;
    }
}

// 8. Hook for Breath Bar (CHud::RenderBreathBar at 0x589190)
static void Hooked_RenderBreathBar(int playerId, int x, int y) {
    if (!(g_nHUDComponents & HUD_COMPONENT_OXYGEN)) {
        return; // Suppress oxygen/breath bar rendering
    }

    CHudColour oldColor = HudColour.m_aColours[HUD_COLOUR_BLUELIGHT];
    if (g_HUDColours.oxygenColor != 0) {
        uint32_t col = g_HUDColours.oxygenColor;
        HudColour.m_aColours[HUD_COLOUR_BLUELIGHT].red   = static_cast<uint8_t>((col >> 24) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_BLUELIGHT].green = static_cast<uint8_t>((col >> 16) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_BLUELIGHT].blue  = static_cast<uint8_t>((col >> 8) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_BLUELIGHT].alpha = static_cast<uint8_t>(col & 0xFF);
    }

    if (g_origRenderBreathBar) {
        g_origRenderBreathBar(playerId, x, y);
    }

    if (g_HUDColours.oxygenColor != 0) {
        HudColour.m_aColours[HUD_COLOUR_BLUELIGHT] = oldColor;
    }
}

// 9. Hook for Armor Bar (CHud::RenderArmorBar at 0x5890A0)
static void Hooked_RenderArmorBar(int playerId, int x, int y) {
    if (!(g_nHUDComponents & HUD_COMPONENT_ARMOUR)) {
        return; // Suppress armor bar rendering
    }

    CHudColour oldColor = HudColour.m_aColours[HUD_COLOUR_GREY];
    if (g_HUDColours.armourColor != 0) {
        uint32_t col = g_HUDColours.armourColor;
        HudColour.m_aColours[HUD_COLOUR_GREY].red   = static_cast<uint8_t>((col >> 24) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_GREY].green = static_cast<uint8_t>((col >> 16) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_GREY].blue  = static_cast<uint8_t>((col >> 8) & 0xFF);
        HudColour.m_aColours[HUD_COLOUR_GREY].alpha = static_cast<uint8_t>(col & 0xFF);
    }

    if (g_origRenderArmorBar) {
        g_origRenderArmorBar(playerId, x, y);
    }

    if (g_HUDColours.armourColor != 0) {
        HudColour.m_aColours[HUD_COLOUR_GREY] = oldColor;
    }
}

// 10. Hook for Radar (CHud::DrawRadar at 0x58A330)
static void Hooked_DrawRadar() {
    if (!(g_nHUDComponents & HUD_COMPONENT_RADAR)) {
        return; // Suppress radar rendering
    }

    if (g_origDrawRadar) {
        g_origDrawRadar();
    }
}

// 11. Hook for Crosshair (CHud::DrawCrossHairs at 0x58E020)
static void Hooked_DrawCrossHairs() {
    if (!(g_nHUDComponents & HUD_COMPONENT_CROSSHAIR)) {
        return; // Suppress crosshair rendering
    }

    if (g_origDrawCrossHairs) {
        g_origDrawCrossHairs();
    }
}

bool Initialize() {
    if (g_bInitialized) return true;

    const MH_STATUS minHookStatus = MH_Initialize();
    if (minHookStatus != MH_OK && minHookStatus != MH_ERROR_ALREADY_INITIALIZED) {
        ClientLog(LogLevel::Error, std::format("Failed to initialize MinHook for game extensions (status={})", static_cast<int>(minHookStatus)));
        return false;
    }

    const auto waterInitialiseAddress = reinterpret_cast<void*>(0x6EAE80);
    const MH_STATUS waterHookStatus = MH_CreateHook(
        waterInitialiseAddress,
        reinterpret_cast<void*>(&Hooked_WaterLevelInitialise),
        reinterpret_cast<void**>(&g_origWaterLevelInitialise)
    );
    if (waterHookStatus != MH_OK) {
        ClientLog(LogLevel::Error, std::format("Failed to hook CWaterLevel::WaterLevelInitialise (status={})", static_cast<int>(waterHookStatus)));
        return false;
    }

    const MH_STATUS waterEnableStatus = MH_EnableHook(waterInitialiseAddress);
    if (waterEnableStatus != MH_OK) {
        ClientLog(LogLevel::Error, std::format("Failed to enable CWaterLevel::WaterLevelInitialise hook (status={})", static_cast<int>(waterEnableStatus)));
        MH_RemoveHook(waterInitialiseAddress);
        g_origWaterLevelInitialise = nullptr;
        return false;
    }

    const auto preRenderWaterAddress = reinterpret_cast<void*>(0x6EB710);
    const MH_STATUS preRenderWaterHookStatus = MH_CreateHook(
        preRenderWaterAddress,
        reinterpret_cast<void*>(&Hooked_PreRenderWater),
        reinterpret_cast<void**>(&g_origPreRenderWater)
    );
    if (preRenderWaterHookStatus != MH_OK) {
        ClientLog(LogLevel::Error, std::format("Failed to hook CWaterLevel::PreRenderWater (status={})", static_cast<int>(preRenderWaterHookStatus)));
        MH_DisableHook(waterInitialiseAddress);
        MH_RemoveHook(waterInitialiseAddress);
        g_origWaterLevelInitialise = nullptr;
        return false;
    }

    const MH_STATUS preRenderWaterEnableStatus = MH_EnableHook(preRenderWaterAddress);
    if (preRenderWaterEnableStatus != MH_OK) {
        ClientLog(LogLevel::Error, std::format("Failed to enable CWaterLevel::PreRenderWater hook (status={})", static_cast<int>(preRenderWaterEnableStatus)));
        MH_DisableHook(waterInitialiseAddress);
        MH_RemoveHook(waterInitialiseAddress);
        MH_RemoveHook(preRenderWaterAddress);
        g_origWaterLevelInitialise = nullptr;
        g_origPreRenderWater = nullptr;
        return false;
    }

    CaptureWorldWaterDefaults();

    if (!g_bAircraftHeightLimitOperandsCaptured) {
        for (size_t i = 0; i < AIRCRAFT_HEIGHT_LIMIT_OPERANDS.size(); ++i) {
            std::memcpy(
                &g_AircraftHeightLimitOriginalOperands[i],
                reinterpret_cast<const void*>(AIRCRAFT_HEIGHT_LIMIT_OPERANDS[i]),
                sizeof(g_AircraftHeightLimitOriginalOperands[i])
            );
        }
        g_bAircraftHeightLimitOperandsCaptured = true;
    }
    if (!g_bSunRenderOriginalOpcodesCaptured) {
        g_SunRenderOriginalOpcodes[0] = *reinterpret_cast<const uint8_t*>(SUN_RENDER_FUNCTION);
        g_SunRenderOriginalOpcodes[1] = *reinterpret_cast<const uint8_t*>(SUN_REFLECTION_FUNCTION);
        g_bSunRenderOriginalOpcodesCaptured = true;
    }
    ApplyAircraftHeightLimitOperandPointers();
    ApplySunEnabled(g_bSunEnabled);

    // Hook CClouds::VolumetricCloudsRender (0x716380)
    MH_CreateHook(
        reinterpret_cast<void*>(0x716380),
        reinterpret_cast<void*>(&Hooked_VolumetricCloudsRender),
        reinterpret_cast<void**>(&g_origVolumetricCloudsRender)
    );
    MH_EnableHook(reinterpret_cast<void*>(0x716380));

    // Hook CHud::DrawAmmo (0x5893B0)
    MH_CreateHook(
        reinterpret_cast<void*>(0x5893B0),
        reinterpret_cast<void*>(&Hooked_DrawAmmo),
        reinterpret_cast<void**>(&g_origDrawAmmo)
    );
    MH_EnableHook(reinterpret_cast<void*>(0x5893B0));

    // Hook CHud::DrawWeaponIcon (0x58D7D0)
    MH_CreateHook(
        reinterpret_cast<void*>(0x58D7D0),
        reinterpret_cast<void*>(&Hooked_DrawWeaponIcon),
        reinterpret_cast<void**>(&g_origDrawWeaponIcon)
    );
    MH_EnableHook(reinterpret_cast<void*>(0x58D7D0));

    // Hook CHud::RenderHealthBar (0x589270)
    MH_CreateHook(
        reinterpret_cast<void*>(0x589270),
        reinterpret_cast<void*>(&Hooked_RenderHealthBar),
        reinterpret_cast<void**>(&g_origRenderHealthBar)
    );
    MH_EnableHook(reinterpret_cast<void*>(0x589270));

    // Hook CHud::RenderBreathBar (0x589190)
    MH_CreateHook(
        reinterpret_cast<void*>(0x589190),
        reinterpret_cast<void*>(&Hooked_RenderBreathBar),
        reinterpret_cast<void**>(&g_origRenderBreathBar)
    );
    MH_EnableHook(reinterpret_cast<void*>(0x589190));

    // Hook CHud::RenderArmorBar (0x5890A0)
    MH_CreateHook(
        reinterpret_cast<void*>(0x5890A0),
        reinterpret_cast<void*>(&Hooked_RenderArmorBar),
        reinterpret_cast<void**>(&g_origRenderArmorBar)
    );
    MH_EnableHook(reinterpret_cast<void*>(0x5890A0));

    // Hook CHud::DrawRadar (0x58A330)
    MH_CreateHook(
        reinterpret_cast<void*>(0x58A330),
        reinterpret_cast<void*>(&Hooked_DrawRadar),
        reinterpret_cast<void**>(&g_origDrawRadar)
    );
    MH_EnableHook(reinterpret_cast<void*>(0x58A330));

    // Hook CHud::DrawCrossHairs (0x58E020)
    MH_CreateHook(
        reinterpret_cast<void*>(0x58E020),
        reinterpret_cast<void*>(&Hooked_DrawCrossHairs),
        reinterpret_cast<void**>(&g_origDrawCrossHairs)
    );
    MH_EnableHook(reinterpret_cast<void*>(0x58E020));

    // Write initial jetpack height limit to the GTA:SA global float (0x8703D8).
    // MTA-SA reference: VAR_fJetpackMaxHeight = 0x8703D8, MemPut<float>(0x8703D8, fHeight).
    // The game's CTaskSimpleJetPack::ProcessThrust reads this value directly.
    plugin::patch::SetFloat(0x8703D8, g_fJetpackHeightLimit);
    if (g_fJetpackHeightLimit > 0.0f && !s_bJetpackThrustPatched) {
        plugin::patch::SetRaw(0x67F26A, const_cast<uint8_t*>(s_patchThrustZeroOpcode), 3);
        s_bJetpackThrustPatched = true;
    }

    // Apply initial jetpack clump placement and rotation
    JETPACK_POS_OFFSET = g_vecJetpackOffset;
    JETPACK_ROT_AXIS   = g_vecJetpackRotAxis;
    plugin::patch::SetFloat(0x67EB01, g_fJetpackRotAngle);

    EnsureJetpackLoaded();

    g_bInitialized = true;
    return true;
}

static bool LoadJetpackDirectly() {
    constexpr int MODEL_JETPACK = 370;
    if (CStreaming::HasModelLoaded(MODEL_JETPACK)) {
        return true;
    }

    char gameDir[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, gameDir, MAX_PATH);
    char* lastSlash = strrchr(gameDir, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';

    std::string imgPath = std::string(gameDir) + "models\\gta3.img";
    FILE* fp = fopen(imgPath.c_str(), "rb");
    if (!fp) {
        ClientLog(LogLevel::Warning, std::format("LoadJetpackDirectly: failed to open {}", imgPath));
        return false;
    }

    char ver[4];
    uint32_t numEntries = 0;
    if (fread(ver, 1, 4, fp) != 4 || fread(&numEntries, 4, 1, fp) != 1) {
        fclose(fp);
        return false;
    }

    uint32_t dffOffset = 0, dffSectors = 0;
    uint32_t txdOffset = 0, txdSectors = 0;

    for (uint32_t i = 0; i < numEntries; ++i) {
        uint32_t offset = 0, size = 0;
        char name[24] = { 0 };
        if (fread(&offset, 4, 1, fp) != 1 ||
            fread(&size, 4, 1, fp) != 1 ||
            fread(name, 1, 24, fp) != 24) {
            break;
        }
        if (_stricmp(name, "jetpack.dff") == 0) {
            dffOffset = offset;
            dffSectors = size;
        } else if (_stricmp(name, "jetpack.txd") == 0) {
            txdOffset = offset;
            txdSectors = size;
        }
    }

    if (dffOffset == 0 || txdOffset == 0) {
        fclose(fp);
        ClientLog(LogLevel::Warning, "LoadJetpackDirectly: jetpack.dff or jetpack.txd not found in gta3.img directory");
        return false;
    }

    // 1. Read DFF clump from dffOffset * 2048
    fseek(fp, static_cast<long>(dffOffset * 2048), SEEK_SET);
    uint32_t dffHeader[3] = { 0 };
    if (fread(dffHeader, 4, 3, fp) != 3 || dffHeader[0] != 0x10) { // 0x10 = rwID_CLUMP
        fclose(fp);
        ClientLog(LogLevel::Warning, std::format("LoadJetpackDirectly: invalid DFF chunk type 0x{:X} at offset {}", dffHeader[0], dffOffset));
        return false;
    }
    uint32_t dffTotalSize = 12 + dffHeader[1];
    std::vector<uint8_t> dffBuffer(dffTotalSize);
    fseek(fp, static_cast<long>(dffOffset * 2048), SEEK_SET);
    if (fread(dffBuffer.data(), 1, dffTotalSize, fp) != dffTotalSize) {
        fclose(fp);
        ClientLog(LogLevel::Warning, "LoadJetpackDirectly: failed to read full DFF buffer");
        return false;
    }

    // 2. Read TXD
    // In gta3.img, scan for rwID_TEXDICTIONARY (0x16) chunk around txdOffset * 2048 (handling any sector overflow)
    fseek(fp, static_cast<long>(txdOffset * 2048), SEEK_SET);
    std::vector<uint8_t> scanBuf(4096);
    size_t scanRead = fread(scanBuf.data(), 1, scanBuf.size(), fp);
    uint32_t txdStartByteOffset = 0;
    bool bFoundTxd = false;

    for (size_t i = 0; i + 12 <= scanRead; ++i) {
        uint32_t type = *reinterpret_cast<uint32_t*>(&scanBuf[i]);
        uint32_t ver = *reinterpret_cast<uint32_t*>(&scanBuf[i + 8]);
        if (type == 0x16 && (ver == 0x1803FFFF || ver == 0x1C020037 || ver == 0x18035416)) {
            txdStartByteOffset = static_cast<uint32_t>(txdOffset * 2048 + i);
            bFoundTxd = true;
            break;
        }
    }

    if (!bFoundTxd) {
        fclose(fp);
        ClientLog(LogLevel::Warning, "LoadJetpackDirectly: could not find rwID_TEXDICTIONARY (0x16) chunk for jetpack.txd");
        return false;
    }

    fseek(fp, static_cast<long>(txdStartByteOffset), SEEK_SET);
    uint32_t txdHeader[3] = { 0 };
    if (fread(txdHeader, 4, 3, fp) != 3 || txdHeader[0] != 0x16) {
        fclose(fp);
        ClientLog(LogLevel::Warning, "LoadJetpackDirectly: failed to read TXD chunk header");
        return false;
    }
    uint32_t txdTotalSize = 12 + txdHeader[1];
    std::vector<uint8_t> txdBuffer(txdTotalSize);
    fseek(fp, static_cast<long>(txdStartByteOffset), SEEK_SET);
    if (fread(txdBuffer.data(), 1, txdTotalSize, fp) != txdTotalSize) {
        fclose(fp);
        ClientLog(LogLevel::Warning, "LoadJetpackDirectly: failed to read full TXD buffer");
        return false;
    }
    fclose(fp);

    // 3. Register and load TXD into CTxdStore
    int jetpackTxdSlot = CTxdStore::FindTxdSlot("jetpack");
    if (jetpackTxdSlot == -1) {
        jetpackTxdSlot = CTxdStore::AddTxdSlot("jetpack");
        ClientLog(LogLevel::Info, std::format("LoadJetpackDirectly: added jetpack.txd slot {}", jetpackTxdSlot));
    } else {
        ClientLog(LogLevel::Info, std::format("LoadJetpackDirectly: found existing jetpack.txd slot {}", jetpackTxdSlot));
    }

    RwMemory rwMemTxd{ txdBuffer.data(), static_cast<RwUInt32>(txdBuffer.size()) };
    RwStream* streamTxd = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &rwMemTxd);
    if (!streamTxd) {
        ClientLog(LogLevel::Warning, "LoadJetpackDirectly: RwStreamOpen failed for TXD");
        return false;
    }
    bool bTxdResult = CTxdStore::LoadTxd(jetpackTxdSlot, streamTxd);
    RwStreamClose(streamTxd, nullptr);
    CTxdStore::AddRef(jetpackTxdSlot);
    ClientLog(LogLevel::Info, std::format("LoadJetpackDirectly: CTxdStore::LoadTxd result: {}", bTxdResult ? "true" : "false"));

    // 4. Load DFF clump into CModelInfo
    CTxdStore::SetCurrentTxd(jetpackTxdSlot);
    auto* pModelInfo = CModelInfo::GetModelInfo(MODEL_JETPACK);
    if (pModelInfo) {
        pModelInfo->m_nTxdIndex = static_cast<int16_t>(jetpackTxdSlot);
    }

    RwMemory rwMemDff{ dffBuffer.data(), static_cast<RwUInt32>(dffBuffer.size()) };
    RwStream* streamDff = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &rwMemDff);
    if (!streamDff) {
        ClientLog(LogLevel::Warning, "LoadJetpackDirectly: RwStreamOpen failed for DFF");
        return false;
    }
    bool bClumpResult = CFileLoader::LoadClumpFile(streamDff, MODEL_JETPACK);
    RwStreamClose(streamDff, nullptr);
    ClientLog(LogLevel::Info, std::format("LoadJetpackDirectly: CFileLoader::LoadClumpFile result: {}", bClumpResult ? "true" : "false"));

    // 5. Mark loaded in CStreaming so CTaskSimpleJetPack recognizes it
    CStreaming::ms_aInfoForModel[MODEL_JETPACK].m_nLoadState = 1; // LOADSTATE_LOADED
    if (jetpackTxdSlot != -1) {
        CStreaming::ms_aInfoForModel[20000 + jetpackTxdSlot].m_nLoadState = 1;
    }

    bool bSuccess = CStreaming::HasModelLoaded(MODEL_JETPACK);
    ClientLog(LogLevel::Info, std::format("LoadJetpackDirectly: finished! HasModelLoaded(370)={}", bSuccess ? "true" : "false"));
    return bSuccess;
}

void EnsureJetpackLoaded() {
    constexpr int MODEL_JETPACK = 370;
    // Check if streaming subsystem is initialized (CStreaming::ms_bIsInitialised at 0x9654B8)
    const int* pStreamingInit = reinterpret_cast<const int*>(0x9654B8);
    if (!pStreamingInit || *pStreamingInit == 0) {
        return;
    }

    if (CStreaming::HasModelLoaded(MODEL_JETPACK)) {
        return;
    }

    static uint32_t s_lastAttempt = 0;
    uint32_t now = GetTickCount();
    if (now - s_lastAttempt < 3000) {
        return;
    }
    s_lastAttempt = now;

    LoadJetpackDirectly();
}

void Process() {
    if (!g_bInitialized) return;

    // Maintain jetpack model loaded for CTaskSimpleJetPack
    EnsureJetpackLoaded();

    // Maintain jetpack clump placement and orientation
    JETPACK_POS_OFFSET = g_vecJetpackOffset;
    JETPACK_ROT_AXIS   = g_vecJetpackRotAxis;

    // Strict ceiling enforcement for jetpack flight
    if (g_fJetpackHeightLimit > 0.0f && g_fJetpackHeightLimit < 50000.0f) {
        CPlayerPed* pLocal = FindPlayerPed(-1);
        if (pLocal && pLocal->m_pIntelligence) {
            CTask* pTask = pLocal->m_pIntelligence->m_TaskMgr.FindActiveTaskByType(TASK_SIMPLE_JETPACK);
            if (pTask) {
                float groundZ = TheCamera.CalculateGroundHeight(2);
                if (!std::isfinite(groundZ) || groundZ < -100.0f) {
                    groundZ = CWorld::FindGroundZForCoord(pLocal->GetPosition().x, pLocal->GetPosition().y);
                }
                float ceilingZ = groundZ + g_fJetpackHeightLimit;
                float currentZ = pLocal->GetPosition().z;

                if (currentZ >= ceilingZ) {
                    if (pLocal->m_vecMoveSpeed.z > 0.0f) {
                        pLocal->m_vecMoveSpeed.z = 0.0f;
                    }
                    if (currentZ > ceilingZ + 0.05f) {
                        pLocal->m_vecMoveSpeed.z = -0.05f;
                    }
                }
            }
        }
    }

    // 1. Maintain local player infinite sprint
    if (g_bInfiniteSprint) {
        uint8_t* pPlayerInfo = reinterpret_cast<uint8_t*>(0xB7CD98);
        if (pPlayerInfo) {
            *reinterpret_cast<bool*>(pPlayerInfo + 0x14C) = true;
        }

        CPlayerPed* pLocalPed = FindPlayerPed(-1);
        if (pLocalPed) {
            pLocalPed->ResetSprintEnergy();
        }
    }

    // 2. Synchronize bike knock off states for active peds
    for (const auto& kv : g_PlayerKnockOffBike) {
        uint16_t playerId = kv.first;
        uint8_t state = kv.second;

        CPed* pPed = GetGamePed(playerId);
        if (pPed) {
            pPed->CantBeKnockedOffBike = state;
        }
    }

    // 3. Synchronize fireproof states for active peds
    for (const auto& kv : g_PlayerFireProof) {
        uint16_t playerId = kv.first;
        bool bFireProof = kv.second;

        CPed* pPed = GetGamePed(playerId);
        if (pPed) {
            pPed->bFireProof = bFireProof;
            if (IsLocalPlayerPed(pPed)) {
                uint8_t* pPlayerInfo = reinterpret_cast<uint8_t*>(0xB7CD98);
                if (pPlayerInfo) {
                    *reinterpret_cast<bool*>(pPlayerInfo + 0x14E) = bFireProof;
                }
            }
        }
    }

    // 4. Handle Sniper Laser Dot rendering when aiming
    if (g_nHUDComponents & HUD_COMPONENT_LASERDOT) {
        CPlayerPed* pLocal = FindPlayerPed(-1);
        if (pLocal) {
            uint8_t activeSlot = pLocal->m_nSelectedWepSlot;
            if (activeSlot < 13) {
                CWeapon* pWeapon = &pLocal->m_aWeapons[activeSlot];
                if (pWeapon && pWeapon->m_eWeaponType == WEAPONTYPE_SNIPERRIFLE) {
                    CPad* pPad = pLocal->GetPadFromPlayer();
                    if (pPad && pPad->GetTarget()) {
                        CVector vecCoord;
                        float fSize = 0.0f;
                        pWeapon->LaserScopeDot(&vecCoord, &fSize);
                    }
                }
            }
        }
    }

    // 5. Maintain active rainbow if enabled
    if (g_bRainbowEnabled) {
        float* pRainbow = reinterpret_cast<float*>(0xC812E4);
        if (pRainbow && *pRainbow < 0.5f) {
            *pRainbow = 1.0f;
        }
    }
}

void Shutdown() {
    if (!g_bInitialized) return;

    MH_DisableHook(reinterpret_cast<void*>(0x6EAE80));
    MH_DisableHook(reinterpret_cast<void*>(0x6EB710));
    MH_DisableHook(reinterpret_cast<void*>(0x716380));
    MH_DisableHook(reinterpret_cast<void*>(0x5893B0));
    MH_DisableHook(reinterpret_cast<void*>(0x58D7D0));
    MH_DisableHook(reinterpret_cast<void*>(0x589270));
    MH_DisableHook(reinterpret_cast<void*>(0x589190));
    MH_DisableHook(reinterpret_cast<void*>(0x5890A0));
    MH_DisableHook(reinterpret_cast<void*>(0x58A330));
    MH_DisableHook(reinterpret_cast<void*>(0x58E020));

    if (g_bAircraftHeightLimitOperandsCaptured) {
        for (size_t i = 0; i < AIRCRAFT_HEIGHT_LIMIT_OPERANDS.size(); ++i) {
            plugin::patch::SetRaw(
                AIRCRAFT_HEIGHT_LIMIT_OPERANDS[i],
                &g_AircraftHeightLimitOriginalOperands[i],
                sizeof(g_AircraftHeightLimitOriginalOperands[i])
            );
        }
        g_bAircraftHeightLimitOperandsCaptured = false;
    }
    ApplySunEnabled(true);
    g_bSunRenderOriginalOpcodesCaptured = false;

    // Restore original jetpack max height global (100.0f default)
    plugin::patch::SetFloat(0x8703D8, 100.0f);
    if (s_bJetpackThrustPatched) {
        plugin::patch::SetRaw(0x67F26A, const_cast<uint8_t*>(s_origThrustDampingOpcode), 3);
        s_bJetpackThrustPatched = false;
    }

    // Restore vanilla jetpack clump placement and rotation
    JETPACK_POS_OFFSET = CVector(0.1f, 0.08f, 0.0f);
    JETPACK_ROT_AXIS   = CVector(0.0f, 1.0f, 0.0f);
    plugin::patch::SetFloat(0x67EB01, 90.0f);

    ClearAllStates();
    RestoreWaterWaveHeight();
    MH_RemoveHook(reinterpret_cast<void*>(0x6EAE80));
    MH_RemoveHook(reinterpret_cast<void*>(0x6EB710));
    g_origWaterLevelInitialise = nullptr;
    g_origPreRenderWater = nullptr;
    g_bInitialized = false;
}

void SetAircraftHeightLimit(float limit) {
    if (!std::isfinite(limit) || limit < 0.0f) {
        ClientLog(LogLevel::Warning, std::format("SetAircraftHeightLimit: rejected invalid limit {:.1f}", limit));
        return;
    }

    g_fAircraftHeightLimit = limit;
    g_fAircraftHeightLimitForGame = limit <= 0.0f ? 100000.0f : limit;
    ApplyAircraftHeightLimitOperandPointers();
}

float GetAircraftHeightLimit() {
    return g_fAircraftHeightLimit;
}

void SetJetpackHeightLimit(float limit) {
    if (!std::isfinite(limit) || limit < 0.0f) {
        ClientLog(LogLevel::Warning, std::format("SetJetpackHeightLimit: rejected invalid limit {:.1f}", limit));
        return;
    }

    g_fJetpackHeightLimit = limit;

    if (limit <= 0.0f) {
        // Unlimited: write a very large value so the ceiling is effectively disabled.
        // 100000.0f is safely above the world bounds; the game will never clamp thrust.
        plugin::patch::SetFloat(0x8703D8, 100000.0f);
        if (s_bJetpackThrustPatched) {
            plugin::patch::SetRaw(0x67F26A, const_cast<uint8_t*>(s_origThrustDampingOpcode), 3);
            s_bJetpackThrustPatched = false;
        }
        ClientLog(LogLevel::Info, "SetJetpackHeightLimit: unlimited (writing 100000.0f to 0x8703D8, restored thrust damping)");
    } else {
        // Set the global jetpack max height directly.
        // MTA-SA equivalent: MemPut<float>(VAR_fJetpackMaxHeight, limit);
        plugin::patch::SetFloat(0x8703D8, limit);
        if (!s_bJetpackThrustPatched) {
            plugin::patch::SetRaw(0x67F26A, const_cast<uint8_t*>(s_patchThrustZeroOpcode), 3);
            s_bJetpackThrustPatched = true;
        }
        ClientLog(LogLevel::Info, std::format("SetJetpackHeightLimit: limit={:.1f} (written to 0x8703D8, zero thrust patch active)", limit));
    }
}

float GetJetpackHeightLimit() {
    return g_fJetpackHeightLimit;
}

void SetJetpackOffset(float x, float y, float z) {
    g_vecJetpackOffset = { x, y, z };
    JETPACK_POS_OFFSET = g_vecJetpackOffset;
    ClientLog(LogLevel::Info, std::format("SetJetpackOffset: X={:.3f}, Y={:.3f}, Z={:.3f}", x, y, z));
}

void GetJetpackOffset(float& x, float& y, float& z) {
    x = g_vecJetpackOffset.x;
    y = g_vecJetpackOffset.y;
    z = g_vecJetpackOffset.z;
}

void SetJetpackRotation(float angle, float axisX, float axisY, float axisZ) {
    g_fJetpackRotAngle = angle;
    g_vecJetpackRotAxis = { axisX, axisY, axisZ };
    JETPACK_ROT_AXIS = g_vecJetpackRotAxis;
    plugin::patch::SetFloat(0x67EB01, angle);
    ClientLog(LogLevel::Info, std::format("SetJetpackRotation: Angle={:.1f} deg, Axis=({:.2f}, {:.2f}, {:.2f})", angle, axisX, axisY, axisZ));
}

void GetJetpackRotation(float& angle, float& axisX, float& axisY, float& axisZ) {
    angle = g_fJetpackRotAngle;
    axisX = g_vecJetpackRotAxis.x;
    axisY = g_vecJetpackRotAxis.y;
    axisZ = g_vecJetpackRotAxis.z;
}

void ResetJetpackOffset() {
    SetJetpackOffset(0.10f, -0.38f, 0.00f);
    SetJetpackRotation(90.0f, 0.0f, 1.0f, 0.0f);
    ClientLog(LogLevel::Info, "ResetJetpackOffset: reset to umbrella-tuned default");
}

void SetPlayerKnockedOffBikeState(uint16_t playerId, uint8_t knockState) {
    g_PlayerKnockOffBike[playerId] = knockState;

    CPed* pPed = GetGamePed(playerId);
    if (pPed) {
        pPed->CantBeKnockedOffBike = knockState;
    }
}

uint8_t GetPlayerKnockedOffBikeState(uint16_t playerId) {
    auto it = g_PlayerKnockOffBike.find(playerId);
    if (it != g_PlayerKnockOffBike.end()) {
        return it->second;
    }
    return BIKE_KNOCK_OFF_DEFAULT;
}

void SetPlayerFireProof(uint16_t playerId, bool enable) {
    g_PlayerFireProof[playerId] = enable;

    CPed* pPed = GetGamePed(playerId);
    if (pPed) {
        pPed->bFireProof = enable;
        if (IsLocalPlayerPed(pPed)) {
            uint8_t* pPlayerInfo = reinterpret_cast<uint8_t*>(0xB7CD98);
            if (pPlayerInfo) {
                *reinterpret_cast<bool*>(pPlayerInfo + 0x14E) = enable;
            }
        }
    }
}

bool IsPlayerFireProof(uint16_t playerId) {
    auto it = g_PlayerFireProof.find(playerId);
    if (it != g_PlayerFireProof.end()) {
        return it->second;
    }
    return false;
}

void TogglePlayerInfiniteSprint(bool enable) {
    g_bInfiniteSprint = enable;

    uint8_t* pPlayerInfo = reinterpret_cast<uint8_t*>(0xB7CD98);
    if (pPlayerInfo) {
        *reinterpret_cast<bool*>(pPlayerInfo + 0x14C) = enable;
    }

    if (enable) {
        CPlayerPed* pLocalPed = FindPlayerPed(-1);
        if (pLocalPed) {
            pLocalPed->ResetSprintEnergy();
        }
    }
}

bool IsInfiniteSprintEnabled() {
    return g_bInfiniteSprint;
}

void TogglePlayerSun(bool enable) {
    g_bSunEnabled = enable;
    ApplySunEnabled(enable);
}

bool IsSunEnabled() {
    return g_bSunEnabled;
}

void TogglePlayerMoon(bool enable) {
    g_bMoonEnabled = enable;
    ApplyMoonPatch(enable);
}

bool IsMoonEnabled() {
    return g_bMoonEnabled;
}

void TogglePlayerStars(bool enable) {
    g_bStarsEnabled = enable;
    ApplyStarsPatch(enable);
}

bool IsStarsEnabled() {
    return g_bStarsEnabled;
}

void TogglePlayerLowClouds(bool enable) {
    g_bLowCloudsEnabled = enable;
    ApplyLowCloudsPatch(enable);
}

bool IsLowCloudsEnabled() {
    return g_bLowCloudsEnabled;
}

void TogglePlayerFluffyClouds(bool enable) {
    g_bFluffyCloudsEnabled = enable;
}

bool IsFluffyCloudsEnabled() {
    return g_bFluffyCloudsEnabled;
}

void TogglePlayerRainbow(bool enable) {
    g_bRainbowEnabled = enable;
    ApplyRainbowPatch(enable);
    *reinterpret_cast<float*>(0xC812E4) = enable ? 1.0f : 0.0f;
}

bool IsRainbowEnabled() {
    return g_bRainbowEnabled;
}

bool SetWaterLevel(float level, bool includeWorldNonSeaLevel, bool includeWorldSeaLevel, bool includeOutsideWorldLevel) {
    if (!std::isfinite(level)) {
        return false;
    }

    if ((includeWorldNonSeaLevel || includeWorldSeaLevel) && !CaptureWorldWaterDefaults()) {
        ClientLog(LogLevel::Debug, "Water level update deferred: GTA world water vertices are not initialized");
        return false;
    }

    g_fWaterLevel = level;
    g_bWaterLevelOverrideEnabled =
        includeWorldNonSeaLevel || includeWorldSeaLevel || includeOutsideWorldLevel;
    g_bIncludeWorldNonSeaWater = includeWorldNonSeaLevel;
    g_bIncludeWorldSeaWater = includeWorldSeaLevel;
    g_bIncludeOutsideWorldWater = includeOutsideWorldLevel;
    ApplyWaterLevelOverride();

    ClientLog(LogLevel::Info, std::format(
        "World water level set to {:.2f} m (non-sea={}, sea={}, outside={})",
        level,
        static_cast<unsigned>(includeWorldNonSeaLevel),
        static_cast<unsigned>(includeWorldSeaLevel),
        static_cast<unsigned>(includeOutsideWorldLevel)
    ));
    return true;
}

bool ResetWaterLevel() {
    const bool worldVerticesAvailable = CaptureWorldWaterDefaults();
    if (worldVerticesAvailable) {
        for (size_t i = 0; i < WATER_WORLD_VERTEX_COUNT; ++i) {
            CWaterLevel::m_aVertices[i].rp.z = g_WorldWaterDefaultLevels[i];
        }
    }

    SetOutsideWorldWaterLevel(0.0f);
    g_fWaterLevel = 0.0f;
    g_bWaterLevelOverrideEnabled = false;
    g_bIncludeWorldNonSeaWater = false;
    g_bIncludeWorldSeaWater = false;
    g_bIncludeOutsideWorldWater = false;
    SetWaterRenderOrderEnabled(false);
    return worldVerticesAvailable;
}

float GetWaterLevel() {
    return g_fWaterLevel;
}

bool SetWaterWaveHeight(float height) {
    if (!std::isfinite(height) || height < -1.0f || height > 100.0f) {
        return false;
    }

    if (height < 0.0f) {
        g_bRestoreNativeWaterWaves = true;
        RestoreWaterWaveHeight();
    } else {
        g_bRestoreNativeWaterWaves = false;
        g_fRequestedWaterWaveHeight = height;
        ApplyWaterWaveHeight(height * WATER_WAVE_PERCENT_SCALE);
    }
    ClientLog(LogLevel::Info, std::format(
        "Water wave height set to {:.2f}% of normal waviness (game multiplier={:.2f})",
        height < 0.0f ? 60.0f : height,
        g_fWaterWaveHeight
    ));
    return true;
}

float GetWaterWaveHeight() {
    return g_bRestoreNativeWaterWaves ? 0.6f : g_fRequestedWaterWaveHeight;
}

void TogglePlayerHUDComponents(uint32_t hudComponents) {
    g_nHUDComponents = hudComponents;

    // Apply patch for money drawing based on HUD_COMPONENT_MONEY bit
    ApplyMoneyPatch((hudComponents & HUD_COMPONENT_MONEY) != 0);
}

uint32_t GetPlayerHUDComponents() {
    return g_nHUDComponents;
}

void SetPlayerHUDComponentsColour(uint32_t hudComponents, uint32_t intColour) {
    if (hudComponents & HUD_COMPONENT_AMMO) {
        g_HUDColours.ammoColor = intColour;
    }
    if (hudComponents & HUD_COMPONENT_WEAPONICON) {
        g_HUDColours.weaponIconColor = intColour;
    }
    if (hudComponents & HUD_COMPONENT_HEALTH) {
        g_HUDColours.healthColor = intColour;
    }
    if (hudComponents & HUD_COMPONENT_OXYGEN) {
        g_HUDColours.oxygenColor = intColour;
    }
    if (hudComponents & HUD_COMPONENT_ARMOUR) {
        g_HUDColours.armourColor = intColour;
    }
    if (hudComponents & HUD_COMPONENT_RADAR) {
        g_HUDColours.radarColor = intColour;
    }
    if (hudComponents & HUD_COMPONENT_CROSSHAIR) {
        g_HUDColours.crosshairColor = intColour;
    }
    if (hudComponents & HUD_COMPONENT_MONEY) {
        g_HUDColours.moneyColor = intColour;
        if (intColour != 0) {
            HudColour.m_aColours[HUD_COLOUR_GREEN].red   = static_cast<uint8_t>((intColour >> 24) & 0xFF);
            HudColour.m_aColours[HUD_COLOUR_GREEN].green = static_cast<uint8_t>((intColour >> 16) & 0xFF);
            HudColour.m_aColours[HUD_COLOUR_GREEN].blue  = static_cast<uint8_t>((intColour >> 8) & 0xFF);
            HudColour.m_aColours[HUD_COLOUR_GREEN].alpha = static_cast<uint8_t>(intColour & 0xFF);
        } else {
            // Restore default Green
            HudColour.m_aColours[HUD_COLOUR_GREEN].red   = 54;
            HudColour.m_aColours[HUD_COLOUR_GREEN].green = 104;
            HudColour.m_aColours[HUD_COLOUR_GREEN].blue  = 44;
            HudColour.m_aColours[HUD_COLOUR_GREEN].alpha = 255;
        }
    }
    if (hudComponents & HUD_COMPONENT_LASERDOT) {
        g_HUDColours.laserDotColor = intColour;
        ApplyLaserDotColorPatch(intColour);
    }
}

uint32_t GetPlayerHUDComponentColour(uint32_t hudComponent) {
    switch (hudComponent) {
    case HUD_COMPONENT_AMMO:       return g_HUDColours.ammoColor;
    case HUD_COMPONENT_WEAPONICON: return g_HUDColours.weaponIconColor;
    case HUD_COMPONENT_HEALTH:     return g_HUDColours.healthColor;
    case HUD_COMPONENT_OXYGEN:     return g_HUDColours.oxygenColor;
    case HUD_COMPONENT_ARMOUR:     return g_HUDColours.armourColor;
    case HUD_COMPONENT_RADAR:      return g_HUDColours.radarColor;
    case HUD_COMPONENT_CROSSHAIR:  return g_HUDColours.crosshairColor;
    case HUD_COMPONENT_MONEY:      return g_HUDColours.moneyColor;
    case HUD_COMPONENT_LASERDOT:   return g_HUDColours.laserDotColor;
    default: break;
    }
    return 0;
}

void ClearAllStates() {
    g_fAircraftHeightLimit = 800.0f;
    g_fAircraftHeightLimitForGame = 800.0f;
    ApplyAircraftHeightLimitOperandPointers();
    g_fJetpackHeightLimit  = 100.0f;
    g_bInfiniteSprint       = false;
    g_bSunEnabled           = true;
    ApplySunEnabled(true);

    g_bMoonEnabled          = true;
    g_bStarsEnabled         = true;
    g_bLowCloudsEnabled     = true;
    g_bFluffyCloudsEnabled  = true;
    g_bRainbowEnabled       = true;
    ResetWaterLevel();
    g_bRestoreNativeWaterWaves = false;
    g_fRequestedWaterWaveHeight = 0.0f;
    ApplyWaterWaveHeight(0.0f);

    g_nHUDComponents        = HUD_COMPONENT_ALL;
    g_HUDColours            = HUDColoursOverride();

    ApplyMoonPatch(true);
    ApplyStarsPatch(true);
    ApplyLowCloudsPatch(true);
    ApplyRainbowPatch(true);
    ApplyMoneyPatch(true);
    ApplyLaserDotColorPatch(0);

    // Restore default Green for money
    HudColour.m_aColours[HUD_COLOUR_GREEN].red   = 54;
    HudColour.m_aColours[HUD_COLOUR_GREEN].green = 104;
    HudColour.m_aColours[HUD_COLOUR_GREEN].blue  = 44;
    HudColour.m_aColours[HUD_COLOUR_GREEN].alpha = 255;

    // Restore jetpack max height to GTA:SA default (100.0f)
    plugin::patch::SetFloat(0x8703D8, 100.0f);

    uint8_t* pPlayerInfo = reinterpret_cast<uint8_t*>(0xB7CD98);
    if (pPlayerInfo) {
        *reinterpret_cast<bool*>(pPlayerInfo + 0x14C) = false;
        *reinterpret_cast<bool*>(pPlayerInfo + 0x14E) = false;
    }

    g_PlayerKnockOffBike.clear();
    g_PlayerFireProof.clear();
}

}
