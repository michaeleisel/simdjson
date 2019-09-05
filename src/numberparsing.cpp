//Copyright (c) 2018 Michael Eisel. All rights reserved.

#include <stdio.h>
#include <float.h>  // float numeric-limit macros
#include <stdint.h>
#include <assert.h>  // C11 static assert
#include <string.h>  // memcpy

#include "simdjson/numberparsing.h"
#include <rapidjson/internal/strtod.h>

// credit to Peter Cordes, https://stackoverflow.com/questions/57499986/how-to-get-the-mantissa-of-an-80-bit-long-double-as-an-int-on-x86-64
really_inline uint64_t ldbl_mant(long double x)
{
  // we can assume CHAR_BIT = 8 when targeting x86, unless you care about DeathStation 9000 implementations.
  static_assert( sizeof(long double) >= 10, "x87 long double must be >= 10 bytes" );
  static_assert( LDBL_MANT_DIG == 64, "x87 long double significand must be 64 bits" );

  uint64_t retval;
  memcpy(&retval, &x, sizeof(retval));
  static_assert( sizeof(retval) < sizeof(x), "uint64_t should be strictly smaller than long double" ); // sanity check for wrong types
  return retval;
}

static uint64_t case1 = 0, case2 = 0, case3 = 0;
static uint64_t z = 0;

void inc() {
  z = (z + 1) % 6;
}

static char buffer[40];

uint64_t c1() {
  return case1;
}

uint64_t c2() {
  return case2;
}

uint64_t c3() {
  return case3;
}

uint64_t dtoi(double d) {
  return *(uint64_t *)&d;
}

uint64_t dtoexp(double d) {
  uint64_t i = dtoi(d);
  return (i >> 52) & ((1 << 12) - 1);
}

uint64_t dtosig(double d) {
  uint64_t i = dtoi(d);
  return i & ((1ULL << 54) - 1);
}

bool dtoneg(double d) {
  return dtoi(d) >> 63;
}

char *itorange(uint64_t ll, uint64_t start, uint64_t length) {
  char *str = (char *)malloc(length + 1);
  for (int i = 0; i < length; i++) {
    str[i] = '0' + ((ll >> (63 - (i + start))) & 1);
  }
  str[length] = '\0';
  return str;
}

char *ltob(__uint128_t ll) {
  char *str = (char *)malloc(128 + 1);
  for (int i = 0; i < 128; i++) {
    str[i] = '0' + ((ll >> (127 - i)) & 1);
  }
  str[128] = '\0';
  return str;
}

char *itob(uint64_t ll) {
  return itorange(ll, 0, 64);
}

char *dtoc(double d) {
  char *c = NULL;
  asprintf(&c, "%s, exp: %lld, sig: %s", (dtoneg(d) ? "negative" : "non-negative"), dtoexp(d), itorange(dtoi(d), 12, 52));
  return c;
}

namespace simdjson {
  static const uint32_t powers_ext[] = {0x6fb92487, 0xa5d3b6d4, 0x8f48a489, 0x331acdab, 0x9ff0c08b, 0x7ecf0ae, 0xc9e82cd9, 0xbe311c08, 0x6dbd630a, 0x92cbbcc, 0x25bbf560, 0xaf2af2b8, 0x1af5af66, 0x50d98d9f, 0xe50ff107, 0x1e53ed49, 0x25e8e89c, 0x77b19161, 0xd59df5b9, 0x4b057328, 0x4ee367f9, 0x229c41f7, 0x6b435275, 0x830a1389, 0x23cc986b, 0x2cbfbe86, 0x7bf7d714, 0xdaf5ccd9, 0xd1b3400f, 0x23100809, 0xabd40a0c, 0x16c90c8f, 0xae3da7d9, 0x99cd11cf, 0x40405643, 0x482835ea, 0xda324365, 0x90bed43e, 0x5a7744a6, 0x711515d0, 0xd5a5b44, 0xe858790a, 0x626e974d, 0xfb0a3d21, 0x7ce66634, 0x1c1fffc1, 0xa327ffb2, 0x4bf1ff9f, 0x6f773fc3, 0xcb550fb4, 0x7e2a53a1, 0x2eda7444, 0xfa911155, 0x793555ab, 0x4bc1558b, 0x9eb1aaed, 0x465e15a9, 0xbfacd89, 0xcef980ec, 0x82b7e127, 0xd1b2ecb8, 0x861fa7e6, 0x67a791e0, 0xe0c8bb2c, 0x58fae9f7, 0xaf39a475, 0x6d8406c9, 0xc8e5087b, 0xfb1e4a9a, 0x5cf2eea0, 0xf42faa48, 0xf13b94da, 0x76c53d08, 0x54768c4b, 0xa9942f5d, 0xd3f93b35, 0xc47bc501, 0x359ab641, 0xc30163d2, 0x79e0de63, 0x985915fc, 0x3e6f5b7b, 0xa705992c, 0x50c6ff78, 0xa4f8bf56, 0x871b7795, 0x28e2557b, 0x331aeada, 0x3ff0d2c8, 0xfed077a, 0xd3e84959, 0x64712dd7, 0xbd8d794d, 0xecf0d7a0, 0xf41686c4, 0x311c2875, 0x7d633293, 0xae5dff9c, 0xd9f57f83, 0xd072df63, 0x4247cb9e, 0x52d9be85, 0x67902e27, 0xba1cd8, 0x80e8a40e, 0x6122cd12, 0x796b8057, 0xcbe33036, 0xbedbfc44, 0xee92fb55, 0x751bdd15, 0xd262d45a, 0x86fb8971, 0xd45d35e6, 0x89748360, 0x2bd1a438, 0x7b6306a3, 0x1a3bc84c, 0x20caba5f, 0x547eb47b, 0xe99e619a, 0x6405fa00, 0xde83bc40, 0x9624ab50, 0x3badd624, 0xe54ca5d7, 0x5e9fcf4c, 0x7647c320, 0x29ecd9f4, 0xf4681071, 0x7182148d, 0xc6f14cd8, 0xb8ada00e, 0xa6d90811, 0x908f4a16, 0x9a598e4e, 0x40eff1e1, 0xd12bee59, 0x82bb74f8, 0xe36a5236, 0xdc44e6c3, 0x29ab103a, 0x7415d448, 0x111b495b, 0xcab10dd9, 0x3d5d514f, 0xcb4a5a3, 0x47f0e785, 0x59ed2167, 0x306869c1, 0x1e414218, 0xe5d1929e, 0xdf45f746, 0x6b8bba8c, 0x66ea92f, 0xc80a537b, 0xbd06742c, 0x2c481138, 0xf75a1586, 0x9a984d73, 0xc13e60d0, 0x318df905, 0xfdf17746, 0xfeb6ea8b, 0xfe64a52e, 0x3dfdce7a, 0x6bea10c, 0x486e494f, 0x5a89dba3, 0xf8962946, 0xf6bbb397, 0x746aa07d, 0xa8c2a44e, 0x92f34d62, 0x77b020ba, 0xace1474, 0xd819992, 0x10e1fff6, 0xca8d3ffa, 0xbd308ff8, 0xac7cb3f6, 0x6bcdf07a, 0x86c16c98, 0xe871c7bf, 0x11471cd7, 0xd598e40d, 0x4aff1d10, 0xcedf722a, 0xc2974eb4, 0x733d2262, 0x806357d, 0xca07c2dc, 0xfc89b393, 0xbbac2078, 0xd54b944b, 0xa9e795e, 0x4d4617b5, 0x504bced1, 0xe45ec286, 0x5d767327, 0x3a6a07f8, 0x890489f7, 0x2b45ac74, 0x3b0b8bc9, 0x9ce6ebb, 0xcc420a6a, 0x9fa94682, 0x47939822, 0x59787e2b, 0x57eb4edb, 0xede62292, 0xe95fab36, 0x11dbcb02, 0xd652bdc2, 0x4be76d33, 0x6f70a440, 0xcb4ccd50, 0x7e2000a4, 0x8ed40066, 0x72890080, 0x4f2b40a0, 0xe2f610c8, 0xdd9ca7d, 0x91503d1c, 0x75a44c63, 0xc986afbe, 0xfbe85bad, 0xfae27299, 0xdccd879f, 0x5400e987, 0x290123e9, 0xf9a0b672, 0xf808e40e, 0xb60b1d12, 0xb1c6f22b, 0x1e38aeb6, 0x25c6da63, 0x579c487e, 0x2d835a9d, 0xf8e43145, 0x1b8e9ecb, 0xe272467e, 0x5b0ed81d, 0x98e94712, 0x3f2398d7, 0x8eec7f0d, 0x1953cf68, 0x5fa8c342, 0x3792f412, 0xe2bbd88b, 0x5b6aceae, 0xf245825a, 0xeed6e2f0, 0x55464dd6, 0xaa97e14c, 0xd53dd99f, 0xe546a803, 0xde985204, 0x963e6685, 0xdde70013, 0x5560c018, 0xaab8f01e, 0xcab39613, 0x3d607b97, 0x8cb89a7d, 0x77f3608e, 0x55f038b2, 0x6b6c46de, 0x2323ac4b, 0xabec975e, 0x96e7bd35, 0x7e50d641, 0xdde50bd1, 0x955e4ec6, 0xbd5af13b, 0xecb1ad8a, 0x67de18ed, 0x80eacf94, 0xa1258379, 0x96ee458, 0x8bca9d6e, 0x775ea264, 0x95364afe, 0x3a83ddbd, 0xc4926a96, 0x75b7053c, 0x5324c68b, 0xd3f6fc16, 0x88f4bb1c, 0x2b31e9e3, 0x3aff322e, 0x9befeb9, 0x4c2ebe68, 0xf9d3701, 0x538484c1, 0x2865a5f2, 0xf93f87b7, 0xf78f69a5, 0xb573440e, 0x31680a88, 0xfdc20d2b, 0x3d329076, 0xa63f9a49, 0xfcf80dc, 0xd3c36113, 0x645a1cac, 0x3d70a3d7, 0xcccccccc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40000000, 0x50000000, 0xa4000000, 0x4d000000, 0xf0200000, 0x6c280000, 0xc7320000, 0x3c7f4000, 0x4b9f1000, 0x1e86d400, 0x13144480, 0x17d955a0, 0x5dcfab08, 0x5aa1cae5, 0xf14a3d9e, 0x6d9ccd05, 0xe4820023, 0xdda2802c, 0xd50b2037, 0x4526f422, 0x9670b12b, 0x3c0cdd76, 0xa5880a69, 0x8eea0d04, 0x72a49045, 0x47a6da2b, 0x999090b6, 0xfff4b4e3, 0xbff8f10e, 0xaff72d52, 0x9bf4f8a6, 0x2f236d0, 0x1d76242, 0x424d3ad2, 0xd2e08987, 0x63cc55f4, 0x3cbf6b71, 0x8bef464e, 0x97758bf0, 0x3d52eeed, 0x4ca7aaa8, 0x8fe8caa9, 0xb3e2fd53, 0x60dbbca8, 0xbc8955e9, 0x6babab63, 0xc696963c, 0xfc1e1de5, 0x3b25a55f, 0x49ef0eb7, 0x6e356932, 0x49c2c37f, 0xdc33745e, 0x69a028bb, 0xc40832ea, 0xf50a3fa4, 0x792667c6, 0x577001b8, 0xed4c0226, 0x544f8158, 0x696361ae, 0x3bc3a19, 0x4ab48a0, 0x62eb0d64, 0x3ba5d0bd, 0xca8f44ec, 0x7e998b13, 0x9e3fedd8, 0xc5cfe94e, 0xbba1f1d1, 0x2a8a6e45, 0xf52d09d7, 0x593c2626, 0x6f8b2fb0, 0xb6dfb9c, 0x4724bd41, 0x58edec91, 0x2f2967b6, 0xbd79e0d2, 0xecd85906, 0xe80e6f48, 0x3109058d, 0xbd4b46f0, 0x6c9e18ac, 0x3e2cf6b, 0x84db8346, 0xe6126418, 0x4fcb7e8f, 0xe3be5e33, 0x5cadf5bf, 0x73d9732f, 0x2867e7fd, 0xb281e1fd, 0x1f225a7c, 0x3375788d, 0x52d6b1, 0xc0678c5d, 0xf840b7ba, 0xb650e5a9, 0xa3e51f13, 0xc66f336c, 0xb80b0047, 0xa60dc059, 0x87c89837, 0x29babe45, 0xf4296dd6, 0x1899e4a6, 0x5ec05dcf, 0x76707543, 0x6a06494a, 0x487db9d, 0x45a9d284, 0xb8a2392, 0x8e6cac77, 0x3207d795, 0x7f44e6bd, 0x5f16206c, 0x36dba887, 0xc2494954, 0xf2db9baa, 0x6f928294, 0xcb772339, 0xff2a7604, 0xfef51385, 0x7eb25866, 0xef2f773f, 0xaafb550f, 0x95ba2a53, 0xdd945a74, 0x94f97111, 0x7a37cd56, 0xac62e055, 0x577b986b, 0xed5a7e85, 0x14588f13, 0x596eb2d8, 0x6fca5f8e, 0x25de7bb9, 0xaf561aa7, 0x1b2ba151, 0x90fb44d2, 0x353a1607, 0x42889b89, 0x69956135, 0x43fab983, 0x94f967e4, 0x1d1be0ee, 0x6462d92a, 0x7d7b8f75, 0x5cda7352, 0x3a088813, 0x88aaa18, 0x8aad549e, 0x36ac54e2, 0x84576a1b, 0x656d44a2, 0x9f644ae5, 0x873d5d9f, 0xa90cb506, 0x9a7f124, 0xc11ed6d, 0x8f1668c8, 0xf96e017d, 0x37c981dc, 0x85bbe253, 0x93956d74, 0x387ac8d1, 0x6997b05, 0x441fece3, 0xd527e81c, 0x8a71e223, 0xf6872d56, 0xb428f8ac, 0xe13336d7, 0xecc00246, 0x27f002d7, 0x31ec038d, 0x7e670471, 0xf0062c6, 0x52c07b78, 0xa7709a56, 0x88a66076, 0x6acff893, 0x583f6b8, 0xc3727a33, 0x744f18c0, 0x1162def0, 0x8addcb56, 0x6d953e2b, 0xc8fa8db6, 0x1d9c9892, 0x2503beb6, 0x2e44ae64, 0x5ceaecfe, 0x7425a83e, 0xd12f124e, 0x82bd6b70, 0x636cc64d, 0x3c47f7e0, 0x65acfaec, 0x7f1839a7, 0x1ede4811, 0x934aed0a, 0xf81da84d, 0x36251260, 0xc1d72b7c, 0xb24cf65b, 0xdee033f2, 0x169840ef, 0x8e1f2895, 0xf1a6f2ba, 0xae10af69, 0xacca6da1, 0x17fd090a, 0xddfc4b4c, 0x4abdaf10, 0x9d6d1ad4, 0x84c86189, 0x32fd3cf5, 0x3fbc8c33, 0xfabaf3f, 0x29cb4d87, 0x743e20e9, 0x914da924, 0x1ad089b6, 0xa184ac24, 0xc9e5d72d, 0x7e2fa67c, 0xddbb901b, 0x552a7422, 0xd53a8895, 0x8a892aba, 0x2d2b7569, 0x9c3b2962, 0x8349f3ba, 0x241c70a9, 0xed238cd3, 0xf4363804, 0xb143c605, 0xdd94b786, 0xca7cf2b4, 0xfd1c2f61, 0xbc633b39, 0xd5be0503, 0x4b2d8644, 0xddf8e7d6, 0xcabb90e5, 0x3d6a751f, 0xcc51267, 0x27fb2b80, 0xb1f9f660, 0x5e7873f8, 0xdb0b487b, 0x91ce1a9a, 0x7641a140, 0xa9e904c8, 0x546345fa, 0xa97c1779, 0x49ed8eab, 0x5c68f256, 0x73832eec, 0xc831fd53, 0xba3e7ca8, 0x28ce1bd2, 0x7980d163, 0xd7e105bc, 0x8dd9472b, 0xb14f98f6, 0x6ed1bf9a, 0xa862f80, 0xcd27bb61, 0x8038d51c, 0xe0470a63, 0x1858ccfc, 0xf37801e, 0xd3056025, 0x47c6b82e, 0x4cdc331d, 0xe0133fe4, 0x58180fdd, 0x570f09ea};


// handle case where strtod finds an invalid number. won't we have a buffer overflow if it's just numbers past the end?
double compute_float_64(uint64_t power_index, uint64_t i, bool negative) {
  if (i == 0) {
    return 0;
  }
  components c = power_of_ten_components[power_index];
  uint64_t factor_mantissa = c.mantissa;
  int lz = leading_zeroes(i);
  i <<= lz;
  __uint128_t large_mantissa = (__uint128_t)i * factor_mantissa;
  uint64_t upper = large_mantissa >> 64;
  uint64_t lower = large_mantissa;
  uint64_t justLastBit = upper & (1ULL << 63);
  if (unlikely((upper & 0x1FF) == 0x1FF)) {//} || (justLastBit && ((upper & 0x3FF) != 0x3FF))) {
    return NAN;
  }
  uint64_t mantissa = 0;
  if (justLastBit) {
    mantissa = upper >> 10;
  } else {
    mantissa = upper >> 9;
    lz++;
  }
  mantissa += mantissa & 1;
  mantissa >>= 1;
  mantissa &= ~(1ULL << 52);
  uint64_t real_exponent = c.exp + 1023 + (127 - lz);
  mantissa |= real_exponent << 52;
  mantissa |= ((uint64_t)negative) << 63; // Assumes negative is in [0, 1]
  double d = 0;
  memcpy(&d, &mantissa, sizeof(d));
  return d;
}

  static const int powersOf10[] = {1, 10, 100, 1000};

  double compute_float_128(uint64_t power_index, uint64_t i_start, int16_t i_end, int digits_in_i_end, bool negative, bool i_is_accurate) {
    components c = power_of_ten_components[power_index];
    uint64_t factor_mantissa = c.mantissa;
    __uint128_t i = i_start;
    i = i * ((int)power_of_ten[digits_in_i_end + 308]) + i_end;
    if (i == 0) {
      return 0;
    }
    uint64_t i_upper = i >> 64;
    int i_lz = i_upper ? leading_zeroes(i_upper) : 64 + leading_zeroes((uint64_t)i);
    int i_shift = i_lz - 48;
    i <<= i_shift;
    __uint128_t m = ((__uint128_t)factor_mantissa << 32) | powers_ext[power_index];
    const uint64_t first_forty_eight_mask = (1ULL << 48) - 1;
    __uint128_t lower_part = i * (m & first_forty_eight_mask);
    __uint128_t upper128 = i * (m >> 48);
    uint64_t upper = upper128 >> (128 - 48);
    __uint128_t lower = lower_part + (upper128 << 48);
    if (lower < lower_part) { // overflow occurred
      upper++;
    }
    int lz = leading_zeroes(upper);
    int safeBits = 192 - lz - 54;
    __uint128_t max_inaccuracy = i;
    if (!i_is_accurate) {
      max_inaccuracy += (m + 1) << i_shift;
    }
    __uint128_t max_lower = lower + max_inaccuracy;
    __uint128_t unsafe_mask = (~((__uint128_t)0)) << safeBits;
    if ((max_lower & unsafe_mask) != (lower & unsafe_mask)) {
      return NAN;
    }
    uint64_t mantissa = upper << (128 - safeBits) | lower >> safeBits;
    mantissa += mantissa & 1;
    mantissa >>= 1;
    mantissa &= ~(1ULL << 52);
    uint64_t real_exponent = c.exp - 32 - i_shift + 1023 + safeBits + 53;
    mantissa |= real_exponent << 52; // lz > 64?
    mantissa |= ((uint64_t)negative) << 63; // is this safe? is this bool in [0, 1]?
    double d = 0;
    memcpy(&d, &mantissa, sizeof(d));
    return d;
}

// parse the number at buf + offset
// define JSON_TEST_NUMBERS for unit testing
//
// It is assumed that the number is followed by a structural ({,},],[) character
// or a white space character. If that is not the case (e.g., when the JSON
// document is made of a single number), then it is necessary to copy the
// content and append a space before calling this function.
//
// Our objective is accurate parsing (ULP of 0 or 1) at high speed.
bool parse_number(const uint8_t *const buf, ParsedJson &pj,
                                       const uint32_t offset,
                                       bool found_minus) {
#ifdef SIMDJSON_SKIPNUMBERPARSING // for performance analysis, it is sometimes
                                  // useful to skip parsing
  pj.write_tape_s64(0);           // always write zero
  return true;                    // always succeeds
#else
  const char *p = reinterpret_cast<const char *>(buf + offset);
  bool negative = false;
  if (found_minus) {
    ++p;
    negative = true;
    if (!is_integer(*p)) { // a negative sign must be followed by an integer
#ifdef JSON_TEST_NUMBERS // for unit testing
      found_invalid_number(buf + offset);
#endif
      return false;
    }
  }
  const char *const start_digits = p;

  uint64_t i;      // an unsigned int avoids signed overflows (which are bad)
  if (*p == '0') { // 0 cannot be followed by an integer
    ++p;
    if (is_not_structural_or_whitespace_or_exponent_or_decimal(*p)) {
#ifdef JSON_TEST_NUMBERS // for unit testing
      found_invalid_number(buf + offset);
#endif
      return false;
    }
    i = 0;
  } else {
    if (!(is_integer(*p))) { // must start with an integer
#ifdef JSON_TEST_NUMBERS // for unit testing
      found_invalid_number(buf + offset);
#endif
      return false;
    }
    unsigned char digit = *p - '0';
    i = digit;
    p++;
    // the is_made_of_eight_digits_fast routine is unlikely to help here because
    // we rarely see large integer parts like 123456789
    while (is_integer(*p)) {
      digit = *p - '0';
      // a multiplication by 10 is cheaper than an arbitrary integer
      // multiplication
      i = 10 * i + digit; // might overflow, we will handle the overflow later
      ++p;
    }
  }
  int64_t exponent = 0;
  bool is_float = false;
  if ('.' == *p) {
    is_float = true; // At this point we know that we have a float
                     // we continue with the fiction that we have an integer. If the
                     // floating point number is representable as x * 10^z for some integer
                     // z that fits in 53 bits, then we will be able to convert back the
                     // the integer into a float in a lossless manner.
    ++p;
    const char *const first_after_period = p;
    if (is_integer(*p)) {
      unsigned char digit = *p - '0';
      ++p;
      i = i * 10 + digit; // might overflow + multiplication by 10 is likely
                          // cheaper than arbitrary mult.
                          // we will handle the overflow later
    } else {
#ifdef JSON_TEST_NUMBERS // for unit testing
      found_invalid_number(buf + offset);
#endif
      return false;
    }
#ifdef SWAR_NUMBER_PARSING
    // this helps if we have lots of decimals!
    // this turns out to be frequent enough.
    if (is_made_of_eight_digits_fast(p)) {
      i = i * 100000000 + parse_eight_digits_unrolled(p);
      p += 8;
    }
#endif
    while (is_integer(*p)) {
      unsigned char digit = *p - '0';
      ++p;
      i = i * 10 + digit; // in rare cases, this will overflow, but that's ok
                          // because we have parse_highprecision_float later.
    }
    exponent = first_after_period - p;
  }
  int digit_count =
    p - start_digits - 1; // used later to guard against overflows
  int64_t exp_number = 0;   // exponential part
  if (('e' == *p) || ('E' == *p)) {
    is_float = true;
    ++p;
    bool neg_exp = false;
    if ('-' == *p) {
      neg_exp = true;
      ++p;
    } else if ('+' == *p) {
      ++p;
    }
    if (!is_integer(*p)) {
#ifdef JSON_TEST_NUMBERS // for unit testing
      found_invalid_number(buf + offset);
#endif
      return false;
    }
    unsigned char digit = *p - '0';
    exp_number = digit;
    p++;
    if (is_integer(*p)) {
      digit = *p - '0';
      exp_number = 10 * exp_number + digit;
      ++p;
    }
    if (is_integer(*p)) {
      digit = *p - '0';
      exp_number = 10 * exp_number + digit;
      ++p;
    }
    while (is_integer(*p)) {
      if (exp_number > 0x100000000) { // we need to check for overflows
                                      // we refuse to parse this
#ifdef JSON_TEST_NUMBERS // for unit testing
        found_invalid_number(buf + offset);
#endif
        return false;
      }
      digit = *p - '0';
      exp_number = 10 * exp_number + digit;
      ++p;
    }
    exponent += (neg_exp ? -exp_number : exp_number);
  }
  if (is_float) {
    uint64_t power_index = 308 + exponent;
    if (unlikely((digit_count >= 19))) { // this is uncommon
                                         // It is possible that the integer had an overflow.
                                         // We have to handle the case where we have 0.0000somenumber.
      const char *start = start_digits;
      while ((*start == '0') || (*start == '.')) {
        start++;
      }
      // we over-decrement by one when there is a '.'
      digit_count -= (start - start_digits);
      if (digit_count >= 19) {
        // Ok, chances are good that we had an overflow!
        // this is almost never going to get called!!!
        // we start anew, going slowly!!!
        return parse_float(buf, pj, offset, found_minus);
      }
    }
    if (unlikely((power_index > 2 * 308))) { // this is uncommon!!!
                                             // this is almost never going to get called!!!
                                             // we start anew, going slowly!!!
      return parse_float(buf, pj, offset, found_minus);
    }
    double d = compute_float_64(power_index, i, negative);
    if (isnan(d)) {
      d = compute_float_128(power_index, i, 0, 0, negative, true);
      if (isnan(d)) {
        d = strtod((char *)(buf + offset), NULL);
      }
    }
    double d2 = strtod((char *)(buf + offset), NULL);
    assert(d == d2);
    pj.write_tape_double(d);
    // could we do a power of 5 mult and then a power of 10 mult, just to increase our chances that one won't have errors?
#ifdef JSON_TEST_NUMBERS // for unit testing
    found_float(d, buf + offset);
#endif
  } else {
    if (unlikely(digit_count >= 18)) { // this is uncommon!!!
                                       // there is a good chance that we had an overflow, so we need
                                       // need to recover: we parse the whole thing again.
      return parse_large_integer(buf, pj, offset, found_minus);
    }
    i = negative ? 0 - i : i;
    pj.write_tape_s64(i);
#ifdef JSON_TEST_NUMBERS // for unit testing
    found_integer(i, buf + offset);
#endif
  }
  return is_structural_or_whitespace(*p);
#endif // SIMDJSON_SKIPNUMBERPARSING
}
}
