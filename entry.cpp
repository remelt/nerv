//#define _CRT_SECURE_NO_WARNINGS
#include "main.hpp"
#include <cstdio>
#include <windows.h>
#include "valve/interfaces/vtables/i_mem_alloc.hpp"
#include "directx/directx.hpp"
#include "features/shared/item_schema.hpp"
#include "features/skin_changer/skin_changer.hpp"
#include "features/glove_changer/glove_changer.hpp"

#include "sdk/config_system/config_system.hpp"
#include <thread>

void destroy(HMODULE h_module) {
    g_hooks->destroy();
    logger::shutdown();

    FreeLibraryAndExitThread(h_module, 0);
}

uintptr_t __stdcall start_address(const HMODULE h_module)
{

    try {

        AllocConsole();
        FILE* p_file;
        freopen_s(&p_file, "CONOUT$", "w", stdout);
        logger::initialize();
       
        LOG_INFO("Initializing modules...");
        g_modules->m_modules.initialize();

        LOG_INFO("Initializing interfaces...");
        g_interfaces->initialize();

        LOG_INFO("Initializing directx...");
        g_directx->initialize();

        LOG_INFO("Initializing hooks...");
        g_hooks->initialize();

        g_config_system->auto_load_key();

        g_item_schema->initialize();
      
        g_config_system->auto_load();

        LOG_SUCCESS(u8"Nerv is ready");

#ifdef _DEBUG
        while (!GetAsyncKeyState(VK_END)) {
            Sleep(100);
        }

        g_config_system->reset();
        destroy(h_module);
#endif
    }
    catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Exception", MB_OK | MB_ICONERROR);
    }
    catch (...) {
        MessageBoxA(NULL, "Unknown exception occurred", "Exception", MB_OK | MB_ICONERROR);
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE h_module, DWORD ul_reason_for_call, LPVOID lp_reserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(h_module);

        HANDLE thread = CreateThread(nullptr, 0,
            reinterpret_cast<LPTHREAD_START_ROUTINE>(start_address),
            h_module, 0, nullptr);
        g_config_system->setup_values();

        if (thread != nullptr && thread != INVALID_HANDLE_VALUE) {
            CloseHandle(thread);
            return TRUE;
        }

        return FALSE;
    }

    return TRUE;
}