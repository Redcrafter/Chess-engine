#pragma once

#if _WIN32 || _WIN64
#include <intrin.h>
#elif __gnu_linux__
#include <x86intrin.h>
#endif

#define popcnt64 _mm_popcnt_u64
#define NumberOfTrailingZeros _tzcnt_u64
