#include "main.h"
#include "cj_manager.h"
#include "defs.h"
#include "game_extensions.h"
#include "textdraw_rotation.h"
#include "utils.h"
#include "water_zones.h"

#include <MinHook.h>
#include <RakHook/rakhook.hpp>

#include <chrono>
#include <format>
#include <thread>

#include "../pluginconfig.h"

using GameLoop_t = void(__cdecl*)();
static GameLoop_t orig_game_loop = nullptr;

static void hooked_game_loop()
{
	if (orig_game_loop) {
		orig_game_loop();
	}
	ExtendedGamePlugin::Instance().OnGameProcess();
}

void ExtendedGamePlugin::Init(HMODULE hModule)
{
	m_hModule = hModule;
	ClientLog(LogLevel::Info, std::format("BrownTurbo-ExtendedGame v{}.{}.{}.{}-{} DLL_PROCESS_ATTACH",
		PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR, PLUGIN_VERSION_PATCH, PLUGIN_VERSION_BUILD, PLUGIN_PHASE));

	// 1. Immediately install Model 0 hook and gameplay/environment hooks in GTA:SA
	if (!m_bModel0HookInstalled) {
		CJManager::Initialize();
		GameExtensions::Initialize();
		TextDrawRotation::Initialize();
		WaterZones::Initialize();
		m_bModel0HookInstalled = true;
		ClientLog(LogLevel::Info, "Model CJ, gameplay extensions, textdraw rotation, and water zones hooks initialized");
	}

	// 2. Hook CGame::Process (0x53BEE0) via MinHook so OnGameProcess executes every frame
	if (!m_bEventHooked) {
		MH_Initialize();
		MH_STATUS status = MH_CreateHook(
			reinterpret_cast<void*>(0x53BEE0),
			reinterpret_cast<void*>(&hooked_game_loop),
			reinterpret_cast<void**>(&orig_game_loop));
		if (status == MH_OK || status == MH_ERROR_ALREADY_CREATED) {
			MH_EnableHook(reinterpret_cast<void*>(0x53BEE0));
			m_bEventHooked = true;
			ClientLog(LogLevel::Info, "CGame::Process (0x53BEE0) hooked via MinHook");
		} else {
			ClientLog(LogLevel::Error, std::format("Failed to hook CGame::Process (status={})", static_cast<int>(status)));
		}
	}

	// 3. Launch background initialization thread to wait for SA-MP and RakHook
	static bool s_bThreadSpawned = false;
	if (!s_bThreadSpawned) {
		s_bThreadSpawned = true;
		std::thread([this]() {
			this->InitializeRakHookThread();
		}).detach();
	}
}

void ExtendedGamePlugin::InitializeRakHookThread()
{
	ClientLog(LogLevel::Info, "InitializeRakHookThread started, waiting for samp.dll...");

	while (::GetModuleHandleA("samp.dll") == nullptr) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	ClientLog(LogLevel::Info, std::format("samp.dll detected at 0x{:X}", rakhook::samp_addr()));

	while (!m_bRakHookReady) {
		if (rakhook::samp_addr() && rakhook::samp_version() != rakhook::samp_ver::unknown) {
			if (IsGameInitialized()) {
				if (rakhook::initialize()) {
					m_bRakHookReady = true;
					PacketHandler::Initialize();
					ClientLog(LogLevel::Info, std::format("rakhook initialized successfully (samp_version={})", static_cast<int>(rakhook::samp_version())));
					SendMsg(0x00FF00AA, "{00FF00}[BrownTurbo-ExtendedGame]{FFFFFF} ASI Plugin initialized.");
					break;
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void ExtendedGamePlugin::OnGameProcess()
{
	static bool s_bClothesInit = false;
	if (!s_bClothesInit) {
		s_bClothesInit = true;
		CJManager::EnsureClothesSystemInitialized();
	}

	// Process gameplay & environmental states every frame
	GameExtensions::Process();
	TextDrawRotation::Process();
	WaterZones::Process();

	// If RakHook is initialized, process periodic sync & peds
	if (m_bRakHookReady) {
		PacketHandler::Process();
		CJManager::Process();
	}
}

void ExtendedGamePlugin::Shutdown()
{
	ClientLog(LogLevel::Info, "BrownTurbo-ExtendedGame DLL_PROCESS_DETACH");
	if (m_bRakHookReady) {
		PacketHandler::Shutdown();
		rakhook::destroy();
		m_bRakHookReady = false;
	}

	if (m_bEventHooked) {
		MH_DisableHook(reinterpret_cast<void*>(0x53BEE0));
		MH_RemoveHook(reinterpret_cast<void*>(0x53BEE0));
		m_bEventHooked = false;
	}

	if (m_bModel0HookInstalled) {
		WaterZones::Shutdown();
		TextDrawRotation::Shutdown();
		GameExtensions::Shutdown();
		CJManager::Shutdown();
		m_bModel0HookInstalled = false;
	}
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		ExtendedGamePlugin::Instance().Init(hModule);
		break;

	case DLL_PROCESS_DETACH:
		ExtendedGamePlugin::Instance().Shutdown();
		break;
	}
	return TRUE;
}
