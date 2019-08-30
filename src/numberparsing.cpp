//Copyright (c) 2018 Michael Eisel. All rights reserved.

#include <stdio.h>
#include "simdjson/numberparsing.h"
#include <rapidjson/internal/strtod.h>

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
  char *str = (char *)malloc(length);
  for (int i = 0; i < length; i++) {
    str[i] = '0' + ((ll >> (63 - (i + start))) & 1);
  }
  str[length] = '\0';
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
  static const uint16_t powers_ext[] = {0x6fb9, 0xa5d3, 0x8f48, 0x331a, 0x9ff0, 0x7ec, 0xc9e8, 0xbe31, 0x6dbd, 0x92c, 0x25bb, 0xaf2a, 0x1af5, 0x50d9, 0xe50f, 0x1e53, 0x25e8, 0x77b1, 0xd59d, 0x4b05, 0x4ee3, 0x229c, 0x6b43, 0x830a, 0x23cc, 0x2cbf, 0x7bf7, 0xdaf5, 0xd1b3, 0x2310, 0xabd4, 0x16c9, 0xae3d, 0x99cd, 0x4040, 0x4828, 0xda32, 0x90be, 0x5a77, 0x7115, 0xd5a, 0xe858, 0x626e, 0xfb0a, 0x7ce6, 0x1c1f, 0xa327, 0x4bf1, 0x6f77, 0xcb55, 0x7e2a, 0x2eda, 0xfa91, 0x7935, 0x4bc1, 0x9eb1, 0x465e, 0xbfa, 0xcef9, 0x82b7, 0xd1b2, 0x861f, 0x67a7, 0xe0c8, 0x58fa, 0xaf39, 0x6d84, 0xc8e5, 0xfb1e, 0x5cf2, 0xf42f, 0xf13b, 0x76c5, 0x5476, 0xa994, 0xd3f9, 0xc47b, 0x359a, 0xc301, 0x79e0, 0x9859, 0x3e6f, 0xa705, 0x50c6, 0xa4f8, 0x871b, 0x28e2, 0x331a, 0x3ff0, 0xfed, 0xd3e8, 0x6471, 0xbd8d, 0xecf0, 0xf416, 0x311c, 0x7d63, 0xae5d, 0xd9f5, 0xd072, 0x4247, 0x52d9, 0x6790, 0xba, 0x80e8, 0x6122, 0x796b, 0xcbe3, 0xbedb, 0xee92, 0x751b, 0xd262, 0x86fb, 0xd45d, 0x8974, 0x2bd1, 0x7b63, 0x1a3b, 0x20ca, 0x547e, 0xe99e, 0x6405, 0xde83, 0x9624, 0x3bad, 0xe54c, 0x5e9f, 0x7647, 0x29ec, 0xf468, 0x7182, 0xc6f1, 0xb8ad, 0xa6d9, 0x908f, 0x9a59, 0x40ef, 0xd12b, 0x82bb, 0xe36a, 0xdc44, 0x29ab, 0x7415, 0x111b, 0xcab1, 0x3d5d, 0xcb4, 0x47f0, 0x59ed, 0x3068, 0x1e41, 0xe5d1, 0xdf45, 0x6b8b, 0x66e, 0xc80a, 0xbd06, 0x2c48, 0xf75a, 0x9a98, 0xc13e, 0x318d, 0xfdf1, 0xfeb6, 0xfe64, 0x3dfd, 0x6be, 0x486e, 0x5a89, 0xf896, 0xf6bb, 0x746a, 0xa8c2, 0x92f3, 0x77b0, 0xace, 0xd81, 0x10e1, 0xca8d, 0xbd30, 0xac7c, 0x6bcd, 0x86c1, 0xe871, 0x1147, 0xd598, 0x4aff, 0xcedf, 0xc297, 0x733d, 0x806, 0xca07, 0xfc89, 0xbbac, 0xd54b, 0xa9e, 0x4d46, 0x504b, 0xe45e, 0x5d76, 0x3a6a, 0x8904, 0x2b45, 0x3b0b, 0x9ce, 0xcc42, 0x9fa9, 0x4793, 0x5978, 0x57eb, 0xede6, 0xe95f, 0x11db, 0xd652, 0x4be7, 0x6f70, 0xcb4c, 0x7e20, 0x8ed4, 0x7289, 0x4f2b, 0xe2f6, 0xdd9, 0x9150, 0x75a4, 0xc986, 0xfbe8, 0xfae2, 0xdccd, 0x5400, 0x2901, 0xf9a0, 0xf808, 0xb60b, 0xb1c6, 0x1e38, 0x25c6, 0x579c, 0x2d83, 0xf8e4, 0x1b8e, 0xe272, 0x5b0e, 0x98e9, 0x3f23, 0x8eec, 0x1953, 0x5fa8, 0x3792, 0xe2bb, 0x5b6a, 0xf245, 0xeed6, 0x5546, 0xaa97, 0xd53d, 0xe546, 0xde98, 0x963e, 0xdde7, 0x5560, 0xaab8, 0xcab3, 0x3d60, 0x8cb8, 0x77f3, 0x55f0, 0x6b6c, 0x2323, 0xabec, 0x96e7, 0x7e50, 0xdde5, 0x955e, 0xbd5a, 0xecb1, 0x67de, 0x80ea, 0xa125, 0x96e, 0x8bca, 0x775e, 0x9536, 0x3a83, 0xc492, 0x75b7, 0x5324, 0xd3f6, 0x88f4, 0x2b31, 0x3aff, 0x9be, 0x4c2e, 0xf9d, 0x5384, 0x2865, 0xf93f, 0xf78f, 0xb573, 0x3168, 0xfdc2, 0x3d32, 0xa63f, 0xfcf, 0xd3c3, 0x645a, 0x3d70, 0xcccc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4000, 0x5000, 0xa400, 0x4d00, 0xf020, 0x6c28, 0xc732, 0x3c7f, 0x4b9f, 0x1e86, 0x1314, 0x17d9, 0x5dcf, 0x5aa1, 0xf14a, 0x6d9c, 0xe482, 0xdda2, 0xd50b, 0x4526, 0x9670, 0x3c0c, 0xa588, 0x8eea, 0x72a4, 0x47a6, 0x9990, 0xfff4, 0xbff8, 0xaff7, 0x9bf4, 0x2f2, 0x1d7, 0x424d, 0xd2e0, 0x63cc, 0x3cbf, 0x8bef, 0x9775, 0x3d52, 0x4ca7, 0x8fe8, 0xb3e2, 0x60db, 0xbc89, 0x6bab, 0xc696, 0xfc1e, 0x3b25, 0x49ef, 0x6e35, 0x49c2, 0xdc33, 0x69a0, 0xc408, 0xf50a, 0x7926, 0x5770, 0xed4c, 0x544f, 0x6963, 0x3bc, 0x4ab, 0x62eb, 0x3ba5, 0xca8f, 0x7e99, 0x9e3f, 0xc5cf, 0xbba1, 0x2a8a, 0xf52d, 0x593c, 0x6f8b, 0xb6d, 0x4724, 0x58ed, 0x2f29, 0xbd79, 0xecd8, 0xe80e, 0x3109, 0xbd4b, 0x6c9e, 0x3e2, 0x84db, 0xe612, 0x4fcb, 0xe3be, 0x5cad, 0x73d9, 0x2867, 0xb281, 0x1f22, 0x3375, 0x52, 0xc067, 0xf840, 0xb650, 0xa3e5, 0xc66f, 0xb80b, 0xa60d, 0x87c8, 0x29ba, 0xf429, 0x1899, 0x5ec0, 0x7670, 0x6a06, 0x487, 0x45a9, 0xb8a, 0x8e6c, 0x3207, 0x7f44, 0x5f16, 0x36db, 0xc249, 0xf2db, 0x6f92, 0xcb77, 0xff2a, 0xfef5, 0x7eb2, 0xef2f, 0xaafb, 0x95ba, 0xdd94, 0x94f9, 0x7a37, 0xac62, 0x577b, 0xed5a, 0x1458, 0x596e, 0x6fca, 0x25de, 0xaf56, 0x1b2b, 0x90fb, 0x353a, 0x4288, 0x6995, 0x43fa, 0x94f9, 0x1d1b, 0x6462, 0x7d7b, 0x5cda, 0x3a08, 0x88a, 0x8aad, 0x36ac, 0x8457, 0x656d, 0x9f64, 0x873d, 0xa90c, 0x9a7, 0xc11, 0x8f16, 0xf96e, 0x37c9, 0x85bb, 0x9395, 0x387a, 0x699, 0x441f, 0xd527, 0x8a71, 0xf687, 0xb428, 0xe133, 0xecc0, 0x27f0, 0x31ec, 0x7e67, 0xf00, 0x52c0, 0xa770, 0x88a6, 0x6acf, 0x583, 0xc372, 0x744f, 0x1162, 0x8add, 0x6d95, 0xc8fa, 0x1d9c, 0x2503, 0x2e44, 0x5cea, 0x7425, 0xd12f, 0x82bd, 0x636c, 0x3c47, 0x65ac, 0x7f18, 0x1ede, 0x934a, 0xf81d, 0x3625, 0xc1d7, 0xb24c, 0xdee0, 0x1698, 0x8e1f, 0xf1a6, 0xae10, 0xacca, 0x17fd, 0xddfc, 0x4abd, 0x9d6d, 0x84c8, 0x32fd, 0x3fbc, 0xfab, 0x29cb, 0x743e, 0x914d, 0x1ad0, 0xa184, 0xc9e5, 0x7e2f, 0xddbb, 0x552a, 0xd53a, 0x8a89, 0x2d2b, 0x9c3b, 0x8349, 0x241c, 0xed23, 0xf436, 0xb143, 0xdd94, 0xca7c, 0xfd1c, 0xbc63, 0xd5be, 0x4b2d, 0xddf8, 0xcabb, 0x3d6a, 0xcc5, 0x27fb, 0xb1f9, 0x5e78, 0xdb0b, 0x91ce, 0x7641, 0xa9e9, 0x5463, 0xa97c, 0x49ed, 0x5c68, 0x7383, 0xc831, 0xba3e, 0x28ce, 0x7980, 0xd7e1, 0x8dd9, 0xb14f, 0x6ed1, 0xa86, 0xcd27, 0x8038, 0xe047, 0x1858, 0xf37, 0xd305, 0x47c6, 0x4cdc, 0xe013, 0x5818, 0x570f};
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
    double d = 0;
    if (z == 0) {// pj.shouldGo) {
      char *start = (char *)(buf + offset);
      if (found_minus) {
        start++;
      }
      int absexp = (exponent < 0 ? -exponent : exponent);
      int nbefore = p - absexp - start - 1;
      memcpy(buffer, start, nbefore);
      memcpy(buffer + nbefore, start + nbefore + 1, absexp);
      buffer[nbefore + absexp] = '\0';
      bool success = rapidjson::internal::StrtodDiyFp(buffer, p - start - 1, exponent,  &d);
      pj.write_tape_double(d);
    } else if (z == 1) {
      components c = power_of_ten_components[power_index];
      uint64_t factor_mantissa = c.mantissa;
      int lz = leading_zeroes(i);
      i <<= lz;
      __uint128_t large_mantissa = (__uint128_t)i * factor_mantissa;
      uint64_t upper = large_mantissa >> 64;
      uint64_t lower = large_mantissa;
      uint64_t justLastBit = upper & (1ULL << 63);
      if (likely((upper & 0x1FF) != 0x1FF)) {//} || (justLastBit && ((upper & 0x3FF) != 0x3FF))) {
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
        uint64_t real_exponent = c.exp + 1023 + (127 - lz); //lz);
        mantissa |= real_exponent << 52;
        mantissa |= ((uint64_t)negative) << 63; // is this safe? is this bool in [0, 1]?
        d = *((double *)&mantissa);
        //double d2 = strtod((char *)(buf + offset), NULL);
        //assert(d == d2);
        pj.write_tape_double(d);
      } else {
        d = strtod((char *)(buf + offset), NULL);
        pj.write_tape_double(d);
      }
    } else if (z == 2) {
        components c = power_of_ten_components[power_index];
        uint64_t factor_mantissa = c.mantissa;
        // no need to worry about the 10...0 case where we need to examine the previous bit as to the behavior (round to even), as we are lowballing already
        __uint128_t large_mantissa = (__uint128_t)i * factor_mantissa;
        __uint128_t change = large_mantissa ^ (large_mantissa + i);
        size_t lz = lz128(large_mantissa);
        size_t shift = 128 - lz - 54;
        // we could use a mantissa that's a mix of powers of 5 and 10 to see if we can find the best one
        double d = 0;
        if (change >> shift == 0) {
          uint64_t mantissa = large_mantissa >> shift;
          mantissa += mantissa & 1;
          mantissa >>= 1;
          mantissa &= ~(1ULL << 52);
          uint64_t real_exponent = c.exp + 1023 + (127 - lz);
          mantissa |= real_exponent << 52; // lz > 64?
          mantissa |= ((uint64_t)negative) << 63; // is this safe? is this bool in [0, 1]?
          d = *((double *)&mantissa);
          pj.write_tape_double(d);
        } else {
          __uint128_t m_ext_upper = (__uint128_t)factor_mantissa;
          __uint128_t m_ext = (m_ext_upper << 16) | powers_ext[power_index];
          __uint128_t i_ext = (__uint128_t)i;
          const uint64_t firstFortyMask = (1ULL << 40) - 1;
          __uint128_t lower_parts = (m_ext & firstFortyMask) * i_ext + m_ext * (i_ext & firstFortyMask);
          __uint128_t upper_and_extra = (m_ext >> 40) * (i_ext >> 40);
          __uint128_t upper = upper_and_extra >> 80;
          __uint128_t lower = (upper_and_extra << 80) + lower_parts;
          if (lower < lower_parts) {
            upper++;
          }
          if (upper) {
            lz = leading_zeroes((uint64_t)upper) + 64;
          } else {
            lz = leading_zeroes((uint64_t)(lower << 64)) + 128;
          }
          size_t safeBits = 256 - lz - 54;
          __uint128_t mask = (~((__uint128_t)0)) >> safeBits;
          __uint128_t max = lower + i_ext + m_ext + 1;
          // what if max overflowed and somehow came back to its old state?
          __uint128_t lower_shifted = (lower >> safeBits);
          if (lower_shifted == (max >> safeBits)) {
            uint64_t mantissa = lower_shifted | (upper << (128 - safeBits));
            mantissa += mantissa & 1;
            mantissa >>= 1;
            mantissa &= ~(1ULL << 52);
            uint64_t real_exponent = c.exp + 1023 + (127 - lz);
            mantissa |= real_exponent << 52; // lz > 64?
            mantissa |= ((uint64_t)negative) << 63; // is this safe? is this bool in [0, 1]?
            d = *((double *)&mantissa);
            d = strtod((char *)(buf + offset), NULL);
            pj.write_tape_double(d);
          } else {
              d = strtod((char *)(buf + offset), NULL);
              pj.write_tape_double(d);
          }
        }
    } else if (z == 3) {
      double factor = power_of_ten[power_index];
      factor = negative ? -factor : factor;
      d = i * factor;
      pj.write_tape_double(d);
    } else if (z == 4) {
      d = strtod((char *)(buf + offset), NULL);
      pj.write_tape_double(d);
    } else if (z == 5) {
      pj.write_tape_double(0);
    }
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
