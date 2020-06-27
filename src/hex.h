#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(_MSC_VER)
#define __restrict__ __restrict  // The C99 keyword, available as a C++ extension
#endif

// Decoders
// Decode src hex string into dest bytes
// Optimal AVX2 vectorized version. len is number of dest bytes (1/2 the size of src).
void decodeHexVec(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len);

