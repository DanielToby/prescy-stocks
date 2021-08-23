#include <Engine/prescy_engine_platform.hpp>

#include <src/log.hpp>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#undef WIN32_LEAN_AND_MEAN

BOOL APIENTRY DllMain([[maybe_unused]] HMODULE hModule,
                      DWORD reason,
                      [[maybe_unused]] LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        prescy::Log::init();
    }
    return TRUE;
}

#endif

#ifdef __APPLE__

static void con() __attribute__((constructor));
void con() {
    prescy::Log::init();
}

#endif
