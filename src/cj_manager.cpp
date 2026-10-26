#include "cj_manager.h"
#include "defs.h"
#include "game_extensions.h"
#include "packet_handler.h"
#include "utils.h"

#include <windows.h>
#include <algorithm>
#include <format>
#include <unordered_map>

#include <game_sa/CClothes.h>
#include <game_sa/CClothesBuilder.h>
#include <game_sa/CKeyGen.h>
#include <game_sa/CModelInfo.h>
#include <game_sa/CPedClothesDesc.h>
#include <game_sa/CPedIntelligence.h>
#include <game_sa/CPlayerPed.h>
#include <game_sa/CStats.h>
#include <game_sa/CStreaming.h>
#include <game_sa/CTask.h>
#include <game_sa/CTaskManager.h>
#include <game_sa/CTxdStore.h>
#include <game_sa/common.h>

#include <MinHook.h>

namespace cj_manager {

static std::unordered_map<uint16_t, CJSkinState> g_cachedStates;
static bool g_bInitialized = false;
static bool g_bInRebuild = false;

// Hook typedef and original pointer for CPed::SetModelIndex
using CPed_SetModelIndex_t = void(__thiscall*)(CPed*, unsigned int);
static CPed_SetModelIndex_t orig_CPed_SetModelIndex = nullptr;

void EnsureClothesSystemInitialized();

// Safe hooks for CClothesBuilder to prevent infinite stream hanging on missing player.img files
using AppendStringToKey_t = unsigned int(__cdecl*)(unsigned int key, const char* str);
static auto fnAppendStringToKey = reinterpret_cast<AppendStringToKey_t>(0x53CF70);

using CdDirFindItem_t = bool(__thiscall*)(void* pCdDir, unsigned int fullKey, unsigned int* pOutOffset, unsigned int* pOutSize);
static auto fnFindItem = reinterpret_cast<CdDirFindItem_t>(0x5324D0);

static bool DoesClothesFileExist(unsigned int baseKey, const char* ext)
{
	if (baseKey == 0) return false;
	EnsureClothesSystemInitialized();
	unsigned int fullKey = fnAppendStringToKey(baseKey, ext);
	void* pCdDir = reinterpret_cast<void*>(0xBC12C0);
	if (!pCdDir) return false;
	unsigned int outOffset = 0, outSize = 0;
	return fnFindItem(pCdDir, fullKey, &outOffset, &outSize);
}

using RequestGeometry_t = void(__cdecl*)(int modelId, unsigned int crc);
static RequestGeometry_t orig_RequestGeometry = nullptr;

static void __cdecl Hooked_RequestGeometry(int modelId, unsigned int crc)
{
	if (crc == 0 || !DoesClothesFileExist(crc, ".DFF")) {
		ClientLog(LogLevel::Warning, std::format("RequestGeometry: Model 0x{:X} (modelId={}) not found in player.img, skipping to prevent freeze", crc, modelId));
		return;
	}
	orig_RequestGeometry(modelId, crc);
}

using RequestTexture_t = int(__cdecl*)(unsigned int crc);
static RequestTexture_t orig_RequestTexture = nullptr;

static int __cdecl Hooked_RequestTexture(unsigned int crc)
{
	if (crc == 0) return -1;
	if (!DoesClothesFileExist(crc, ".TXD")) {
		ClientLog(LogLevel::Warning, std::format("RequestTexture: Texture 0x{:X} not found in player.img, skipping to prevent freeze", crc));
		return -1;
	}
	return orig_RequestTexture(crc);
}

static bool SafeLoadCdDirectory()
{
	__try {
		CClothesBuilder::LoadCdDirectory();
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

static bool SafeClothesInit()
{
	__try {
		CClothes::Init();
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

static void* SafeExtractAnimAssocs(void* pRwClump)
{
	if (!pRwClump) return nullptr;
	using ExtractAssocs_t = void*(__cdecl*)(void*);
	auto fnExtractAssocs = reinterpret_cast<ExtractAssocs_t>(0x4D6BE0);
	__try {
		return fnExtractAssocs(pRwClump);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

static void SafeGiveAnimAssocs(void* pRwClump, void* pAnimAssocs)
{
	if (!pRwClump || !pAnimAssocs) return;
	using GiveAssocs_t = void(__cdecl*)(void*, void*);
	auto fnGiveAssocs = reinterpret_cast<GiveAssocs_t>(0x4D6C30);
	__try {
		fnGiveAssocs(pRwClump, pAnimAssocs);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
}

static void SafeAbortIKTask(CPed* pPed)
{
	if (!pPed || !pPed->m_pIntelligence) return;
	__try {
		CTask* pTask = pPed->m_pIntelligence->m_TaskMgr.GetTaskSecondary(3); // TASK_SECONDARY_IK = 3
		if (pTask) {
			pTask->MakeAbortable(pPed, ABORT_PRIORITY_IMMEDIATE, nullptr);
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
}

__declspec(noinline) static void ConstructAndSetModelInternal(CPedClothesDesc* pClothesDesc, CPed* pPed)
{
	ClientLog(LogLevel::Debug, "ConstructAndSetModelInternal: starting ped rebuild");

	// 1. Ensure Model 0 (player.dff) is loaded in CStreaming.
	if (!CStreaming::HasModelLoaded(0)) {
		ClientLog(LogLevel::Debug, "ConstructAndSetModelInternal: Model 0 not loaded! Requesting via CStreaming (no wait)...");
		CStreaming::RequestModel(0, 0x16);
		for (int i = 0; i < 50 && !CStreaming::HasModelLoaded(0); ++i) {
			using StreamingUpdate_t = void(__cdecl*)();
			auto fnUpdate = reinterpret_cast<StreamingUpdate_t>(0x40E460); // CStreaming::Update
			fnUpdate();
		}
		if (!CStreaming::HasModelLoaded(0)) {
			ClientLog(LogLevel::Error, "ConstructAndSetModelInternal: Model 0 still not loaded after spin-wait, aborting rebuild.");
			return;
		}
	}
	// 2. CClothes::ms_clothesDesc at 0xBC1C78 is GTA's static previous clothes descriptor
	auto* pOldClothes = reinterpret_cast<CPedClothesDesc*>(0xBC1C78);

	// 3. Extract animation blend associations and abort secondary IK task before destroying old clump
	void* pAnimAssocs = pPed->m_pRwClump ? SafeExtractAnimAssocs(pPed->m_pRwClump) : nullptr;
	SafeAbortIKTask(pPed);

	// 4. Delete old RwObject if attached to avoid leaking / duplicate RenderWare clumps
	if (pPed->m_pRwClump) {
		ClientLog(LogLevel::Debug, "ConstructAndSetModelInternal: deleting old RwObject");
		pPed->DeleteRwObject();
	}

	// 5. Remove ped from world sectors to prevent duplicate entry in sector linked list.
	ClientLog(LogLevel::Debug, "ConstructAndSetModelInternal: removing ped from world sector");
	using DeInit_t = void(__cdecl*)(CPed*);
	auto fnDeInit = reinterpret_cast<DeInit_t>(0x563280);
	fnDeInit(pPed);

	// 6. Construct the skinned ped model for model 0
	ClientLog(LogLevel::Debug, "ConstructAndSetModelInternal: calling CClothes::ConstructPedModel");
	CClothes::ConstructPedModel(0, *pClothesDesc, pOldClothes, false);
	ClientLog(LogLevel::Debug, "ConstructAndSetModelInternal: CClothes::ConstructPedModel finished");

	// 7. Update the static previous clothes descriptor
	memcpy(pOldClothes, pClothesDesc, sizeof(CPedClothesDesc));

	// 8. Ensure ped model index is set to 0 before invoking GTA's rebuild helper
	pPed->m_nModelIndex = 0;

	// 9. GTA's internal rebuild helper at 0x5E0130 (CPed::Dress)
	ClientLog(LogLevel::Debug, "ConstructAndSetModelInternal: calling fnRebuildHelper (CPed::Dress)");
	using RebuildHelper_t = void(__thiscall*)(CPed*);
	auto fnRebuildHelper = reinterpret_cast<RebuildHelper_t>(0x5E0130);
	fnRebuildHelper(pPed);
	ClientLog(LogLevel::Debug, "ConstructAndSetModelInternal: fnRebuildHelper finished successfully");

	// 10. Restore preserved animation associations onto the newly created clump
	if (pAnimAssocs && pPed->m_pRwClump) {
		SafeGiveAnimAssocs(pPed->m_pRwClump, pAnimAssocs);
	}
}

static void LogSehException()
{
	ClientLog(LogLevel::Error, "SafeConstructAndSetModel: CAUGHT EXCEPTION inside __except block");
}

static bool SafeConstructAndSetModel(CPedClothesDesc* pClothesDesc, CPed* pPed)
{
	__try {
		ConstructAndSetModelInternal(pClothesDesc, pPed);
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		LogSehException();
		return false;
	}
}

#pragma pack(push, 1)
struct tStreamingFile {
	char m_szName[40];
	bool m_bIsStreamed;
	char m_pad[3];
	int  m_streamHandle;
};
#pragma pack(pop)
static_assert(sizeof(tStreamingFile) == 48, "tStreamingFile size must be 48");

void EnsureClothesSystemInitialized()
{
	static bool s_bClothesInit = false;
	if (s_bClothesInit) return;
	s_bClothesInit = true;

	ClientLog(LogLevel::Info, "EnsureClothesSystemInitialized: scanning CStreaming::ms_files [0x8E48D8]...");
	auto* files = reinterpret_cast<tStreamingFile*>(0x8E48D8);
	int playerImgSlot = -1;
	int freeSlot = -1;
	int replaceableSlot = -1;

	for (int i = 0; i < 8; ++i) {
		ClientLog(LogLevel::Debug, std::format("  ms_files[{}]: name='{}', streamed={}, handle=0x{:X}",
			i, files[i].m_szName, (int)files[i].m_bIsStreamed, files[i].m_streamHandle));

		if (_stricmp(files[i].m_szName, "MODELS\\PLAYER.IMG") == 0 ||
			_stricmp(files[i].m_szName, "models/player.img") == 0 ||
			_stricmp(files[i].m_szName, "player.img") == 0) {
			playerImgSlot = i;
		}
		if (files[i].m_szName[0] == '\0' && freeSlot == -1) {
			freeSlot = i;
		}
		// Singleplayer cutscenes / car recordings / mission scripts are not used in SA-MP
		if (replaceableSlot == -1 &&
			(strstr(files[i].m_szName, "cutscene") || strstr(files[i].m_szName, "CUTSCENE") ||
			 strstr(files[i].m_szName, "carrec")   || strstr(files[i].m_szName, "CARREC") ||
			 strstr(files[i].m_szName, "script")   || strstr(files[i].m_szName, "SCRIPT"))) {
			replaceableSlot = i;
		}
	}

	using CdStreamOpen_t = int(__cdecl*)(const char*, bool);
	auto fnCdStreamOpen = reinterpret_cast<CdStreamOpen_t>(0x4067B0);

	if (playerImgSlot == -1) {
		if (freeSlot != -1) {
			playerImgSlot = freeSlot;
			ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: assigning free slot {} to MODELS\\PLAYER.IMG", playerImgSlot));
		} else if (replaceableSlot != -1) {
			playerImgSlot = replaceableSlot;
			ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: replacing unused SP archive at slot {} ('{}') with MODELS\\PLAYER.IMG",
				playerImgSlot, files[playerImgSlot].m_szName));
		} else {
			playerImgSlot = 7;
			ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: fallback to slot {} for MODELS\\PLAYER.IMG", playerImgSlot));
		}

		strncpy(files[playerImgSlot].m_szName, "MODELS\\PLAYER.IMG", sizeof(files[playerImgSlot].m_szName) - 1);
		files[playerImgSlot].m_szName[sizeof(files[playerImgSlot].m_szName) - 1] = '\0';
		files[playerImgSlot].m_bIsStreamed = false;
		files[playerImgSlot].m_streamHandle = fnCdStreamOpen("MODELS\\PLAYER.IMG", false);
		ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: opened MODELS\\PLAYER.IMG at slot {} (handle=0x{:X})",
			playerImgSlot, files[playerImgSlot].m_streamHandle));
	} else {
		if (files[playerImgSlot].m_streamHandle == 0) {
			files[playerImgSlot].m_bIsStreamed = false;
			files[playerImgSlot].m_streamHandle = fnCdStreamOpen("MODELS\\PLAYER.IMG", false);
			ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: refreshed handle for slot {} (handle=0x{:X})",
				playerImgSlot, files[playerImgSlot].m_streamHandle));
		} else {
			ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: MODELS\\PLAYER.IMG already open at slot {} (handle=0x{:X})",
				playerImgSlot, files[playerImgSlot].m_streamHandle));
		}
	}

	ClientLog(LogLevel::Info, "EnsureClothesSystemInitialized: calling CClothes::Init() [0x5A80D0]...");
	if (SafeClothesInit()) {
		uint32_t* pClothesCount = reinterpret_cast<uint32_t*>(0xBC12FC);
		ClientLog(LogLevel::Info, std::format("CClothes::Init() finished: clothesCount={}", (pClothesCount ? *pClothesCount : 0)));
	} else {
		ClientLog(LogLevel::Error, "CClothes::Init() failed with SEH exception!");
	}

	// Force 0xBC12F8 (ms_clothesImgId) to the valid playerImgSlot where MODELS\\PLAYER.IMG is open!
	*reinterpret_cast<int*>(0xBC12F8) = playerImgSlot;
	ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: 0xBC12F8 (ms_clothesImgId) forced to slot {}", playerImgSlot));

	ClientLog(LogLevel::Info, "EnsureClothesSystemInitialized: loading CD directory [0x5A4190]...");
	if (SafeLoadCdDirectory()) {
		uint32_t* pNumEntries = reinterpret_cast<uint32_t*>(0xBC12C8);
		ClientLog(LogLevel::Info, std::format("CClothesBuilder::LoadCdDirectory() finished: entries={}", (pNumEntries ? *pNumEntries : 0)));
	} else {
		ClientLog(LogLevel::Error, "CClothesBuilder::LoadCdDirectory() failed with SEH exception!");
	}

	// Ensure player.txd is loaded in CTxdStore
	int playerTxdSlot = CTxdStore::FindTxdSlot("player");
	if (playerTxdSlot == -1) {
		playerTxdSlot = CTxdStore::AddTxdSlot("player");
		ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: added player.txd slot: {}", playerTxdSlot));
	} else {
		ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: found player.txd slot: {}", playerTxdSlot));
	}

	bool bTxdLoaded = false;
	if (CTxdStore::ms_pTxdPool && CTxdStore::ms_pTxdPool->GetAt(playerTxdSlot)) {
		bTxdLoaded = (CTxdStore::ms_pTxdPool->GetAt(playerTxdSlot)->m_pRwDictionary != nullptr);
	}
	if (!bTxdLoaded) {
		ClientLog(LogLevel::Info, "EnsureClothesSystemInitialized: requesting player.txd via CStreaming::RequestTxdModel...");
		CStreaming::RequestTxdModel(playerTxdSlot, 0x16);
		CStreaming::LoadAllRequestedModels(false);
		if (CTxdStore::ms_pTxdPool && CTxdStore::ms_pTxdPool->GetAt(playerTxdSlot)) {
			bTxdLoaded = (CTxdStore::ms_pTxdPool->GetAt(playerTxdSlot)->m_pRwDictionary != nullptr);
		}
		ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: player.txd loaded: {}", bTxdLoaded ? "true" : "false"));
	}

	// Ensure Model 0 (player.dff) is loaded in CStreaming
	if (!CStreaming::HasModelLoaded(0)) {
		ClientLog(LogLevel::Info, "EnsureClothesSystemInitialized: Model 0 (player.dff) not loaded, requesting via CStreaming...");
		CStreaming::RequestModel(0, 0x16);
		CStreaming::LoadAllRequestedModels(false);
		ClientLog(LogLevel::Info, std::format("EnsureClothesSystemInitialized: Model 0 load result: {}", CStreaming::HasModelLoaded(0) ? "true" : "false"));
	}

	CClothes::RequestMotionGroupAnims();
	game_extensions::EnsureJetpackLoaded();
	ClientLog(LogLevel::Info, "EnsureClothesSystemInitialized: GTA:SA Clothes subsystem verified and ready");
}

static void __fastcall Hook_CPed_SetModelIndex(CPed* thisPed, void* edx, unsigned int modelIndex)
{
	if (g_bInRebuild) {
		if (orig_CPed_SetModelIndex) {
			orig_CPed_SetModelIndex(thisPed, modelIndex);
		}
		return;
	}

	if (modelIndex == 0) {
		g_bInRebuild = true;
		EnsureClothesSystemInitialized();
		EnsurePedIsCJ(thisPed);

		uint16_t localId = GetLocalPlayerId();
		if (IsLocalPlayerPed(thisPed)) {
			if (HasCachedState(CJ_LOCAL_PLAYER_ID)) {
				ApplyCachedState(CJ_LOCAL_PLAYER_ID, thisPed);
			} else if (localId != CJ_LOCAL_PLAYER_ID && HasCachedState(localId)) {
				ApplyCachedState(localId, thisPed);
			} else {
				RebuildCJ(thisPed);
			}
		} else {
			bool applied = false;
			for (auto& [playerId, state] : g_cachedStates) {
				if (GetGamePed(playerId) == thisPed) {
					ApplyCachedState(playerId, thisPed);
					applied = true;
					break;
				}
			}
			if (!applied) {
				RebuildCJ(thisPed);
			}
		}

		thisPed->m_nModelIndex = 0;
		g_bInRebuild = false;
		return;
	}

	// Ped is switching from CJ to a non-CJ model:
	if (!IsLocalPlayerPed(thisPed) && thisPed->m_pPlayerData) {
		if (thisPed->m_pPlayerData->m_pPedClothesDesc) {
			delete thisPed->m_pPlayerData->m_pPedClothesDesc;
			thisPed->m_pPlayerData->m_pPedClothesDesc = nullptr;
		}
		delete thisPed->m_pPlayerData;
		thisPed->m_pPlayerData = nullptr;
	}

	if (orig_CPed_SetModelIndex) {
		orig_CPed_SetModelIndex(thisPed, modelIndex);
	}
}

bool Initialize()
{
	if (g_bInitialized) return true;

	MH_Initialize();

	MH_STATUS status = MH_CreateHook(
		reinterpret_cast<void*>(0x5E4880),
		reinterpret_cast<void*>(&Hook_CPed_SetModelIndex),
		reinterpret_cast<void**>(&orig_CPed_SetModelIndex));

	if (status == MH_OK || status == MH_ERROR_ALREADY_CREATED) {
		MH_EnableHook(reinterpret_cast<void*>(0x5E4880));
	}

	MH_CreateHook(
		reinterpret_cast<void*>(0x5A41C0),
		reinterpret_cast<void*>(&Hooked_RequestGeometry),
		reinterpret_cast<void**>(&orig_RequestGeometry));
	MH_EnableHook(reinterpret_cast<void*>(0x5A41C0));

	MH_CreateHook(
		reinterpret_cast<void*>(0x5A4220),
		reinterpret_cast<void*>(&Hooked_RequestTexture),
		reinterpret_cast<void**>(&orig_RequestTexture));
	MH_EnableHook(reinterpret_cast<void*>(0x5A4220));

	g_bInitialized = true;
	return true;
}

void Shutdown()
{
	if (!g_bInitialized) return;

	MH_DisableHook(reinterpret_cast<void*>(0x5E4880));
	MH_RemoveHook(reinterpret_cast<void*>(0x5E4880));

	MH_DisableHook(reinterpret_cast<void*>(0x5A41C0));
	MH_RemoveHook(reinterpret_cast<void*>(0x5A41C0));

	MH_DisableHook(reinterpret_cast<void*>(0x5A4220));
	MH_RemoveHook(reinterpret_cast<void*>(0x5A4220));

	g_cachedStates.clear();
	g_bInitialized = false;
}

static inline eClothesModelPart GetModelPartForTextureSlot(uint8_t slot)
{
	static constexpr eClothesModelPart s_mapping[CJ_SLOT_COUNT] = {
		CLOTHES_MODEL_TORSO,        // 0: CLOTHES_TEXTURE_TORSO
		CLOTHES_MODEL_HEAD,         // 1: CLOTHES_TEXTURE_HEAD
		CLOTHES_MODEL_LEGS,         // 2: CLOTHES_TEXTURE_LEGS
		CLOTHES_MODEL_SHOES,        // 3: CLOTHES_TEXTURE_SHOES
		CLOTHES_MODEL_UNAVAILABLE,  // 4: Tattoo lower left arm (none)
		CLOTHES_MODEL_UNAVAILABLE,  // 5: Tattoo upper left arm (none)
		CLOTHES_MODEL_UNAVAILABLE,  // 6: Tattoo lower right arm (none)
		CLOTHES_MODEL_UNAVAILABLE,  // 7: Tattoo upper right arm (none)
		CLOTHES_MODEL_UNAVAILABLE,  // 8: Tattoo back (none)
		CLOTHES_MODEL_UNAVAILABLE,  // 9: Tattoo left chest (none)
		CLOTHES_MODEL_UNAVAILABLE,  // 10: Tattoo right chest (none)
		CLOTHES_MODEL_UNAVAILABLE,  // 11: Tattoo stomach (none)
		CLOTHES_MODEL_UNAVAILABLE,  // 12: Tattoo lower back (none)
		CLOTHES_MODEL_NECKLACE,     // 13: CLOTHES_TEXTURE_NECKLACE
		CLOTHES_MODEL_BRACELET,     // 14: CLOTHES_TEXTURE_BRACELET (watch)
		CLOTHES_MODEL_GLASSES,      // 15: CLOTHES_TEXTURE_GLASSES
		CLOTHES_MODEL_HATS,         // 16: CLOTHES_TEXTURE_HATS
		CLOTHES_MODEL_SPECIAL       // 17: CLOTHES_TEXTURE_SPECIAL (suits, uniforms, etc.)
	};
	return (slot < CJ_SLOT_COUNT) ? s_mapping[slot] : CLOTHES_MODEL_UNAVAILABLE;
}

static inline void SetClothesDescHash(CPedClothesDesc* desc, uint8_t slot, uint32_t textureHash, uint32_t modelHash)
{
	if (!desc || slot >= CJ_SLOT_COUNT) return;
	desc->m_anTextureKeys[slot] = textureHash;
	eClothesModelPart modelPart = GetModelPartForTextureSlot(slot);
	if (modelPart < 10) {
		desc->m_anModelKeys[modelPart] = (textureHash == 0) ? 0 : modelHash;
	}
}

static inline void SetDefaultCJClothes(CPedClothesDesc* desc)
{
	if (!desc) return;
	memset(desc, 0, sizeof(CPedClothesDesc));
	SetClothesDescHash(desc, CLOTHES_TEXTURE_TORSO, CKeyGen::GetUppercaseKey("vest"), CKeyGen::GetUppercaseKey("vest"));
	SetClothesDescHash(desc, CLOTHES_TEXTURE_LEGS, CKeyGen::GetUppercaseKey("jeansdenim"), CKeyGen::GetUppercaseKey("jeans"));
	SetClothesDescHash(desc, CLOTHES_TEXTURE_SHOES, CKeyGen::GetUppercaseKey("sneakerbincblk"), CKeyGen::GetUppercaseKey("sneaker"));
	SetClothesDescHash(desc, CLOTHES_TEXTURE_HEAD, CKeyGen::GetUppercaseKey("player_face"), CKeyGen::GetUppercaseKey("head"));
}

bool EnsurePedIsCJ(CPed* pPed)
{
	if (!pPed) return false;
	EnsureClothesSystemInitialized();

	if (!pPed->m_pPlayerData) {
		pPed->m_pPlayerData = new CPlayerData();
		memset(pPed->m_pPlayerData, 0, sizeof(CPlayerData));
	}

	if (!pPed->m_pPlayerData->m_pPedClothesDesc) {
		pPed->m_pPlayerData->m_pPedClothesDesc = new CPedClothesDesc();
		SetDefaultCJClothes(pPed->m_pPlayerData->m_pPedClothesDesc);
	}

	return true;
}

bool SetClothes(CPed* pPed, uint8_t slot, const std::string& texture, const std::string& model, bool bRebuild)
{
	if (!pPed || slot >= CJ_SLOT_COUNT) return false;
	EnsurePedIsCJ(pPed);

	auto* pClothesDesc = pPed->m_pPlayerData->m_pPedClothesDesc;
	if (!pClothesDesc) return false;

	bool isRemove = (texture.empty() || texture == "none" || texture == "0");
	uint32_t texHash = isRemove ? 0 : CKeyGen::GetUppercaseKey(texture.c_str());
	uint32_t mdlHash = (isRemove || model.empty() || model == "none" || model == "0") ? 0 : CKeyGen::GetUppercaseKey(model.c_str());

	// Validate that the requested texture and model actually exist in player.img
	if (texHash != 0 && !DoesClothesFileExist(texHash, ".TXD")) {
		ClientLog(LogLevel::Warning, std::format("SetClothes: Texture '{}' (0x{:X}) not found in player.img, skipping", texture, texHash));
		texHash = 0;
	}
	if (mdlHash != 0 && !DoesClothesFileExist(mdlHash, ".DFF")) {
		ClientLog(LogLevel::Warning, std::format("SetClothes: Model '{}' (0x{:X}) not found in player.img, skipping", model, mdlHash));
		mdlHash = 0;
	}

	SetClothesDescHash(pClothesDesc, slot, texHash, mdlHash);

	if (bRebuild) {
		RebuildCJ(pPed);
	}
	return true;
}

bool SetClothesHash(CPed* pPed, uint8_t slot, uint32_t textureHash, uint32_t modelHash, bool bRebuild)
{
	if (!pPed || slot >= CJ_SLOT_COUNT) return false;
	EnsurePedIsCJ(pPed);

	auto* pClothesDesc = pPed->m_pPlayerData->m_pPedClothesDesc;
	if (!pClothesDesc) return false;

	// Validate that the requested texture and model actually exist in player.img
	if (textureHash != 0 && !DoesClothesFileExist(textureHash, ".TXD")) {
		ClientLog(LogLevel::Warning, std::format("SetClothesHash: Texture 0x{:X} not found in player.img, skipping", textureHash));
		textureHash = 0;
	}
	if (modelHash != 0 && !DoesClothesFileExist(modelHash, ".DFF")) {
		ClientLog(LogLevel::Warning, std::format("SetClothesHash: Model 0x{:X} not found in player.img, skipping", modelHash));
		modelHash = 0;
	}

	SetClothesDescHash(pClothesDesc, slot, textureHash, modelHash);

	if (bRebuild) {
		RebuildCJ(pPed);
	}
	return true;
}

bool RemoveClothes(CPed* pPed, uint8_t slot, bool bRebuild)
{
	return SetClothesHash(pPed, slot, 0, 0, bRebuild);
}

bool SetBodyStats(CPed* pPed, float fFat, float fMuscle, bool bRebuild)
{
	if (!pPed) return false;
	EnsurePedIsCJ(pPed);

	auto* pClothesDesc = pPed->m_pPlayerData->m_pPedClothesDesc;
	if (!pClothesDesc) return false;

	fFat = std::clamp(fFat, 0.0f, 1000.0f);
	fMuscle = std::clamp(fMuscle, 0.0f, 1000.0f);

	pClothesDesc->m_fFatStat = fFat;
	pClothesDesc->m_fMuscleStat = fMuscle;

	if (IsLocalPlayerPed(pPed)) {
		CStats::SetStatValue(STAT_FAT, fFat);
		CStats::SetStatValue(STAT_MUSCLE, fMuscle);
	}

	if (bRebuild) {
		RebuildCJ(pPed);
	}
	return true;
}

bool ResetClothes(CPed* pPed, bool toDefault, bool bRebuild)
{
	if (!pPed) return false;
	EnsurePedIsCJ(pPed);

	auto* pClothesDesc = pPed->m_pPlayerData->m_pPedClothesDesc;
	if (!pClothesDesc) return false;

	if (toDefault) {
		SetDefaultCJClothes(pClothesDesc);
	} else {
		memset(pClothesDesc, 0, sizeof(CPedClothesDesc));
	}

	if (bRebuild) {
		RebuildCJ(pPed);
	}
	return true;
}

bool RebuildCJ(CPed* pPed)
{
	if (!pPed) return false;
	EnsureClothesSystemInitialized();
	EnsurePedIsCJ(pPed);

	auto* pClothesDesc = pPed->m_pPlayerData ? pPed->m_pPlayerData->m_pPedClothesDesc : nullptr;
	if (!pClothesDesc) {
		return false;
	}

	// Skip duplicate rebuild if ped is already wearing the exact requested clothes and stats
	auto* pOldClothes = reinterpret_cast<CPedClothesDesc*>(0xBC1C78);
	if (pPed->m_nModelIndex == 0 && pPed->m_pRwClump != nullptr && pOldClothes != nullptr) {
		if (memcmp(pClothesDesc->m_anModelKeys, pOldClothes->m_anModelKeys, sizeof(pClothesDesc->m_anModelKeys)) == 0 &&
			memcmp(pClothesDesc->m_anTextureKeys, pOldClothes->m_anTextureKeys, sizeof(pClothesDesc->m_anTextureKeys)) == 0 &&
			pClothesDesc->m_fFatStat == pOldClothes->m_fFatStat &&
			pClothesDesc->m_fMuscleStat == pOldClothes->m_fMuscleStat) {
			ClientLog(LogLevel::Debug, "RebuildCJ: ped already wearing requested clothes and stats, skipping duplicate rebuild");
			return true;
		}
	}

	AccessoryBackup accBackup;
	BackupAccessories(pPed, accBackup);

	bool previousRebuild = g_bInRebuild;
	g_bInRebuild = true;

	bool bSuccess = SafeConstructAndSetModel(pClothesDesc, pPed);
	if (bSuccess) {
		pPed->m_nModelIndex = 0;

		for (auto& [id, st] : g_cachedStates) {
			if (GetGamePed(id) == pPed) {
				st.appliedVersion = st.version;
				st.lastPed = pPed;
			}
		}
	} else {
		ClientLog(LogLevel::Error, "SafeConstructAndSetModel failed with SEH exception in RebuildCJ");
	}

	g_bInRebuild = previousRebuild;
	RestoreAccessories(accBackup);
	return bSuccess;
}

void CacheClothes(uint16_t playerId, uint8_t slot, uint32_t textureHash, uint32_t modelHash)
{
	if (slot >= CJ_SLOT_COUNT) return;

	if (textureHash != 0 && !DoesClothesFileExist(textureHash, ".TXD")) {
		textureHash = 0;
	}
	if (modelHash != 0 && !DoesClothesFileExist(modelHash, ".DFF")) {
		modelHash = 0;
	}

	auto& state = g_cachedStates[playerId];
	state.hasCustomSkin = true;
	state.textureKeys[slot] = textureHash;

	eClothesModelPart modelPart = GetModelPartForTextureSlot(slot);
	if (modelPart < 10) {
		state.modelKeys[modelPart] = (textureHash == 0) ? 0 : modelHash;
	}
	state.version++;
}

void CacheBodyStats(uint16_t playerId, float fat, float muscle)
{
	auto& state = g_cachedStates[playerId];
	state.hasCustomSkin = true;
	state.fat = std::clamp(fat, 0.0f, 1000.0f);
	state.muscle = std::clamp(muscle, 0.0f, 1000.0f);
	state.version++;
}

void CacheReset(uint16_t playerId, bool toDefault)
{
	auto& state = g_cachedStates[playerId];
	state = CJSkinState{};
	state.hasCustomSkin = true;
	if (toDefault) {
		CacheClothes(playerId, CLOTHES_TEXTURE_TORSO, CKeyGen::GetUppercaseKey("vest"), CKeyGen::GetUppercaseKey("vest"));
		CacheClothes(playerId, CLOTHES_TEXTURE_LEGS, CKeyGen::GetUppercaseKey("jeansdenim"), CKeyGen::GetUppercaseKey("jeans"));
		CacheClothes(playerId, CLOTHES_TEXTURE_SHOES, CKeyGen::GetUppercaseKey("sneakerbincblk"), CKeyGen::GetUppercaseKey("sneaker"));
		CacheClothes(playerId, CLOTHES_TEXTURE_HEAD, CKeyGen::GetUppercaseKey("player_face"), CKeyGen::GetUppercaseKey("head"));
	}
	state.version++;
}

bool HasCachedState(uint16_t playerId)
{
	auto it = g_cachedStates.find(playerId);
	return it != g_cachedStates.end() && it->second.hasCustomSkin;
}

void ApplyCachedState(uint16_t playerId, CPed* pPed)
{
	if (!pPed) return;
	auto it = g_cachedStates.find(playerId);
	if (it == g_cachedStates.end() || !it->second.hasCustomSkin) return;

	EnsurePedIsCJ(pPed);
	auto* pClothesDesc = pPed->m_pPlayerData->m_pPedClothesDesc;
	if (!pClothesDesc) return;

	auto& state = it->second;
	memset(pClothesDesc, 0, sizeof(CPedClothesDesc));

	for (int slot = 0; slot < CJ_SLOT_COUNT; ++slot) {
		if (state.textureKeys[slot] != 0) {
			uint32_t modelKey = 0;
			eClothesModelPart modelPart = GetModelPartForTextureSlot(slot);
			if (modelPart < 10) {
				modelKey = state.modelKeys[modelPart];
			}
			SetClothesDescHash(pClothesDesc, slot, state.textureKeys[slot], modelKey);
		}
	}

	SetBodyStats(pPed, state.fat, state.muscle, false);
	RebuildCJ(pPed);

	state.appliedVersion = state.version;
	state.lastPed = pPed;
}

static std::unordered_map<uint16_t, uint32_t> g_lastSyncRequests;

void RemoveCachedState(uint16_t playerId)
{
	g_cachedStates.erase(playerId);
	g_lastSyncRequests.erase(playerId);
}

void ClearAllCachedStates()
{
	g_cachedStates.clear();
	g_lastSyncRequests.clear();
}

void Process()
{
	EnsureClothesSystemInitialized();

	// 1. Synchronize cached states if needed (only when version updated or newly streamed in ped)
	for (auto& [playerId, state] : g_cachedStates) {
		if (!state.hasCustomSkin) continue;

		CPed* pPed = GetGamePed(playerId);
		if (!pPed || pPed->m_nModelIndex != 0) continue;

		if (state.appliedVersion != state.version || state.lastPed != pPed) {
			ApplyCachedState(playerId, pPed);
		}
	}

	// 2. Client-side fallback request: check if any streamed remote player is Model 0 but has NO cached state
	uint32_t now = GetTickCount();
	static uint32_t s_lastPoolScan = 0;
	if (now - s_lastPoolScan >= 1000) {
		s_lastPoolScan = now;
		for (uint16_t i = 0; i < 1000; ++i) {
			CPed* pPed = GetGamePed(i);
			if (pPed && pPed->m_nModelIndex == 0 && !HasCachedState(i)) {
				auto it = g_lastSyncRequests.find(i);
				if (it == g_lastSyncRequests.end() || (now - it->second) >= 3000) {
					g_lastSyncRequests[i] = now;
					packet_handler::RequestPlayerSync(i);
				}
			}
		}
	}
}

}
