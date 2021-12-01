#pragma once
#include <cstdint>

inline uint32_t reverseBits(uint32_t x) {
    uint32_t result;
    asm ("rbit %0, %1"
        : "=r" (result)
        : "r" (x)
        );
    return result;
}
