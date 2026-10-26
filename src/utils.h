#pragma once

#include <sampapi/CChat.h>
#include <sampapi/CGame.h>
#include <sampapi/CLocalPlayer.h>
#include <sampapi/CNetGame.h>
#include <sampapi/CPlayerPool.h>
#include <sampapi/sampapi.h>

#include <RakHook/rakhook.hpp>
#include <RakHook/samp.hpp>

#include <windows.h>
#include <cstdint>
#include <cstdarg>
#include <format>
#include <string>
#include <string_view>
#include <variant>

// Forward declarations of GTA:SA types
class CPed;
class CPlayerPed;

enum class LogLevel : uint8_t {
	None = 0,
	Trace,
	Debug,
	Info,
	Warning,
	Error
};
using ClientLogLevel = LogLevel;

void ClientLog(const std::string& msg, LogLevel level = LogLevel::None);
inline void ClientLog(LogLevel level, const std::string& msg) { ClientLog(msg, level); }

template <typename... Args>
inline void ClientLog(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
{
	ClientLog(level, std::format(fmt, std::forward<Args>(args)...));
}

void ClientLogf(LogLevel level, const char* fmt, ...);
inline void ClientLogf(const char* fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	ClientLog(LogLevel::Info, std::string(buf));
}

void SetClientDebugMode(bool enabled);
bool IsClientDebugMode();

bool SendMsg(int color, const char* msg);
void ToggleSampCursor(bool enable);

using PlayerPoolVariant = std::variant<
	std::nullptr_t,
	sampapi::v037r1::CPlayerPool*,
	sampapi::v037r3::CPlayerPool*,
	sampapi::v037r5::CPlayerPool*,
	sampapi::v03dl::CPlayerPool*>;

PlayerPoolVariant GetPlayerPoolPtr();
bool IsSampLoaded();
bool IsGameInitialized();
int GetSampVersion();
uint16_t GetLocalPlayerId();

CPed* GetGamePed(uint16_t playerId);
bool IsLocalPlayerPed(CPed* pPed);

struct AccessoryBackup {
	bool hasAccessories = false;
	uint8_t version = 0;
	void* sampPedPtr = nullptr;
	bool activeSlots[10] = { false };
	uint8_t data[10 * 64] = { 0 };
};

void BackupAccessories(CPed* pGamePed, AccessoryBackup& backup);
void RestoreAccessories(AccessoryBackup& backup);

// Backwards compatibility namespace
namespace SAMPUtils {
	inline void Log(const std::string& msg) { ClientLog(LogLevel::Info, msg); }
	inline void Logf(const char* fmt, ...) {
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);
		ClientLog(LogLevel::Info, std::string(buf));
	}
	inline bool IsSampLoaded() { return ::IsSampLoaded(); }
	inline bool IsGameInitialized() { return ::IsGameInitialized(); }
	inline int GetSampVersion() { return ::GetSampVersion(); }
	inline uint16_t GetLocalPlayerId() { return ::GetLocalPlayerId(); }
	inline CPed* GetGamePed(uint16_t playerId) { return ::GetGamePed(playerId); }
	inline bool IsLocalPlayerPed(CPed* pPed) { return ::IsLocalPlayerPed(pPed); }
	using AccessoryBackup = ::AccessoryBackup;
	inline void BackupAccessories(CPed* pGamePed, AccessoryBackup& backup) { ::BackupAccessories(pGamePed, backup); }
	inline void RestoreAccessories(AccessoryBackup& backup) { ::RestoreAccessories(backup); }
	inline void LogToChat(const char* szMessage, uint32_t color = 0x33CCFFAA) { ::SendMsg(static_cast<int>(color), szMessage); }
}
