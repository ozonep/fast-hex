#pragma once

#include <stdint.h>
#include <stddef.h>


// Decoders
// Decode src hex string into dest bytes
// Optimal AVX2 vectorized version. len is number of dest bytes (1/2 the size of src).
void decodeHexVec(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len);

