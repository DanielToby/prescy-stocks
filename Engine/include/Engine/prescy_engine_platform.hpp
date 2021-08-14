#pragma once

#if defined(_WIN32)
#if defined(PRESCY_ENGINE_EXPORTS)
#define Prescy_Engine __declspec(dllexport)
#else
#define Prescy_Engine __declspec(dllimport)
#endif

#pragma warning(disable : 4251)
#pragma warning(disable : 4996)
#else
#define Prescy_Engine
#endif
