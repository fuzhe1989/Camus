#pragma once

#include <emmintrin.h>

namespace ds {
inline void cpuPause() {
    __mm_pause();
}
} // namespace ds
