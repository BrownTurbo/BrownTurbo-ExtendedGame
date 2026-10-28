#include "textdraw_rotation.h"

#include "utils.h"

#include <MinHook.h>
#include <RakHook/rakhook.hpp>
#include <RenderWare.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <sampapi/CNetGame.h>
#include <sampapi/CTextDraw.h>
#include <sampapi/CFonts.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <numbers>
#include <vector>

namespace TextDrawRotation{
namespace {

constexpr uint16_t MAX_GLOBAL_TEXTDRAWS = 2048;
constexpr uint16_t MAX_LOCAL_TEXTDRAWS = 256;
constexpr size_t MAX_TEXTDRAW_SLOTS = MAX_GLOBAL_TEXTDRAWS + MAX_LOCAL_TEXTDRAWS;
constexpr float MAX_ABSOLUTE_ANGLE = 1000000.0f;
constexpr float MAX_ABSOLUTE_SPIN_SPEED = 36000.0f;
constexpr size_t STACK_VERTICES_LIMIT = 256;

struct RotationState {
    bool enabled = false;
    float angle = 0.0f;
    float pivotX = -1.0f; // < 0 means auto-pivot at textdraw center
    float pivotY = -1.0f; // < 0 means auto-pivot at textdraw center
    float spinSpeed = 0.0f; // degrees per second
    ULONGLONG startTimeMs = 0;
};

struct ActiveRotationContext {
    float cosA = 1.0f;
    float sinA = 0.0f;
    float pivotX = 0.0f;
    float pivotY = 0.0f;
    bool hasExplicitPivot = false;
    bool hasCalculatedPivot = false;
};

using TextDrawDraw_t = void(__thiscall*)(void*);
using TextDrawPoolDelete_t = void(__thiscall*)(void*, uint16_t);
using TextDrawPoolDraw_t = void(__thiscall*)(void*);

std::array<RotationState, MAX_TEXTDRAW_SLOTS> g_RotationStates{};
std::mutex g_RotationStatesMutex;

TextDrawDraw_t g_OriginalTextDrawDraw = nullptr;
uintptr_t g_TextDrawDrawAddress = 0;

TextDrawPoolDelete_t g_OriginalTextDrawPoolDelete = nullptr;
uintptr_t g_TextDrawPoolDeleteAddress = 0;

TextDrawPoolDraw_t g_OriginalTextDrawPoolDraw = nullptr;
uintptr_t g_TextDrawPoolDrawAddress = 0;

void* g_pTextDrawPool = nullptr;

RwIm2DRenderPrimitiveFunction g_OriginalIm2DRenderPrimitive = nullptr;
RwIm2DRenderIndexedPrimitiveFunction g_OriginalIm2DRenderIndexedPrimitive = nullptr;
void* g_pTargetIm2DRenderPrimitive = nullptr;
void* g_pTargetIm2DRenderIndexedPrimitive = nullptr;
RwIm2DRenderPrimitiveFunction g_RealOriginalIm2DPrimitive = nullptr;
RwIm2DRenderIndexedPrimitiveFunction g_RealOriginalIm2DIndexedPrimitive = nullptr;
bool g_RwIm2DHookInstalled = false;

ActiveRotationContext* g_pActiveRotation = nullptr;

bool g_Initialized = false;
bool g_HookInstallationAttempted = false;

// Forward declarations
void __fastcall HookedTextDrawDraw(void* textDraw, void* edx);
void __fastcall HookedTextDrawPoolDelete(void* pool, void* edx, uint16_t nId);
void __fastcall HookedTextDrawPoolDraw(void* pool, void* edx);
RwBool Hooked_Im2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex* vertices, RwInt32 numVertices);
RwBool Hooked_Im2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex* vertices, RwInt32 numVertices, RwImVertexIndex* indices, RwInt32 numIndices);

void* GetTextDrawPool() {
    switch (rakhook::samp_version()) {
    case rakhook::samp_ver::v037r1: {
        auto* net = sampapi::v037r1::RefNetGame();
        return (net && net->m_pPools) ? net->m_pPools->m_pTextDraw : nullptr;
    }
    case rakhook::samp_ver::v037r31: {
        auto* net = sampapi::v037r3::RefNetGame();
        return (net && net->m_pPools) ? net->m_pPools->m_pTextDraw : nullptr;
    }
    case rakhook::samp_ver::v037r5: {
        auto* net = sampapi::v037r5::RefNetGame();
        return (net && net->m_pPools) ? net->m_pPools->m_pTextDraw : nullptr;
    }
    case rakhook::samp_ver::v03dlr1: {
        auto* net = sampapi::v03dl::RefNetGame();
        return (net && net->m_pPools) ? net->m_pPools->m_pTextDraw : nullptr;
    }
    default:
        return nullptr;
    }
}

uintptr_t GetTextDrawDrawAddress() {
    const uintptr_t sampBase = rakhook::samp_addr();
    if (!sampBase) {
        return 0;
    }

    uintptr_t offset = 0;
    switch (rakhook::samp_version()) {
    case rakhook::samp_ver::v037r1:
        offset = 0xACD90;
        break;
    case rakhook::samp_ver::v037r31:
        offset = 0xB2BF0;
        break;
    case rakhook::samp_ver::v037r5:
        offset = 0xB3480;
        break;
    case rakhook::samp_ver::v03dlr1:
        offset = 0xB3080;
        break;
    default:
        return 0;
    }

    return sampBase + offset;
}

uintptr_t GetTextDrawPoolDeleteAddress() {
    const uintptr_t sampBase = rakhook::samp_addr();
    if (!sampBase) {
        return 0;
    }

    uintptr_t offset = 0;
    switch (rakhook::samp_version()) {
    case rakhook::samp_ver::v037r1:
        offset = 0x1AD00;
        break;
    case rakhook::samp_ver::v037r31:
        offset = 0x1E0A0;
        break;
    case rakhook::samp_ver::v037r5:
        offset = 0x1E7F0;
        break;
    case rakhook::samp_ver::v03dlr1:
        offset = 0x1E2B0;
        break;
    default:
        return 0;
    }

    return sampBase + offset;
}

uintptr_t GetTextDrawPoolDrawAddress() {
    const uintptr_t sampBase = rakhook::samp_addr();
    if (!sampBase) {
        return 0;
    }

    uintptr_t offset = 0;
    switch (rakhook::samp_version()) {
    case rakhook::samp_ver::v037r1:
        offset = 0x1AD40;
        break;
    case rakhook::samp_ver::v037r31:
        offset = 0x1E0E0;
        break;
    case rakhook::samp_ver::v037r5:
        offset = 0x1E830;
        break;
    case rakhook::samp_ver::v03dlr1:
        offset = 0x1E2F0;
        break;
    default:
        return 0;
    }

    return sampBase + offset;
}

uint32_t GetTextDrawIndex(void* textDraw) {
    if (!textDraw) return static_cast<uint32_t>(MAX_TEXTDRAW_SLOTS);

    void* pPool = g_pTextDrawPool;
    if (!pPool) {
        pPool = GetTextDrawPool();
        if (pPool) {
            g_pTextDrawPool = pPool;
        }
    }
    if (!pPool) {
        return static_cast<uint32_t>(MAX_TEXTDRAW_SLOTS);
    }

    // In SA-MP CTextDrawPool across 0.3.7-R1, 0.3.7-R3, 0.3.7-R5, and 0.3.DL:
    // BOOL m_bNotEmpty[2304] at offset 0x0000 (2304 * 4 = 0x2400 bytes)
    // CTextDraw* m_pObject[2304] at offset 0x2400 (2304 * 4 = 0x2400 bytes)
    void** objects = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(pPool) + 0x2400);
    for (uint32_t i = 0; i < MAX_TEXTDRAW_SLOTS; ++i) {
        if (objects[i] == textDraw) {
            return i;
        }
    }
    return static_cast<uint32_t>(MAX_TEXTDRAW_SLOTS);
}

static inline void CallCFontDrawFonts() {
    // 0x71A210 is CFont::DrawFonts() in GTA:SA 1.0 US (flushes buffered characters)
    reinterpret_cast<void(__cdecl*)()>(0x71A210)();
}

bool InstallRenderWareHooks() {
    if (g_RwIm2DHookInstalled) {
        return true;
    }

    if (!RwEngineInstance) {
        return false;
    }

    auto* rwGlobals = reinterpret_cast<RwGlobals*>(RwEngineInstance);
    auto origPrim = rwGlobals->dOpenDevice.fpIm2DRenderPrimitive;
    auto origIndexed = rwGlobals->dOpenDevice.fpIm2DRenderIndexedPrimitive;

    if (!origPrim || !origIndexed) {
        return false;
    }

    g_RealOriginalIm2DPrimitive = origPrim;
    g_RealOriginalIm2DIndexedPrimitive = origIndexed;
    g_pTargetIm2DRenderPrimitive = reinterpret_cast<void*>(origPrim);
    g_pTargetIm2DRenderIndexedPrimitive = reinterpret_cast<void*>(origIndexed);
    g_OriginalIm2DRenderPrimitive = origPrim;
    g_OriginalIm2DRenderIndexedPrimitive = origIndexed;

    MH_STATUS s1 = MH_CreateHook(
        g_pTargetIm2DRenderPrimitive,
        reinterpret_cast<void*>(&Hooked_Im2DRenderPrimitive),
        reinterpret_cast<void**>(&g_OriginalIm2DRenderPrimitive)
    );
    if (s1 == MH_OK) {
        MH_EnableHook(g_pTargetIm2DRenderPrimitive);
    }

    MH_STATUS s2 = MH_CreateHook(
        g_pTargetIm2DRenderIndexedPrimitive,
        reinterpret_cast<void*>(&Hooked_Im2DRenderIndexedPrimitive),
        reinterpret_cast<void**>(&g_OriginalIm2DRenderIndexedPrimitive)
    );
    if (s2 == MH_OK) {
        MH_EnableHook(g_pTargetIm2DRenderIndexedPrimitive);
    }

    // Also update function pointers in dOpenDevice table
    rwGlobals->dOpenDevice.fpIm2DRenderPrimitive = &Hooked_Im2DRenderPrimitive;
    rwGlobals->dOpenDevice.fpIm2DRenderIndexedPrimitive = &Hooked_Im2DRenderIndexedPrimitive;

    g_RwIm2DHookInstalled = true;
    ClientLog(LogLevel::Info, "Installed RenderWare Im2D rotation hooks");
    return true;
}

bool InstallSampTextDrawHooks() {
    const uintptr_t drawTarget = GetTextDrawDrawAddress();
    if (!drawTarget) {
        ClientLog(LogLevel::Warning, std::format("TextDraw rotation is unsupported for SA-MP version {}",
            GetSampVersion()));
        return false;
    }

    const MH_STATUS drawCreateStatus = MH_CreateHook(
        reinterpret_cast<void*>(drawTarget),
        reinterpret_cast<void*>(&HookedTextDrawDraw),
        reinterpret_cast<void**>(&g_OriginalTextDrawDraw)
    );
    if (drawCreateStatus != MH_OK) {
        ClientLog(LogLevel::Error, std::format("Failed to create TextDraw::Draw hook (status={})",
            static_cast<int>(drawCreateStatus)));
        return false;
    }

    const MH_STATUS drawEnableStatus = MH_EnableHook(reinterpret_cast<void*>(drawTarget));
    if (drawEnableStatus != MH_OK) {
        MH_RemoveHook(reinterpret_cast<void*>(drawTarget));
        g_OriginalTextDrawDraw = nullptr;
        ClientLog(LogLevel::Error, std::format("Failed to enable TextDraw::Draw hook (status={})",
            static_cast<int>(drawEnableStatus)));
        return false;
    }

    g_TextDrawDrawAddress = drawTarget;
    ClientLog(LogLevel::Info, std::format("TextDraw::Draw hook installed at 0x{:X}",
        static_cast<unsigned int>(drawTarget)));

    // Install CTextDrawPool::Delete hook to clean up state when textdraw is destroyed
    const uintptr_t deleteTarget = GetTextDrawPoolDeleteAddress();
    if (deleteTarget) {
        const MH_STATUS delCreateStatus = MH_CreateHook(
            reinterpret_cast<void*>(deleteTarget),
            reinterpret_cast<void*>(&HookedTextDrawPoolDelete),
            reinterpret_cast<void**>(&g_OriginalTextDrawPoolDelete)
        );
        if (delCreateStatus == MH_OK) {
            MH_EnableHook(reinterpret_cast<void*>(deleteTarget));
            g_TextDrawPoolDeleteAddress = deleteTarget;
            ClientLog(LogLevel::Info, std::format("CTextDrawPool::Delete hook installed at 0x{:X}",
                static_cast<unsigned int>(deleteTarget)));
        }
    }

    // Install CTextDrawPool::Draw hook to track pool pointer and ensure it is valid
    const uintptr_t poolDrawTarget = GetTextDrawPoolDrawAddress();
    if (poolDrawTarget) {
        const MH_STATUS poolDrawStatus = MH_CreateHook(
            reinterpret_cast<void*>(poolDrawTarget),
            reinterpret_cast<void*>(&HookedTextDrawPoolDraw),
            reinterpret_cast<void**>(&g_OriginalTextDrawPoolDraw)
        );
        if (poolDrawStatus == MH_OK) {
            MH_EnableHook(reinterpret_cast<void*>(poolDrawTarget));
            g_TextDrawPoolDrawAddress = poolDrawTarget;
            ClientLog(LogLevel::Info, std::format("CTextDrawPool::Draw hook installed at 0x{:X}",
                static_cast<unsigned int>(poolDrawTarget)));
        }
    }

    return true;
}

void __fastcall HookedTextDrawPoolDraw(void* pool, void*) {
    g_pTextDrawPool = pool;
    if (g_OriginalTextDrawPoolDraw) {
        g_OriginalTextDrawPoolDraw(pool);
    }
}

void __fastcall HookedTextDrawPoolDelete(void* pool, void*, uint16_t nId) {
    g_pTextDrawPool = pool;
    TextDrawRotation::Reset(nId);
    if (g_OriginalTextDrawPoolDelete) {
        g_OriginalTextDrawPoolDelete(pool, nId);
    }
}

void __fastcall HookedTextDrawDraw(void* textDraw, void*) {
    if (!g_OriginalTextDrawDraw) {
        return;
    }

    const uint32_t index = GetTextDrawIndex(textDraw);
    if (index >= g_RotationStates.size()) {
        g_OriginalTextDrawDraw(textDraw);
        return;
    }

    RotationState state;
    {
        const std::lock_guard<std::mutex> lock(g_RotationStatesMutex);
        state = g_RotationStates[index];
    }

    if (!state.enabled) {
        g_OriginalTextDrawDraw(textDraw);
        return;
    }

    // Flush any previously buffered text before starting this rotated textdraw
    CallCFontDrawFonts();

    // Compute animated angle
    const ULONGLONG elapsedMs = GetTickCount64() - state.startTimeMs;
    const float angleDegrees = std::fmod(
        state.angle + state.spinSpeed * (static_cast<float>(elapsedMs) * 0.001f),
        360.0f
    );
    constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;
    const float angleRadians = angleDegrees * DEG_TO_RAD;

    // Get screen dimensions from GTA globals (RsGlobal.maximumWidth / maximumHeight)
    float screenWidth = static_cast<float>(*reinterpret_cast<const int*>(0xC17044));
    float screenHeight = static_cast<float>(*reinterpret_cast<const int*>(0xC17048));
    if (screenWidth <= 0.0f || screenHeight <= 0.0f) {
        screenWidth = 640.0f;
        screenHeight = 480.0f;
    }

    ActiveRotationContext activeCtx{};
    activeCtx.cosA = std::cos(angleRadians);
    activeCtx.sinA = std::sin(angleRadians);

    if (state.pivotX >= 0.0f && state.pivotY >= 0.0f) {
        activeCtx.hasExplicitPivot = true;
        activeCtx.hasCalculatedPivot = true;
        if (state.pivotX <= 1.0f && state.pivotY <= 1.0f) {
            // Normalized screen coordinates (0.0 to 1.0)
            activeCtx.pivotX = state.pivotX * screenWidth;
            activeCtx.pivotY = state.pivotY * screenHeight;
        } else {
            // SA-MP 640x480 coordinate space
            activeCtx.pivotX = (state.pivotX / 640.0f) * screenWidth;
            activeCtx.pivotY = (state.pivotY / 480.0f) * screenHeight;
        }
    } else {
        // Auto-pivot: calculated dynamically on the first rendered primitive
        // using the real screen vertices: (minX + maxX) * 0.5f and (minY + maxY) * 0.5f
        activeCtx.hasExplicitPivot = false;
        activeCtx.hasCalculatedPivot = false;
    }

    // Activate RenderWare Im2D rotation context
    g_pActiveRotation = &activeCtx;

    // Draw the textdraw
    g_OriginalTextDrawDraw(textDraw);

    // Immediately flush this textdraw's text so letters render under the active rotation
    CallCFontDrawFonts();

    // Clear RenderWare rotation context
    g_pActiveRotation = nullptr;
}

RwBool Hooked_Im2DRenderPrimitive(RwPrimitiveType primType, RwIm2DVertex* vertices, RwInt32 numVertices) {
    if (g_pActiveRotation && vertices && numVertices > 0) {
        // If auto-pivot and not calculated yet, compute from bounding box of this primitive
        if (!g_pActiveRotation->hasCalculatedPivot) {
            float minX = vertices[0].x, maxX = vertices[0].x;
            float minY = vertices[0].y, maxY = vertices[0].y;
            for (RwInt32 i = 1; i < numVertices; ++i) {
                if (vertices[i].x < minX) minX = vertices[i].x;
                if (vertices[i].x > maxX) maxX = vertices[i].x;
                if (vertices[i].y < minY) minY = vertices[i].y;
                if (vertices[i].y > maxY) maxY = vertices[i].y;
            }
            g_pActiveRotation->pivotX = (minX + maxX) * 0.5f;
            g_pActiveRotation->pivotY = (minY + maxY) * 0.5f;
            g_pActiveRotation->hasCalculatedPivot = true;
        }

        const float pX = g_pActiveRotation->pivotX;
        const float pY = g_pActiveRotation->pivotY;
        const float cosA = g_pActiveRotation->cosA;
        const float sinA = g_pActiveRotation->sinA;

        RwIm2DVertex stackVerts[STACK_VERTICES_LIMIT];
        RwIm2DVertex* pVerts = (numVertices <= static_cast<RwInt32>(STACK_VERTICES_LIMIT))
            ? stackVerts
            : new RwIm2DVertex[numVertices];

        std::memcpy(pVerts, vertices, numVertices * sizeof(RwIm2DVertex));

        for (RwInt32 i = 0; i < numVertices; ++i) {
            const float dx = pVerts[i].x - pX;
            const float dy = pVerts[i].y - pY;
            pVerts[i].x = pX + dx * cosA - dy * sinA;
            pVerts[i].y = pY + dx * sinA + dy * cosA;
        }

        RwBool result = FALSE;
        if (g_OriginalIm2DRenderPrimitive) {
            result = g_OriginalIm2DRenderPrimitive(primType, pVerts, numVertices);
        }

        if (pVerts != stackVerts) {
            delete[] pVerts;
        }
        return result;
    }

    if (g_OriginalIm2DRenderPrimitive) {
        return g_OriginalIm2DRenderPrimitive(primType, vertices, numVertices);
    }
    return FALSE;
}

RwBool Hooked_Im2DRenderIndexedPrimitive(RwPrimitiveType primType, RwIm2DVertex* vertices, RwInt32 numVertices, RwImVertexIndex* indices, RwInt32 numIndices) {
    if (g_pActiveRotation && vertices && numVertices > 0) {
        if (!g_pActiveRotation->hasCalculatedPivot) {
            float minX = vertices[0].x, maxX = vertices[0].x;
            float minY = vertices[0].y, maxY = vertices[0].y;
            for (RwInt32 i = 1; i < numVertices; ++i) {
                if (vertices[i].x < minX) minX = vertices[i].x;
                if (vertices[i].x > maxX) maxX = vertices[i].x;
                if (vertices[i].y < minY) minY = vertices[i].y;
                if (vertices[i].y > maxY) maxY = vertices[i].y;
            }
            g_pActiveRotation->pivotX = (minX + maxX) * 0.5f;
            g_pActiveRotation->pivotY = (minY + maxY) * 0.5f;
            g_pActiveRotation->hasCalculatedPivot = true;
        }

        const float pX = g_pActiveRotation->pivotX;
        const float pY = g_pActiveRotation->pivotY;
        const float cosA = g_pActiveRotation->cosA;
        const float sinA = g_pActiveRotation->sinA;

        RwIm2DVertex stackVerts[STACK_VERTICES_LIMIT];
        RwIm2DVertex* pVerts = (numVertices <= static_cast<RwInt32>(STACK_VERTICES_LIMIT))
            ? stackVerts
            : new RwIm2DVertex[numVertices];

        std::memcpy(pVerts, vertices, numVertices * sizeof(RwIm2DVertex));

        for (RwInt32 i = 0; i < numVertices; ++i) {
            const float dx = pVerts[i].x - pX;
            const float dy = pVerts[i].y - pY;
            pVerts[i].x = pX + dx * cosA - dy * sinA;
            pVerts[i].y = pY + dx * sinA + dy * cosA;
        }

        RwBool result = FALSE;
        if (g_OriginalIm2DRenderIndexedPrimitive) {
            result = g_OriginalIm2DRenderIndexedPrimitive(primType, pVerts, numVertices, indices, numIndices);
        }

        if (pVerts != stackVerts) {
            delete[] pVerts;
        }
        return result;
    }

    if (g_OriginalIm2DRenderIndexedPrimitive) {
        return g_OriginalIm2DRenderIndexedPrimitive(primType, vertices, numVertices, indices, numIndices);
    }
    return FALSE;
}

} // namespace

bool Initialize() {
    g_Initialized = true;
    g_HookInstallationAttempted = false;
    return true;
}

void Process() {
    if (!g_Initialized) {
        return;
    }

    if (!g_RwIm2DHookInstalled) {
        InstallRenderWareHooks();
    }

    if (!g_HookInstallationAttempted && IsSampLoaded()) {
        g_HookInstallationAttempted = true;
        InstallSampTextDrawHooks();
    }
}

void Shutdown() {
    if (g_TextDrawDrawAddress) {
        MH_DisableHook(reinterpret_cast<void*>(g_TextDrawDrawAddress));
        MH_RemoveHook(reinterpret_cast<void*>(g_TextDrawDrawAddress));
        g_TextDrawDrawAddress = 0;
        g_OriginalTextDrawDraw = nullptr;
    }

    if (g_TextDrawPoolDeleteAddress) {
        MH_DisableHook(reinterpret_cast<void*>(g_TextDrawPoolDeleteAddress));
        MH_RemoveHook(reinterpret_cast<void*>(g_TextDrawPoolDeleteAddress));
        g_TextDrawPoolDeleteAddress = 0;
        g_OriginalTextDrawPoolDelete = nullptr;
    }

    if (g_TextDrawPoolDrawAddress) {
        MH_DisableHook(reinterpret_cast<void*>(g_TextDrawPoolDrawAddress));
        MH_RemoveHook(reinterpret_cast<void*>(g_TextDrawPoolDrawAddress));
        g_TextDrawPoolDrawAddress = 0;
        g_OriginalTextDrawPoolDraw = nullptr;
    }

    g_pTextDrawPool = nullptr;

    if (g_RwIm2DHookInstalled && RwEngineInstance) {
        auto* rwGlobals = reinterpret_cast<RwGlobals*>(RwEngineInstance);
        if (g_pTargetIm2DRenderPrimitive) {
            MH_DisableHook(g_pTargetIm2DRenderPrimitive);
            MH_RemoveHook(g_pTargetIm2DRenderPrimitive);
            rwGlobals->dOpenDevice.fpIm2DRenderPrimitive = g_RealOriginalIm2DPrimitive;
            g_pTargetIm2DRenderPrimitive = nullptr;
            g_RealOriginalIm2DPrimitive = nullptr;
            g_OriginalIm2DRenderPrimitive = nullptr;
        }
        if (g_pTargetIm2DRenderIndexedPrimitive) {
            MH_DisableHook(g_pTargetIm2DRenderIndexedPrimitive);
            MH_RemoveHook(g_pTargetIm2DRenderIndexedPrimitive);
            rwGlobals->dOpenDevice.fpIm2DRenderIndexedPrimitive = g_RealOriginalIm2DIndexedPrimitive;
            g_pTargetIm2DRenderIndexedPrimitive = nullptr;
            g_RealOriginalIm2DIndexedPrimitive = nullptr;
            g_OriginalIm2DRenderIndexedPrimitive = nullptr;
        }
        g_RwIm2DHookInstalled = false;
    }

    g_pActiveRotation = nullptr;
    g_Initialized = false;
    g_HookInstallationAttempted = false;
    ClearAllStates();
}

bool Set(uint16_t textDrawIndex, float angle, float pivotX, float pivotY, float spinSpeed) {
    if (textDrawIndex >= g_RotationStates.size() ||
        !std::isfinite(angle) || std::fabs(angle) > MAX_ABSOLUTE_ANGLE ||
        !std::isfinite(pivotX) || !std::isfinite(pivotY) ||
        !std::isfinite(spinSpeed) || std::fabs(spinSpeed) > MAX_ABSOLUTE_SPIN_SPEED) {
        return false;
    }

    const std::lock_guard<std::mutex> lock(g_RotationStatesMutex);
    auto& state = g_RotationStates[textDrawIndex];
    state.enabled = true;
    state.angle = angle;
    state.pivotX = pivotX;
    state.pivotY = pivotY;
    state.spinSpeed = spinSpeed;
    state.startTimeMs = GetTickCount64();
    return true;
}

void Reset(uint16_t textDrawIndex) {
    if (textDrawIndex >= g_RotationStates.size()) {
        return;
    }

    const std::lock_guard<std::mutex> lock(g_RotationStatesMutex);
    g_RotationStates[textDrawIndex] = {};
}

void ClearAllStates() {
    const std::lock_guard<std::mutex> lock(g_RotationStatesMutex);
    g_RotationStates.fill({});
}

}
