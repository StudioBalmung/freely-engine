#pragma once

// Freely Engine 0.4.2 — UUID implementation

#include "Components.h"
#include <random>
#include <chrono>

namespace Freely {

namespace detail {

inline uint64_t GenerateUUID64() {
    // Use a thread-local Mersenne Twister seeded from high-res clock + random_device
    static thread_local std::mt19937_64 s_Engine([]() {
        std::random_device rd;
        auto seed = static_cast<uint64_t>(rd()) ^
            static_cast<uint64_t>(
                std::chrono::high_resolution_clock::now().time_since_epoch().count());
        return seed;
    }());
    std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);
    return dist(s_Engine);
}

} // namespace detail

inline UUID UUID::Generate() {
    return UUID(detail::GenerateUUID64());
}

} // namespace Freely
