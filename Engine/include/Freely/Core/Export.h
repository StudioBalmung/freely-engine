#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(FREELY_ENGINE_EXPORTS)
        // When building the DLL
        #define FREELY_API __declspec(dllexport)
    #else
        // When using the DLL
        #define FREELY_API __declspec(dllimport)
    #endif
#else
    // Non‑Windows platforms can use visibility attributes if desired
    #if __GNUC__ >= 4
        #define FREELY_API __attribute__((visibility("default")))
    #else
        #define FREELY_API
    #endif
#endif
