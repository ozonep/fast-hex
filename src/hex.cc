#include "hex.h"

#include <immintrin.h>


static inline int8_t unhexBitManip(uint8_t x) {
  return 9 * (x >> 6) + (x & 0xf);
}

static const __m256i _9 = _mm256_set1_epi16(9);
static const __m256i _15 = _mm256_set1_epi16(0xf);

inline static __m256i unhexBitManip(const __m256i value) {
  __m256i and15 = _mm256_and_si256(value, _15);

#ifndef NO_MADDUBS
  __m256i sr6 = _mm256_srai_epi16(value, 6);
  __m256i mul = _mm256_maddubs_epi16(sr6, _9);
#else
  __m256i sr6_lo2 = _mm256_and_si256(_mm256_srli_epi16(value, 6), _mm256_set1_epi16(0b11));
  __m256i sr6_lo2_sl3 = _mm256_slli_epi16(sr6_lo2, 3);
  __m256i mul = _mm256_or_si256(sr6_lo2_sl3, sr6_lo2);
#endif

  __m256i add = _mm256_add_epi16(mul, and15);
  return add;
}

static const char hex_table[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};
inline static char hex(uint8_t value) { return hex_table[value]; }
static const __m256i HEX_LUTR = _mm256_setr_epi8(
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f');
inline static __m256i hex(__m256i value) {
  return _mm256_shuffle_epi8(HEX_LUTR, value);
}

inline static __m256i nib2byte(__m256i a1, __m256i b1, __m256i a2, __m256i b2) {
  __m256i a4_1 = _mm256_slli_epi16(a1, 4);
  __m256i a4_2 = _mm256_slli_epi16(a2, 4);
  __m256i a4orb_1 = _mm256_or_si256(a4_1, b1);
  __m256i a4orb_2 = _mm256_or_si256(a4_2, b2);
  __m256i pck1 = _mm256_packus_epi16(a4orb_1, a4orb_2);
  const int _0213 = 0b11'01'10'00;
  __m256i pck64 = _mm256_permute4x64_epi64(pck1, _0213);
  return pck64;
}

static const __m256i ROT2 = _mm256_setr_epi8(
  -1, 0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14,
  -1, 0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14
);

inline static __m256i byte2nib(__m128i val) {
  __m256i doubled = _mm256_cvtepu8_epi16(val);
  __m256i hi = _mm256_srli_epi16(doubled, 4);
  __m256i lo = _mm256_shuffle_epi8(doubled, ROT2);
  __m256i bytes = _mm256_or_si256(hi, lo);
  bytes = _mm256_and_si256(bytes, _mm256_set1_epi8(0b1111));
  return bytes;
}

void decodeHexBMI(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  for (size_t i = 0; i < len; i++) {
    uint8_t a = *src++;
    uint8_t b = *src++;
    a = unhexBitManip(a);
    b = unhexBitManip(b);
    dest[i] = (a << 4) | b;
  }
}

void decodeHexVec(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t len) {
  const __m256i A_MASK = _mm256_setr_epi8(
    0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14, -1,
    0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14, -1);
  const __m256i B_MASK = _mm256_setr_epi8(
    1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, 13, -1, 15, -1,
    1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, 13, -1, 15, -1);

  const __m256i* val3 = reinterpret_cast<const __m256i*>(src);
  __m256i* dec256 = reinterpret_cast<__m256i*>(dest);

  while (len >= 32) {
    __m256i av1 = _mm256_lddqu_si256(val3++); // 32 nibbles, 16 bytes
    __m256i av2 = _mm256_lddqu_si256(val3++);
                                                // Separate high and low nibbles and extend into 16-bit elements
    __m256i a1 = _mm256_shuffle_epi8(av1, A_MASK);
    __m256i b1 = _mm256_shuffle_epi8(av1, B_MASK);
    __m256i a2 = _mm256_shuffle_epi8(av2, A_MASK);
    __m256i b2 = _mm256_shuffle_epi8(av2, B_MASK);

    a1 = unhexBitManip(a1);
    a2 = unhexBitManip(a2);
    b1 = unhexBitManip(b1);
    b2 = unhexBitManip(b2);

    __m256i bytes = nib2byte(a1, b1, a2, b2);

    _mm256_storeu_si256(dec256++, bytes);
    len -= 32;
  }

  src = reinterpret_cast<const uint8_t*>(val3);
  dest = reinterpret_cast<uint8_t*>(dec256);
  decodeHexBMI(dest, src, len);
}
