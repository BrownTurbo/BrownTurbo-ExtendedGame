#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "cj_manager.h"
#include "packet_handler.h"

class ExtendedGamePlugin {
public:
    static ExtendedGamePlugin& Instance() {
        static ExtendedGamePlugin instance;
        return instance;
    }

    void Init(HMODULE hModule);
    void Shutdown();
    void OnGameProcess();
    void InitializeRakHookThread();

    bool IsModel0HookInstalled() const { return m_bModel0HookInstalled; }
    bool IsRakHookReady() const { return m_bRakHookReady; }
    HMODULE GetPluginModule() const { return m_hModule; }

private:
    ExtendedGamePlugin() = default;
    ~ExtendedGamePlugin() = default;
    ExtendedGamePlugin(const ExtendedGamePlugin&) = delete;
    ExtendedGamePlugin& operator=(const ExtendedGamePlugin&) = delete;

    HMODULE m_hModule = nullptr;
    bool m_bModel0HookInstalled = false;
    bool m_bRakHookReady = false;
    bool m_bEventHooked = false;
};
