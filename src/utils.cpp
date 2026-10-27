#include "utils.h"
#include "defs.h"

#include <game_sa/CPad.h>
#include <game_sa/CPlayerPed.h>
#include <game_sa/common.h>

#include <atomic>
#include <fstream>
#include <mutex>

static std::atomic<bool> s_clientDebugMode { false };

void SetClientDebugMode(bool enabled)
{
	s_clientDebugMode.store(enabled, std::memory_order_relaxed);
}

bool IsClientDebugMode()
{
	return s_clientDebugMode.load(std::memory_order_relaxed);
}

static const char* LogLevelToString(LogLevel level)
{
	switch (level) {
	case LogLevel::Trace:
		return "TRACE";
	case LogLevel::Debug:
		return "DEBUG";
	case LogLevel::Warning:
		return "WARN";
	case LogLevel::Error:
		return "ERROR";
	case LogLevel::Info:
	case LogLevel::None:
	default:
		return "INFO";
	}
}

void ClientLog(const std::string& msg, LogLevel level)
{
	if (level == LogLevel::None) {
		level = LogLevel::Info;
	}

	if (!IsClientDebugMode() && (level == LogLevel::Debug || level == LogLevel::Trace)) {
		return;
	}

	static std::mutex logMutex;
	std::lock_guard<std::mutex> lock(logMutex);

	SYSTEMTIME st;
	GetLocalTime(&st);
	char prefix[48];
	snprintf(prefix, sizeof(prefix), "[%04u-%02u-%02u %02u:%02u:%02u.%03u]<%s> ",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		LogLevelToString(level));

	std::ofstream log("brownturbo-extendedgame.log", std::ios::app);
	if (log.is_open()) {
		log << prefix << msg << "\n";
		log.flush();
	}
}

void ClientLogf(LogLevel level, const char* fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	ClientLog(std::string(buf), level);
}

bool SendMsg(int color, const char* msg)
{
	rakhook::samp_ver version = rakhook::samp_version();
	bool _sent = false;
	switch (version) {
	case rakhook::samp_ver::v037r1: {
		SAMPAPI_EXPORT sampapi::v037r1::CChat*& p_chat = sampapi::v037r1::RefChat();
		if (p_chat) {
			_sent = true;
			p_chat->AddMessage(color, msg);
		}
		break;
	}
	case rakhook::samp_ver::v037r31: {
		SAMPAPI_EXPORT sampapi::v037r3::CChat*& p_chat = sampapi::v037r3::RefChat();
		if (p_chat) {
			_sent = true;
			p_chat->AddMessage(color, msg);
		}
		break;
	}
	case rakhook::samp_ver::v037r5: {
		SAMPAPI_EXPORT sampapi::v037r5::CChat*& p_chat = sampapi::v037r5::RefChat();
		if (p_chat) {
			_sent = true;
			p_chat->AddMessage(color, msg);
		}
		break;
	}
	case rakhook::samp_ver::v03dlr1: {
		SAMPAPI_EXPORT sampapi::v03dl::CChat*& p_chat = sampapi::v03dl::RefChat();
		if (p_chat) {
			_sent = true;
			p_chat->AddMessage(color, msg);
		}
		break;
	}
	default: {
		_sent = false;
		break;
	}
	}
	return _sent;
}

void ToggleSampCursor(bool enable)
{
	rakhook::samp_ver version = rakhook::samp_version();
	switch (version) {
	case rakhook::samp_ver::v037r1: {
		auto* pGame = sampapi::v037r1::RefGame();
		if (pGame)
			pGame->SetCursorMode(enable ? 3 : 0, !enable);
		break;
	}
	case rakhook::samp_ver::v037r31: {
		auto* pGame = sampapi::v037r3::RefGame();
		if (pGame)
			pGame->SetCursorMode(enable ? 3 : 0, !enable);
		break;
	}
	case rakhook::samp_ver::v037r5: {
		auto* pGame = sampapi::v037r5::RefGame();
		if (pGame)
			pGame->SetCursorMode(enable ? 3 : 0, !enable);
		break;
	}
	case rakhook::samp_ver::v03dlr1: {
		auto* pGame = sampapi::v03dl::RefGame();
		if (pGame)
			pGame->SetCursorMode(enable ? 3 : 0, !enable);
		break;
	}
	default:
		break;
	}

	if (!enable) {
		CPad::ClearMouseHistory();
	}
}

PlayerPoolVariant GetPlayerPoolPtr()
{
	rakhook::samp_ver version = rakhook::samp_version();
	switch (version) {
	case rakhook::samp_ver::v037r1: {
		auto* pNetGame = sampapi::v037r1::RefNetGame();
		if (!pNetGame || !pNetGame->m_pPools || !pNetGame->m_pPools->m_pPlayer)
			return nullptr;
		return pNetGame->m_pPools->m_pPlayer;
	}
	case rakhook::samp_ver::v037r31: {
		auto* pNetGame = sampapi::v037r3::RefNetGame();
		if (!pNetGame || !pNetGame->m_pPools || !pNetGame->m_pPools->m_pPlayer)
			return nullptr;
		return pNetGame->m_pPools->m_pPlayer;
	}
	case rakhook::samp_ver::v037r5: {
		auto* pNetGame = sampapi::v037r5::RefNetGame();
		if (!pNetGame || !pNetGame->m_pPools || !pNetGame->m_pPools->m_pPlayer)
			return nullptr;
		return pNetGame->m_pPools->m_pPlayer;
	}
	case rakhook::samp_ver::v03dlr1: {
		auto* pNetGame = sampapi::v03dl::RefNetGame();
		if (!pNetGame || !pNetGame->m_pPools || !pNetGame->m_pPools->m_pPlayer)
			return nullptr;
		return pNetGame->m_pPools->m_pPlayer;
	}
	default:
		return nullptr;
	}
}

bool IsSampLoaded()
{
	return rakhook::samp_addr() != 0 && rakhook::samp_version() != rakhook::samp_ver::unknown;
}

bool IsGameInitialized()
{
	__try {
		auto ver = rakhook::samp_version();
		switch (ver) {
		case rakhook::samp_ver::v037r1:
			return (sampapi::v037r1::RefGame() != nullptr);
		case rakhook::samp_ver::v037r31:
			return (sampapi::v037r3::RefGame() != nullptr);
		case rakhook::samp_ver::v037r5:
			return (sampapi::v037r5::RefGame() != nullptr);
		case rakhook::samp_ver::v03dlr1:
			return (sampapi::v03dl::RefGame() != nullptr);
		default:
			return false;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

int GetSampVersion()
{
	return static_cast<int>(rakhook::samp_version());
}

uint16_t GetLocalPlayerId()
{
	auto pool = GetPlayerPoolPtr();
	return std::visit([](auto&& p) -> uint16_t {
		using T = std::decay_t<decltype(p)>;
		if constexpr (std::is_same_v<T, std::nullptr_t>) {
			return CJ_LOCAL_PLAYER_ID;
		} else if constexpr (std::is_same_v<T, sampapi::v03dl::CPlayerPool*>) {
			return p ? p->m_nLocalPlayerId : CJ_LOCAL_PLAYER_ID;
		} else {
			return p ? p->m_localInfo.m_nId : CJ_LOCAL_PLAYER_ID;
		}
	}, pool);
}

CPed* GetGamePed(uint16_t playerId)
{
	if (playerId == CJ_LOCAL_PLAYER_ID) {
		return FindPlayerPed(0);
	}

	auto pool = GetPlayerPoolPtr();
	return std::visit([playerId](auto&& p) -> CPed* {
		using T = std::decay_t<decltype(p)>;
		if constexpr (std::is_same_v<T, std::nullptr_t>) {
			return nullptr;
		} else {
			if (!p)
				return nullptr;
			uint16_t localId = CJ_LOCAL_PLAYER_ID;
			if constexpr (std::is_same_v<T, sampapi::v03dl::CPlayerPool*>) {
				localId = p->m_nLocalPlayerId;
			} else {
				localId = p->m_localInfo.m_nId;
			}
			if (playerId == localId) {
				return FindPlayerPed(0);
			}
			if (playerId >= 1004 || !p->IsConnected(playerId)) {
				return nullptr;
			}
			auto* remote = p->GetPlayer(playerId);
			if (remote && remote->m_pPed) {
				return remote->m_pPed->m_pGamePed;
			}
			return nullptr;
		}
	}, pool);
}

bool IsLocalPlayerPed(CPed* pPed)
{
	if (!pPed)
		return false;
	return pPed == FindPlayerPed(0);
}

uint16_t GetPedPlayerId(CPed* pGamePed)
{
	if (!pGamePed)
		return 0xFFFF;
	if (IsLocalPlayerPed(pGamePed))
		return GetLocalPlayerId();

	auto pool = GetPlayerPoolPtr();
	return std::visit([pGamePed](auto&& p) -> uint16_t {
		using T = std::decay_t<decltype(p)>;
		if constexpr (std::is_same_v<T, std::nullptr_t>) {
			return 0xFFFF;
		} else {
			if (!p) return 0xFFFF;
			sampapi::ID id = p->Find(pGamePed);
			if (id < 1004 && p->IsConnected(id))
				return static_cast<uint16_t>(id);
			return 0xFFFF;
		}
	}, pool);
}

void BackupAccessories(CPed* pGamePed, AccessoryBackup& backup)
{
	backup.hasAccessories = false;
	backup.sampPedPtr = nullptr;
	if (!pGamePed || !pGamePed->m_pRwClump)
		return;

	__try {
		auto ver = rakhook::samp_version();
		backup.version = static_cast<uint8_t>(ver);

		switch (ver) {
		case rakhook::samp_ver::v037r1: {
			auto* netGame = sampapi::v037r1::RefNetGame();
			if (!netGame || !netGame->m_pPools || !netGame->m_pPools->m_pPlayer)
				return;
			sampapi::v037r1::CPed* pSampPed = nullptr;
			if (pGamePed == FindPlayerPed(0)) {
				auto* local = netGame->m_pPools->m_pPlayer->GetLocalPlayer();
				if (local)
					pSampPed = local->m_pPed;
			} else {
				sampapi::ID id = netGame->m_pPools->m_pPlayer->Find(pGamePed);
				if (id < sampapi::v037r1::CPlayerPool::MAX_PLAYERS && netGame->m_pPools->m_pPlayer->IsConnected(id)) {
					auto* remote = netGame->m_pPools->m_pPlayer->GetPlayer(id);
					if (remote)
						pSampPed = remote->m_pPed;
				}
			}
			if (pSampPed) {
				for (int i = 0; i < 10; ++i) {
					if (pSampPed->m_accessories.m_bNotEmpty[i]) {
						backup.activeSlots[i] = true;
						memcpy(&backup.data[i * sizeof(sampapi::v037r1::CPed::Accessory)],
							&pSampPed->m_accessories.m_info[i],
							sizeof(sampapi::v037r1::CPed::Accessory));
						backup.hasAccessories = true;
					}
				}
				if (backup.hasAccessories) {
					pSampPed->DeleteAllAccessories();
					backup.sampPedPtr = pSampPed;
				}
			}
			break;
		}
		case rakhook::samp_ver::v037r31: {
			auto* netGame = sampapi::v037r3::RefNetGame();
			if (!netGame || !netGame->m_pPools || !netGame->m_pPools->m_pPlayer)
				return;
			sampapi::v037r3::CPed* pSampPed = nullptr;
			if (pGamePed == FindPlayerPed(0)) {
				auto* local = netGame->m_pPools->m_pPlayer->GetLocalPlayer();
				if (local)
					pSampPed = local->m_pPed;
			} else {
				sampapi::ID id = netGame->m_pPools->m_pPlayer->Find(pGamePed);
				if (id < sampapi::v037r3::CPlayerPool::MAX_PLAYERS && netGame->m_pPools->m_pPlayer->IsConnected(id)) {
					auto* remote = netGame->m_pPools->m_pPlayer->GetPlayer(id);
					if (remote)
						pSampPed = remote->m_pPed;
				}
			}
			if (pSampPed) {
				for (int i = 0; i < 10; ++i) {
					if (pSampPed->m_accessories.m_bNotEmpty[i]) {
						backup.activeSlots[i] = true;
						memcpy(&backup.data[i * sizeof(sampapi::v037r3::CPed::Accessory)],
							&pSampPed->m_accessories.m_info[i],
							sizeof(sampapi::v037r3::CPed::Accessory));
						backup.hasAccessories = true;
					}
				}
				if (backup.hasAccessories) {
					pSampPed->DeleteAllAccessories();
					backup.sampPedPtr = pSampPed;
				}
			}
			break;
		}
		case rakhook::samp_ver::v037r5: {
			auto* netGame = sampapi::v037r5::RefNetGame();
			if (!netGame || !netGame->m_pPools || !netGame->m_pPools->m_pPlayer)
				return;
			sampapi::v037r5::CPed* pSampPed = nullptr;
			if (pGamePed == FindPlayerPed(0)) {
				auto* local = netGame->m_pPools->m_pPlayer->GetLocalPlayer();
				if (local)
					pSampPed = local->m_pPed;
			} else {
				sampapi::ID id = netGame->m_pPools->m_pPlayer->Find(pGamePed);
				if (id < sampapi::v037r5::CPlayerPool::MAX_PLAYERS && netGame->m_pPools->m_pPlayer->IsConnected(id)) {
					auto* remote = netGame->m_pPools->m_pPlayer->GetPlayer(id);
					if (remote)
						pSampPed = remote->m_pPed;
				}
			}
			if (pSampPed) {
				for (int i = 0; i < 10; ++i) {
					if (pSampPed->m_accessories.m_bNotEmpty[i]) {
						backup.activeSlots[i] = true;
						memcpy(&backup.data[i * sizeof(sampapi::v037r5::CPed::Accessory)],
							&pSampPed->m_accessories.m_info[i],
							sizeof(sampapi::v037r5::CPed::Accessory));
						backup.hasAccessories = true;
					}
				}
				if (backup.hasAccessories) {
					pSampPed->DeleteAllAccessories();
					backup.sampPedPtr = pSampPed;
				}
			}
			break;
		}
		case rakhook::samp_ver::v03dlr1: {
			auto* netGame = sampapi::v03dl::RefNetGame();
			if (!netGame || !netGame->m_pPools || !netGame->m_pPools->m_pPlayer)
				return;
			sampapi::v03dl::CPed* pSampPed = nullptr;
			if (pGamePed == FindPlayerPed(0)) {
				auto* local = netGame->m_pPools->m_pPlayer->GetLocalPlayer();
				if (local)
					pSampPed = local->m_pPed;
			} else {
				sampapi::ID id = netGame->m_pPools->m_pPlayer->Find(pGamePed);
				if (id < sampapi::v03dl::CPlayerPool::MAX_PLAYERS && netGame->m_pPools->m_pPlayer->IsConnected(id)) {
					auto* remote = netGame->m_pPools->m_pPlayer->GetPlayer(id);
					if (remote)
						pSampPed = remote->m_pPed;
				}
			}
			if (pSampPed) {
				for (int i = 0; i < 10; ++i) {
					if (pSampPed->m_accessories.m_bNotEmpty[i]) {
						backup.activeSlots[i] = true;
						memcpy(&backup.data[i * sizeof(sampapi::v03dl::CPed::Accessory)],
							&pSampPed->m_accessories.m_info[i],
							sizeof(sampapi::v03dl::CPed::Accessory));
						backup.hasAccessories = true;
					}
				}
				if (backup.hasAccessories) {
					pSampPed->DeleteAllAccessories();
					backup.sampPedPtr = pSampPed;
				}
			}
			break;
		}
		default:
			break;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		backup.hasAccessories = false;
		backup.sampPedPtr = nullptr;
	}
}

void RestoreAccessories(AccessoryBackup& backup)
{
	if (!backup.hasAccessories || !backup.sampPedPtr)
		return;

	__try {
		auto ver = static_cast<rakhook::samp_ver>(backup.version);
		switch (ver) {
		case rakhook::samp_ver::v037r1: {
			auto* pSampPed = reinterpret_cast<sampapi::v037r1::CPed*>(backup.sampPedPtr);
			for (int i = 0; i < 10; ++i) {
				if (backup.activeSlots[i]) {
					auto* pInfo = reinterpret_cast<sampapi::v037r1::CPed::Accessory*>(&backup.data[i * sizeof(sampapi::v037r1::CPed::Accessory)]);
					pSampPed->AddAccessory(i, pInfo);
				}
			}
			break;
		}
		case rakhook::samp_ver::v037r31: {
			auto* pSampPed = reinterpret_cast<sampapi::v037r3::CPed*>(backup.sampPedPtr);
			for (int i = 0; i < 10; ++i) {
				if (backup.activeSlots[i]) {
					auto* pInfo = reinterpret_cast<sampapi::v037r3::CPed::Accessory*>(&backup.data[i * sizeof(sampapi::v037r3::CPed::Accessory)]);
					pSampPed->AddAccessory(i, pInfo);
				}
			}
			break;
		}
		case rakhook::samp_ver::v037r5: {
			auto* pSampPed = reinterpret_cast<sampapi::v037r5::CPed*>(backup.sampPedPtr);
			for (int i = 0; i < 10; ++i) {
				if (backup.activeSlots[i]) {
					auto* pInfo = reinterpret_cast<sampapi::v037r5::CPed::Accessory*>(&backup.data[i * sizeof(sampapi::v037r5::CPed::Accessory)]);
					pSampPed->AddAccessory(i, pInfo);
				}
			}
			break;
		}
		case rakhook::samp_ver::v03dlr1: {
			auto* pSampPed = reinterpret_cast<sampapi::v03dl::CPed*>(backup.sampPedPtr);
			for (int i = 0; i < 10; ++i) {
				if (backup.activeSlots[i]) {
					auto* pInfo = reinterpret_cast<sampapi::v03dl::CPed::Accessory*>(&backup.data[i * sizeof(sampapi::v03dl::CPed::Accessory)]);
					pSampPed->AddAccessory(i, pInfo);
				}
			}
			break;
		}
		default:
			break;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }
	backup.hasAccessories = false;
	backup.sampPedPtr = nullptr;
}
