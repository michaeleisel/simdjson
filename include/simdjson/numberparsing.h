#ifndef SIMDJSON_NUMBERPARSING_H
#define SIMDJSON_NUMBERPARSING_H

#include "simdjson/common_defs.h"
#include "simdjson/jsoncharutils.h"
#include "simdjson/parsedjson.h"
#include "simdjson/portability.h"
#include <cmath>

#ifdef JSON_TEST_NUMBERS // for unit testing
void found_invalid_number(const uint8_t *buf);
void found_integer(int64_t result, const uint8_t *buf);
void found_float(double result, const uint8_t *buf);
#endif

void inc1(void);
void inc2(void);
// extern size_t sCount1, sCount2;

static bool sUseNew = false;

static void switchh() {
  sUseNew = !sUseNew;
}

static bool get() {
  return sUseNew;
}

namespace simdjson {
// Allowable floating-point values range from
// std::numeric_limits<double>::lowest() to std::numeric_limits<double>::max(),
// so from -1.7976e308 all the way to 1.7975e308 in binary64. The lowest
// non-zero normal values is std::numeric_limits<double>::min() or
// about 2.225074e-308.
static const double power_of_ten[] = {
    1e-308, 1e-307, 1e-306, 1e-305, 1e-304, 1e-303, 1e-302, 1e-301, 1e-300,
    1e-299, 1e-298, 1e-297, 1e-296, 1e-295, 1e-294, 1e-293, 1e-292, 1e-291,
    1e-290, 1e-289, 1e-288, 1e-287, 1e-286, 1e-285, 1e-284, 1e-283, 1e-282,
    1e-281, 1e-280, 1e-279, 1e-278, 1e-277, 1e-276, 1e-275, 1e-274, 1e-273,
    1e-272, 1e-271, 1e-270, 1e-269, 1e-268, 1e-267, 1e-266, 1e-265, 1e-264,
    1e-263, 1e-262, 1e-261, 1e-260, 1e-259, 1e-258, 1e-257, 1e-256, 1e-255,
    1e-254, 1e-253, 1e-252, 1e-251, 1e-250, 1e-249, 1e-248, 1e-247, 1e-246,
    1e-245, 1e-244, 1e-243, 1e-242, 1e-241, 1e-240, 1e-239, 1e-238, 1e-237,
    1e-236, 1e-235, 1e-234, 1e-233, 1e-232, 1e-231, 1e-230, 1e-229, 1e-228,
    1e-227, 1e-226, 1e-225, 1e-224, 1e-223, 1e-222, 1e-221, 1e-220, 1e-219,
    1e-218, 1e-217, 1e-216, 1e-215, 1e-214, 1e-213, 1e-212, 1e-211, 1e-210,
    1e-209, 1e-208, 1e-207, 1e-206, 1e-205, 1e-204, 1e-203, 1e-202, 1e-201,
    1e-200, 1e-199, 1e-198, 1e-197, 1e-196, 1e-195, 1e-194, 1e-193, 1e-192,
    1e-191, 1e-190, 1e-189, 1e-188, 1e-187, 1e-186, 1e-185, 1e-184, 1e-183,
    1e-182, 1e-181, 1e-180, 1e-179, 1e-178, 1e-177, 1e-176, 1e-175, 1e-174,
    1e-173, 1e-172, 1e-171, 1e-170, 1e-169, 1e-168, 1e-167, 1e-166, 1e-165,
    1e-164, 1e-163, 1e-162, 1e-161, 1e-160, 1e-159, 1e-158, 1e-157, 1e-156,
    1e-155, 1e-154, 1e-153, 1e-152, 1e-151, 1e-150, 1e-149, 1e-148, 1e-147,
    1e-146, 1e-145, 1e-144, 1e-143, 1e-142, 1e-141, 1e-140, 1e-139, 1e-138,
    1e-137, 1e-136, 1e-135, 1e-134, 1e-133, 1e-132, 1e-131, 1e-130, 1e-129,
    1e-128, 1e-127, 1e-126, 1e-125, 1e-124, 1e-123, 1e-122, 1e-121, 1e-120,
    1e-119, 1e-118, 1e-117, 1e-116, 1e-115, 1e-114, 1e-113, 1e-112, 1e-111,
    1e-110, 1e-109, 1e-108, 1e-107, 1e-106, 1e-105, 1e-104, 1e-103, 1e-102,
    1e-101, 1e-100, 1e-99,  1e-98,  1e-97,  1e-96,  1e-95,  1e-94,  1e-93,
    1e-92,  1e-91,  1e-90,  1e-89,  1e-88,  1e-87,  1e-86,  1e-85,  1e-84,
    1e-83,  1e-82,  1e-81,  1e-80,  1e-79,  1e-78,  1e-77,  1e-76,  1e-75,
    1e-74,  1e-73,  1e-72,  1e-71,  1e-70,  1e-69,  1e-68,  1e-67,  1e-66,
    1e-65,  1e-64,  1e-63,  1e-62,  1e-61,  1e-60,  1e-59,  1e-58,  1e-57,
    1e-56,  1e-55,  1e-54,  1e-53,  1e-52,  1e-51,  1e-50,  1e-49,  1e-48,
    1e-47,  1e-46,  1e-45,  1e-44,  1e-43,  1e-42,  1e-41,  1e-40,  1e-39,
    1e-38,  1e-37,  1e-36,  1e-35,  1e-34,  1e-33,  1e-32,  1e-31,  1e-30,
    1e-29,  1e-28,  1e-27,  1e-26,  1e-25,  1e-24,  1e-23,  1e-22,  1e-21,
    1e-20,  1e-19,  1e-18,  1e-17,  1e-16,  1e-15,  1e-14,  1e-13,  1e-12,
    1e-11,  1e-10,  1e-9,   1e-8,   1e-7,   1e-6,   1e-5,   1e-4,   1e-3,
    1e-2,   1e-1,   1e0,    1e1,    1e2,    1e3,    1e4,    1e5,    1e6,
    1e7,    1e8,    1e9,    1e10,   1e11,   1e12,   1e13,   1e14,   1e15,
    1e16,   1e17,   1e18,   1e19,   1e20,   1e21,   1e22,   1e23,   1e24,
    1e25,   1e26,   1e27,   1e28,   1e29,   1e30,   1e31,   1e32,   1e33,
    1e34,   1e35,   1e36,   1e37,   1e38,   1e39,   1e40,   1e41,   1e42,
    1e43,   1e44,   1e45,   1e46,   1e47,   1e48,   1e49,   1e50,   1e51,
    1e52,   1e53,   1e54,   1e55,   1e56,   1e57,   1e58,   1e59,   1e60,
    1e61,   1e62,   1e63,   1e64,   1e65,   1e66,   1e67,   1e68,   1e69,
    1e70,   1e71,   1e72,   1e73,   1e74,   1e75,   1e76,   1e77,   1e78,
    1e79,   1e80,   1e81,   1e82,   1e83,   1e84,   1e85,   1e86,   1e87,
    1e88,   1e89,   1e90,   1e91,   1e92,   1e93,   1e94,   1e95,   1e96,
    1e97,   1e98,   1e99,   1e100,  1e101,  1e102,  1e103,  1e104,  1e105,
    1e106,  1e107,  1e108,  1e109,  1e110,  1e111,  1e112,  1e113,  1e114,
    1e115,  1e116,  1e117,  1e118,  1e119,  1e120,  1e121,  1e122,  1e123,
    1e124,  1e125,  1e126,  1e127,  1e128,  1e129,  1e130,  1e131,  1e132,
    1e133,  1e134,  1e135,  1e136,  1e137,  1e138,  1e139,  1e140,  1e141,
    1e142,  1e143,  1e144,  1e145,  1e146,  1e147,  1e148,  1e149,  1e150,
    1e151,  1e152,  1e153,  1e154,  1e155,  1e156,  1e157,  1e158,  1e159,
    1e160,  1e161,  1e162,  1e163,  1e164,  1e165,  1e166,  1e167,  1e168,
    1e169,  1e170,  1e171,  1e172,  1e173,  1e174,  1e175,  1e176,  1e177,
    1e178,  1e179,  1e180,  1e181,  1e182,  1e183,  1e184,  1e185,  1e186,
    1e187,  1e188,  1e189,  1e190,  1e191,  1e192,  1e193,  1e194,  1e195,
    1e196,  1e197,  1e198,  1e199,  1e200,  1e201,  1e202,  1e203,  1e204,
    1e205,  1e206,  1e207,  1e208,  1e209,  1e210,  1e211,  1e212,  1e213,
    1e214,  1e215,  1e216,  1e217,  1e218,  1e219,  1e220,  1e221,  1e222,
    1e223,  1e224,  1e225,  1e226,  1e227,  1e228,  1e229,  1e230,  1e231,
    1e232,  1e233,  1e234,  1e235,  1e236,  1e237,  1e238,  1e239,  1e240,
    1e241,  1e242,  1e243,  1e244,  1e245,  1e246,  1e247,  1e248,  1e249,
    1e250,  1e251,  1e252,  1e253,  1e254,  1e255,  1e256,  1e257,  1e258,
    1e259,  1e260,  1e261,  1e262,  1e263,  1e264,  1e265,  1e266,  1e267,
    1e268,  1e269,  1e270,  1e271,  1e272,  1e273,  1e274,  1e275,  1e276,
    1e277,  1e278,  1e279,  1e280,  1e281,  1e282,  1e283,  1e284,  1e285,
    1e286,  1e287,  1e288,  1e289,  1e290,  1e291,  1e292,  1e293,  1e294,
    1e295,  1e296,  1e297,  1e298,  1e299,  1e300,  1e301,  1e302,  1e303,
    1e304,  1e305,  1e306,  1e307,  1e308};

static inline bool is_integer(char c) {
  return (c >= '0' && c <= '9');
  // this gets compiled to (uint8_t)(c - '0') <= 9 on all decent compilers
}

// We need to check that the character following a zero is valid. This is
// probably frequent and it is hard than it looks. We are building all of this
// just to differentiate between 0x1 (invalid), 0,1 (valid) 0e1 (valid)...
const bool structural_or_whitespace_or_exponent_or_decimal_negated[256] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1,
    1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

really_inline bool
is_not_structural_or_whitespace_or_exponent_or_decimal(unsigned char c) {
  return structural_or_whitespace_or_exponent_or_decimal_negated[c];
}
} // namespace simdjson
#ifndef SIMDJSON_DISABLE_SWAR_NUMBER_PARSING
#define SWAR_NUMBER_PARSING
#endif

#ifdef SWAR_NUMBER_PARSING

namespace simdjson {
// check quickly whether the next 8 chars are made of digits
// at a glance, it looks better than Mula's
// http://0x80.pl/articles/swar-digits-validate.html
static inline bool is_made_of_eight_digits_fast(const char *chars) {
  uint64_t val;
  // this can read up to 7 bytes beyond the buffer size, but we require
  // SIMDJSON_PADDING of padding
  static_assert(7 <= SIMDJSON_PADDING);
  memcpy(&val, chars, 8);
  // a branchy method might be faster:
  // return (( val & 0xF0F0F0F0F0F0F0F0 ) == 0x3030303030303030)
  //  && (( (val + 0x0606060606060606) & 0xF0F0F0F0F0F0F0F0 ) ==
  //  0x3030303030303030);
  return (((val & 0xF0F0F0F0F0F0F0F0) |
           (((val + 0x0606060606060606) & 0xF0F0F0F0F0F0F0F0) >> 4)) ==
          0x3333333333333333);
}
} // namespace simdjson
#ifdef IS_X86_64
TARGET_WESTMERE
namespace simdjson {
static inline uint32_t parse_eight_digits_unrolled(const char *chars) {
  // this actually computes *16* values so we are being wasteful.
  const __m128i ascii0 = _mm_set1_epi8('0');
  const __m128i mul_1_10 =
      _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
  const __m128i mul_1_100 = _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1);
  const __m128i mul_1_10000 =
      _mm_setr_epi16(10000, 1, 10000, 1, 10000, 1, 10000, 1);
  const __m128i input = _mm_sub_epi8(
      _mm_loadu_si128(reinterpret_cast<const __m128i *>(chars)), ascii0);
  const __m128i t1 = _mm_maddubs_epi16(input, mul_1_10);
  const __m128i t2 = _mm_madd_epi16(t1, mul_1_100);
  const __m128i t3 = _mm_packus_epi32(t2, t2);
  const __m128i t4 = _mm_madd_epi16(t3, mul_1_10000);
  return _mm_cvtsi128_si32(
      t4); // only captures the sum of the first 8 digits, drop the rest
}
} // namespace simdjson
UNTARGET_REGION
#endif

namespace simdjson {
#ifdef IS_ARM64
// we don't have SSE, so let us use a scalar function
// credit: https://johnnylee-sde.github.io/Fast-numeric-string-to-int/
static inline uint32_t parse_eight_digits_unrolled(const char *chars) {
  uint64_t val;
  memcpy(&val, chars, sizeof(uint64_t));
  val = (val & 0x0F0F0F0F0F0F0F0F) * 2561 >> 8;
  val = (val & 0x00FF00FF00FF00FF) * 6553601 >> 16;
  return (val & 0x0000FFFF0000FFFF) * 42949672960001 >> 32;
}
#endif

#endif

//
// This function computes base * 10 ^ (- negative_exponent ).
// It is only even going to be used when negative_exponent is tiny.
static double subnormal_power10(double base, int negative_exponent) {
  // this is probably not going to be fast
  return base * 1e-308 * pow(10, negative_exponent + 308);
}

// called by parse_number when we know that the output is a float,
// but where there might be some integer overflow. The trick here is to
// parse using floats from the start.
// Do not call this function directly as it skips some of the checks from
// parse_number
//
// This function will almost never be called!!!
//
// Note: a redesign could avoid this function entirely.
//
static never_inline bool parse_float(const uint8_t *const buf, ParsedJson &pj,
                                     const uint32_t offset, bool found_minus) {
  const char *p = reinterpret_cast<const char *>(buf + offset);
  bool negative = false;
  if (found_minus) {
    ++p;
    negative = true;
  }
  long double i;
  if (*p == '0') { // 0 cannot be followed by an integer
    ++p;
    i = 0;
  } else {
    unsigned char digit = *p - '0';
    i = digit;
    p++;
    while (is_integer(*p)) {
      digit = *p - '0';
      i = 10 * i + digit;
      ++p;
    }
  }
  if ('.' == *p) {
    ++p;
    int fractional_weight = 308;
    if (is_integer(*p)) {
      unsigned char digit = *p - '0';
      ++p;

      fractional_weight--;
      i = i + digit * (fractional_weight >= 0 ? power_of_ten[fractional_weight]
                                              : 0);
    } else {
#ifdef JSON_TEST_NUMBERS // for unit testing
      found_invalid_number(buf + offset);
#endif
      return false;
    }
    while (is_integer(*p)) {
      unsigned char digit = *p - '0';
      ++p;
      fractional_weight--;
      i = i + digit * (fractional_weight >= 0 ? power_of_ten[fractional_weight]
                                              : 0);
    }
  }
  if (('e' == *p) || ('E' == *p)) {
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
    int64_t exp_number = digit; // exponential part
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
    if (unlikely(exp_number > 308)) {
      // this path is unlikely
      if (neg_exp) {
        // We either have zero or a subnormal.
        // We expect this to be uncommon so we go through a slow path.
        i = subnormal_power10(i, -exp_number);
      } else {
// We know for sure that we have a number that is too large,
// we refuse to parse this
#ifdef JSON_TEST_NUMBERS // for unit testing
        found_invalid_number(buf + offset);
#endif
        return false;
      }
    } else {
      int exponent = (neg_exp ? -exp_number : exp_number);
      // we have that exp_number is [0,308] so that
      // exponent is [-308,308] so that
      // 308 + exponent is in [0, 2 * 308]
      i *= power_of_ten[308 + exponent];
    }
  }
  if (is_not_structural_or_whitespace(*p)) {
    return false;
  }
  double d = negative ? -i : i;
  pj.write_tape_double(d);
#ifdef JSON_TEST_NUMBERS // for unit testing
  found_float(d, buf + offset);
#endif
  return is_structural_or_whitespace(*p);
}

// called by parse_number when we know that the output is an integer,
// but where there might be some integer overflow.
// we want to catch overflows!
// Do not call this function directly as it skips some of the checks from
// parse_number
//
// This function will almost never be called!!!
//
static never_inline bool parse_large_integer(const uint8_t *const buf,
                                             ParsedJson &pj,
                                             const uint32_t offset,
                                             bool found_minus) {
  const char *p = reinterpret_cast<const char *>(buf + offset);

  bool negative = false;
  if (found_minus) {
    ++p;
    negative = true;
  }
  uint64_t i;
  if (*p == '0') { // 0 cannot be followed by an integer
    ++p;
    i = 0;
  } else {
    unsigned char digit = *p - '0';
    i = digit;
    p++;
    // the is_made_of_eight_digits_fast routine is unlikely to help here because
    // we rarely see large integer parts like 123456789
    while (is_integer(*p)) {
      digit = *p - '0';
      if (mul_overflow(i, 10, &i)) {
#ifdef JSON_TEST_NUMBERS // for unit testing
        found_invalid_number(buf + offset);
#endif
        return false; // overflow
      }
      if (add_overflow(i, digit, &i)) {
#ifdef JSON_TEST_NUMBERS // for unit testing
        found_invalid_number(buf + offset);
#endif
        return false; // overflow
      }
      ++p;
    }
  }
  if (negative) {
    if (i > 0x8000000000000000) {
// overflows!
#ifdef JSON_TEST_NUMBERS // for unit testing
      found_invalid_number(buf + offset);
#endif
      return false; // overflow
    }
  } else {
    if (i >= 0x8000000000000000) {
// overflows!
#ifdef JSON_TEST_NUMBERS // for unit testing
      found_invalid_number(buf + offset);
#endif
      return false; // overflow
    }
  }
  int64_t signed_answer =
      negative ? -static_cast<int64_t>(i) : static_cast<int64_t>(i);
  pj.write_tape_s64(signed_answer);
#ifdef JSON_TEST_NUMBERS // for unit testing
  found_integer(signed_answer, buf + offset);
#endif
  return is_structural_or_whitespace(*p);
}

  // the mantissas of powers of five from -308 to 308, extended out to sixty four bits
  typedef struct {
    uint64_t mantissa;
    int32_t exp;
  } components;



  static const components power_of_ten_components[] = {{0xe61acf033d1a45df, -1087}, {0x8fd0c16206306bab, -1083}, {0xb3c4f1ba87bc8696, -1080}, {0xe0b62e2929aba83c, -1077}, {0x8c71dcd9ba0b4925, -1073}, {0xaf8e5410288e1b6f, -1070}, {0xdb71e91432b1a24a, -1067}, {0x892731ac9faf056e, -1063}, {0xab70fe17c79ac6ca, -1060}, {0xd64d3d9db981787d, -1057}, {0x85f0468293f0eb4e, -1053}, {0xa76c582338ed2621, -1050}, {0xd1476e2c07286faa, -1047}, {0x82cca4db847945ca, -1043}, {0xa37fce126597973c, -1040}, {0xcc5fc196fefd7d0c, -1037}, {0xff77b1fcbebcdc4f, -1034}, {0x9faacf3df73609b1, -1030}, {0xc795830d75038c1d, -1027}, {0xf97ae3d0d2446f25, -1024}, {0x9becce62836ac577, -1020}, {0xc2e801fb244576d5, -1017}, {0xf3a20279ed56d48a, -1014}, {0x9845418c345644d6, -1010}, {0xbe5691ef416bd60c, -1007}, {0xedec366b11c6cb8f, -1004}, {0x94b3a202eb1c3f39, -1000}, {0xb9e08a83a5e34f07, -997}, {0xe858ad248f5c22c9, -994}, {0x91376c36d99995be, -990}, {0xb58547448ffffb2d, -987}, {0xe2e69915b3fff9f9, -984}, {0x8dd01fad907ffc3b, -980}, {0xb1442798f49ffb4a, -977}, {0xdd95317f31c7fa1d, -974}, {0x8a7d3eef7f1cfc52, -970}, {0xad1c8eab5ee43b66, -967}, {0xd863b256369d4a40, -964}, {0x873e4f75e2224e68, -960}, {0xa90de3535aaae202, -957}, {0xd3515c2831559a83, -954}, {0x8412d9991ed58091, -950}, {0xa5178fff668ae0b6, -947}, {0xce5d73ff402d98e3, -944}, {0x80fa687f881c7f8e, -940}, {0xa139029f6a239f72, -937}, {0xc987434744ac874e, -934}, {0xfbe9141915d7a922, -931}, {0x9d71ac8fada6c9b5, -927}, {0xc4ce17b399107c22, -924}, {0xf6019da07f549b2b, -921}, {0x99c102844f94e0fb, -917}, {0xc0314325637a1939, -914}, {0xf03d93eebc589f88, -911}, {0x96267c7535b763b5, -907}, {0xbbb01b9283253ca2, -904}, {0xea9c227723ee8bcb, -901}, {0x92a1958a7675175f, -897}, {0xb749faed14125d36, -894}, {0xe51c79a85916f484, -891}, {0x8f31cc0937ae58d2, -887}, {0xb2fe3f0b8599ef07, -884}, {0xdfbdcece67006ac9, -881}, {0x8bd6a141006042bd, -877}, {0xaecc49914078536d, -874}, {0xda7f5bf590966848, -871}, {0x888f99797a5e012d, -867}, {0xaab37fd7d8f58178, -864}, {0xd5605fcdcf32e1d6, -861}, {0x855c3be0a17fcd26, -857}, {0xa6b34ad8c9dfc06f, -854}, {0xd0601d8efc57b08b, -851}, {0x823c12795db6ce57, -847}, {0xa2cb1717b52481ed, -844}, {0xcb7ddcdda26da268, -841}, {0xfe5d54150b090b02, -838}, {0x9efa548d26e5a6e1, -834}, {0xc6b8e9b0709f109a, -831}, {0xf867241c8cc6d4c0, -828}, {0x9b407691d7fc44f8, -824}, {0xc21094364dfb5636, -821}, {0xf294b943e17a2bc4, -818}, {0x979cf3ca6cec5b5a, -814}, {0xbd8430bd08277231, -811}, {0xece53cec4a314ebd, -808}, {0x940f4613ae5ed136, -804}, {0xb913179899f68584, -801}, {0xe757dd7ec07426e5, -798}, {0x9096ea6f3848984f, -794}, {0xb4bca50b065abe63, -791}, {0xe1ebce4dc7f16dfb, -788}, {0x8d3360f09cf6e4bd, -784}, {0xb080392cc4349dec, -781}, {0xdca04777f541c567, -778}, {0x89e42caaf9491b60, -774}, {0xac5d37d5b79b6239, -771}, {0xd77485cb25823ac7, -768}, {0x86a8d39ef77164bc, -764}, {0xa8530886b54dbdeb, -761}, {0xd267caa862a12d66, -758}, {0x8380dea93da4bc60, -754}, {0xa46116538d0deb78, -751}, {0xcd795be870516656, -748}, {0x806bd9714632dff6, -744}, {0xa086cfcd97bf97f3, -741}, {0xc8a883c0fdaf7df0, -738}, {0xfad2a4b13d1b5d6c, -735}, {0x9cc3a6eec6311a63, -731}, {0xc3f490aa77bd60fc, -728}, {0xf4f1b4d515acb93b, -725}, {0x991711052d8bf3c5, -721}, {0xbf5cd54678eef0b6, -718}, {0xef340a98172aace4, -715}, {0x9580869f0e7aac0e, -711}, {0xbae0a846d2195712, -708}, {0xe998d258869facd7, -705}, {0x91ff83775423cc06, -701}, {0xb67f6455292cbf08, -698}, {0xe41f3d6a7377eeca, -695}, {0x8e938662882af53e, -691}, {0xb23867fb2a35b28d, -688}, {0xdec681f9f4c31f31, -685}, {0x8b3c113c38f9f37e, -681}, {0xae0b158b4738705e, -678}, {0xd98ddaee19068c76, -675}, {0x87f8a8d4cfa417c9, -671}, {0xa9f6d30a038d1dbc, -668}, {0xd47487cc8470652b, -665}, {0x84c8d4dfd2c63f3b, -661}, {0xa5fb0a17c777cf09, -658}, {0xcf79cc9db955c2cc, -655}, {0x81ac1fe293d599bf, -651}, {0xa21727db38cb002f, -648}, {0xca9cf1d206fdc03b, -645}, {0xfd442e4688bd304a, -642}, {0x9e4a9cec15763e2e, -638}, {0xc5dd44271ad3cdba, -635}, {0xf7549530e188c128, -632}, {0x9a94dd3e8cf578b9, -628}, {0xc13a148e3032d6e7, -625}, {0xf18899b1bc3f8ca1, -622}, {0x96f5600f15a7b7e5, -618}, {0xbcb2b812db11a5de, -615}, {0xebdf661791d60f56, -612}, {0x936b9fcebb25c995, -608}, {0xb84687c269ef3bfb, -605}, {0xe65829b3046b0afa, -602}, {0x8ff71a0fe2c2e6dc, -598}, {0xb3f4e093db73a093, -595}, {0xe0f218b8d25088b8, -592}, {0x8c974f7383725573, -588}, {0xafbd2350644eeacf, -585}, {0xdbac6c247d62a583, -582}, {0x894bc396ce5da772, -578}, {0xab9eb47c81f5114f, -575}, {0xd686619ba27255a2, -572}, {0x8613fd0145877585, -568}, {0xa798fc4196e952e7, -565}, {0xd17f3b51fca3a7a0, -562}, {0x82ef85133de648c4, -558}, {0xa3ab66580d5fdaf5, -555}, {0xcc963fee10b7d1b3, -552}, {0xffbbcfe994e5c61f, -549}, {0x9fd561f1fd0f9bd3, -545}, {0xc7caba6e7c5382c8, -542}, {0xf9bd690a1b68637b, -539}, {0x9c1661a651213e2d, -535}, {0xc31bfa0fe5698db8, -532}, {0xf3e2f893dec3f126, -529}, {0x986ddb5c6b3a76b7, -525}, {0xbe89523386091465, -522}, {0xee2ba6c0678b597f, -519}, {0x94db483840b717ef, -515}, {0xba121a4650e4ddeb, -512}, {0xe896a0d7e51e1566, -509}, {0x915e2486ef32cd60, -505}, {0xb5b5ada8aaff80b8, -502}, {0xe3231912d5bf60e6, -499}, {0x8df5efabc5979c8f, -495}, {0xb1736b96b6fd83b3, -492}, {0xddd0467c64bce4a0, -489}, {0x8aa22c0dbef60ee4, -485}, {0xad4ab7112eb3929d, -482}, {0xd89d64d57a607744, -479}, {0x87625f056c7c4a8b, -475}, {0xa93af6c6c79b5d2d, -472}, {0xd389b47879823479, -469}, {0x843610cb4bf160cb, -465}, {0xa54394fe1eedb8fe, -462}, {0xce947a3da6a9273e, -459}, {0x811ccc668829b887, -455}, {0xa163ff802a3426a8, -452}, {0xc9bcff6034c13052, -449}, {0xfc2c3f3841f17c67, -446}, {0x9d9ba7832936edc0, -442}, {0xc5029163f384a931, -439}, {0xf64335bcf065d37d, -436}, {0x99ea0196163fa42e, -432}, {0xc06481fb9bcf8d39, -429}, {0xf07da27a82c37088, -426}, {0x964e858c91ba2655, -422}, {0xbbe226efb628afea, -419}, {0xeadab0aba3b2dbe5, -416}, {0x92c8ae6b464fc96f, -412}, {0xb77ada0617e3bbcb, -409}, {0xe55990879ddcaabd, -406}, {0x8f57fa54c2a9eab6, -402}, {0xb32df8e9f3546564, -399}, {0xdff9772470297ebd, -396}, {0x8bfbea76c619ef36, -392}, {0xaefae51477a06b03, -389}, {0xdab99e59958885c4, -386}, {0x88b402f7fd75539b, -382}, {0xaae103b5fcd2a881, -379}, {0xd59944a37c0752a2, -376}, {0x857fcae62d8493a5, -372}, {0xa6dfbd9fb8e5b88e, -369}, {0xd097ad07a71f26b2, -366}, {0x825ecc24c873782f, -362}, {0xa2f67f2dfa90563b, -359}, {0xcbb41ef979346bca, -356}, {0xfea126b7d78186bc, -353}, {0x9f24b832e6b0f436, -349}, {0xc6ede63fa05d3143, -346}, {0xf8a95fcf88747d94, -343}, {0x9b69dbe1b548ce7c, -339}, {0xc24452da229b021b, -336}, {0xf2d56790ab41c2a2, -333}, {0x97c560ba6b0919a5, -329}, {0xbdb6b8e905cb600f, -326}, {0xed246723473e3813, -323}, {0x9436c0760c86e30b, -319}, {0xb94470938fa89bce, -316}, {0xe7958cb87392c2c2, -313}, {0x90bd77f3483bb9b9, -309}, {0xb4ecd5f01a4aa828, -306}, {0xe2280b6c20dd5232, -303}, {0x8d590723948a535f, -299}, {0xb0af48ec79ace837, -296}, {0xdcdb1b2798182244, -293}, {0x8a08f0f8bf0f156b, -289}, {0xac8b2d36eed2dac5, -286}, {0xd7adf884aa879177, -283}, {0x86ccbb52ea94baea, -279}, {0xa87fea27a539e9a5, -276}, {0xd29fe4b18e88640e, -273}, {0x83a3eeeef9153e89, -269}, {0xa48ceaaab75a8e2b, -266}, {0xcdb02555653131b6, -263}, {0x808e17555f3ebf11, -259}, {0xa0b19d2ab70e6ed6, -256}, {0xc8de047564d20a8b, -253}, {0xfb158592be068d2e, -250}, {0x9ced737bb6c4183d, -246}, {0xc428d05aa4751e4c, -243}, {0xf53304714d9265df, -240}, {0x993fe2c6d07b7fab, -236}, {0xbf8fdb78849a5f96, -233}, {0xef73d256a5c0f77c, -230}, {0x95a8637627989aad, -226}, {0xbb127c53b17ec159, -223}, {0xe9d71b689dde71af, -220}, {0x9226712162ab070d, -216}, {0xb6b00d69bb55c8d1, -213}, {0xe45c10c42a2b3b05, -210}, {0x8eb98a7a9a5b04e3, -206}, {0xb267ed1940f1c61c, -203}, {0xdf01e85f912e37a3, -200}, {0x8b61313bbabce2c6, -196}, {0xae397d8aa96c1b77, -193}, {0xd9c7dced53c72255, -190}, {0x881cea14545c7575, -186}, {0xaa242499697392d2, -183}, {0xd4ad2dbfc3d07787, -180}, {0x84ec3c97da624ab4, -176}, {0xa6274bbdd0fadd61, -173}, {0xcfb11ead453994ba, -170}, {0x81ceb32c4b43fcf4, -166}, {0xa2425ff75e14fc31, -163}, {0xcad2f7f5359a3b3e, -160}, {0xfd87b5f28300ca0d, -157}, {0x9e74d1b791e07e48, -153}, {0xc612062576589dda, -150}, {0xf79687aed3eec551, -147}, {0x9abe14cd44753b52, -143}, {0xc16d9a0095928a27, -140}, {0xf1c90080baf72cb1, -137}, {0x971da05074da7bee, -133}, {0xbce5086492111aea, -130}, {0xec1e4a7db69561a5, -127}, {0x9392ee8e921d5d07, -123}, {0xb877aa3236a4b449, -120}, {0xe69594bec44de15b, -117}, {0x901d7cf73ab0acd9, -113}, {0xb424dc35095cd80f, -110}, {0xe12e13424bb40e13, -107}, {0x8cbccc096f5088cb, -103}, {0xafebff0bcb24aafe, -100}, {0xdbe6fecebdedd5be, -97}, {0x89705f4136b4a597, -93}, {0xabcc77118461cefc, -90}, {0xd6bf94d5e57a42bc, -87}, {0x8637bd05af6c69b5, -83}, {0xa7c5ac471b478423, -80}, {0xd1b71758e219652b, -77}, {0x83126e978d4fdf3b, -73}, {0xa3d70a3d70a3d70a, -70}, {0xcccccccccccccccc, -67}, {0x8000000000000000, -64}, {0xa000000000000000, -60}, {0xc800000000000000, -57}, {0xfa00000000000000, -54}, {0x9c40000000000000, -50}, {0xc350000000000000, -47}, {0xf424000000000000, -44}, {0x9896800000000000, -40}, {0xbebc200000000000, -37}, {0xee6b280000000000, -34}, {0x9502f90000000000, -30}, {0xba43b74000000000, -27}, {0xe8d4a51000000000, -24}, {0x9184e72a00000000, -20}, {0xb5e620f480000000, -17}, {0xe35fa931a0000000, -14}, {0x8e1bc9bf04000000, -10}, {0xb1a2bc2ec5000000, -7}, {0xde0b6b3a76400000, -4}, {0x8ac7230489e80000, 0}, {0xad78ebc5ac620000, 3}, {0xd8d726b7177a8000, 6}, {0x878678326eac9000, 10}, {0xa968163f0a57b400, 13}, {0xd3c21bcecceda100, 16}, {0x84595161401484a0, 20}, {0xa56fa5b99019a5c8, 23}, {0xcecb8f27f4200f3a, 26}, {0x813f3978f8940984, 30}, {0xa18f07d736b90be5, 33}, {0xc9f2c9cd04674ede, 36}, {0xfc6f7c4045812296, 39}, {0x9dc5ada82b70b59d, 43}, {0xc5371912364ce305, 46}, {0xf684df56c3e01bc6, 49}, {0x9a130b963a6c115c, 53}, {0xc097ce7bc90715b3, 56}, {0xf0bdc21abb48db20, 59}, {0x96769950b50d88f4, 63}, {0xbc143fa4e250eb31, 66}, {0xeb194f8e1ae525fd, 69}, {0x92efd1b8d0cf37be, 73}, {0xb7abc627050305ad, 76}, {0xe596b7b0c643c719, 79}, {0x8f7e32ce7bea5c6f, 83}, {0xb35dbf821ae4f38b, 86}, {0xe0352f62a19e306e, 89}, {0x8c213d9da502de45, 93}, {0xaf298d050e4395d6, 96}, {0xdaf3f04651d47b4c, 99}, {0x88d8762bf324cd0f, 103}, {0xab0e93b6efee0053, 106}, {0xd5d238a4abe98068, 109}, {0x85a36366eb71f041, 113}, {0xa70c3c40a64e6c51, 116}, {0xd0cf4b50cfe20765, 119}, {0x82818f1281ed449f, 123}, {0xa321f2d7226895c7, 126}, {0xcbea6f8ceb02bb39, 129}, {0xfee50b7025c36a08, 132}, {0x9f4f2726179a2245, 136}, {0xc722f0ef9d80aad6, 139}, {0xf8ebad2b84e0d58b, 142}, {0x9b934c3b330c8577, 146}, {0xc2781f49ffcfa6d5, 149}, {0xf316271c7fc3908a, 152}, {0x97edd871cfda3a56, 156}, {0xbde94e8e43d0c8ec, 159}, {0xed63a231d4c4fb27, 162}, {0x945e455f24fb1cf8, 166}, {0xb975d6b6ee39e436, 169}, {0xe7d34c64a9c85d44, 172}, {0x90e40fbeea1d3a4a, 176}, {0xb51d13aea4a488dd, 179}, {0xe264589a4dcdab14, 182}, {0x8d7eb76070a08aec, 186}, {0xb0de65388cc8ada8, 189}, {0xdd15fe86affad912, 192}, {0x8a2dbf142dfcc7ab, 196}, {0xacb92ed9397bf996, 199}, {0xd7e77a8f87daf7fb, 202}, {0x86f0ac99b4e8dafd, 206}, {0xa8acd7c0222311bc, 209}, {0xd2d80db02aabd62b, 212}, {0x83c7088e1aab65db, 216}, {0xa4b8cab1a1563f52, 219}, {0xcde6fd5e09abcf26, 222}, {0x80b05e5ac60b6178, 226}, {0xa0dc75f1778e39d6, 229}, {0xc913936dd571c84c, 232}, {0xfb5878494ace3a5f, 235}, {0x9d174b2dcec0e47b, 239}, {0xc45d1df942711d9a, 242}, {0xf5746577930d6500, 245}, {0x9968bf6abbe85f20, 249}, {0xbfc2ef456ae276e8, 252}, {0xefb3ab16c59b14a2, 255}, {0x95d04aee3b80ece5, 259}, {0xbb445da9ca61281f, 262}, {0xea1575143cf97226, 265}, {0x924d692ca61be758, 269}, {0xb6e0c377cfa2e12e, 272}, {0xe498f455c38b997a, 275}, {0x8edf98b59a373fec, 279}, {0xb2977ee300c50fe7, 282}, {0xdf3d5e9bc0f653e1, 285}, {0x8b865b215899f46c, 289}, {0xae67f1e9aec07187, 292}, {0xda01ee641a708de9, 295}, {0x884134fe908658b2, 299}, {0xaa51823e34a7eede, 302}, {0xd4e5e2cdc1d1ea96, 305}, {0x850fadc09923329e, 309}, {0xa6539930bf6bff45, 312}, {0xcfe87f7cef46ff16, 315}, {0x81f14fae158c5f6e, 319}, {0xa26da3999aef7749, 322}, {0xcb090c8001ab551c, 325}, {0xfdcb4fa002162a63, 328}, {0x9e9f11c4014dda7e, 332}, {0xc646d63501a1511d, 335}, {0xf7d88bc24209a565, 338}, {0x9ae757596946075f, 342}, {0xc1a12d2fc3978937, 345}, {0xf209787bb47d6b84, 348}, {0x9745eb4d50ce6332, 352}, {0xbd176620a501fbff, 355}, {0xec5d3fa8ce427aff, 358}, {0x93ba47c980e98cdf, 362}, {0xb8a8d9bbe123f017, 365}, {0xe6d3102ad96cec1d, 368}, {0x9043ea1ac7e41392, 372}, {0xb454e4a179dd1877, 375}, {0xe16a1dc9d8545e94, 378}, {0x8ce2529e2734bb1d, 382}, {0xb01ae745b101e9e4, 385}, {0xdc21a1171d42645d, 388}, {0x899504ae72497eba, 392}, {0xabfa45da0edbde69, 395}, {0xd6f8d7509292d603, 398}, {0x865b86925b9bc5c2, 402}, {0xa7f26836f282b732, 405}, {0xd1ef0244af2364ff, 408}, {0x8335616aed761f1f, 412}, {0xa402b9c5a8d3a6e7, 415}, {0xcd036837130890a1, 418}, {0x802221226be55a64, 422}, {0xa02aa96b06deb0fd, 425}, {0xc83553c5c8965d3d, 428}, {0xfa42a8b73abbf48c, 431}, {0x9c69a97284b578d7, 435}, {0xc38413cf25e2d70d, 438}, {0xf46518c2ef5b8cd1, 441}, {0x98bf2f79d5993802, 445}, {0xbeeefb584aff8603, 448}, {0xeeaaba2e5dbf6784, 451}, {0x952ab45cfa97a0b2, 455}, {0xba756174393d88df, 458}, {0xe912b9d1478ceb17, 461}, {0x91abb422ccb812ee, 465}, {0xb616a12b7fe617aa, 468}, {0xe39c49765fdf9d94, 471}, {0x8e41ade9fbebc27d, 475}, {0xb1d219647ae6b31c, 478}, {0xde469fbd99a05fe3, 481}, {0x8aec23d680043bee, 485}, {0xada72ccc20054ae9, 488}, {0xd910f7ff28069da4, 491}, {0x87aa9aff79042286, 495}, {0xa99541bf57452b28, 498}, {0xd3fa922f2d1675f2, 501}, {0x847c9b5d7c2e09b7, 505}, {0xa59bc234db398c25, 508}, {0xcf02b2c21207ef2e, 511}, {0x8161afb94b44f57d, 515}, {0xa1ba1ba79e1632dc, 518}, {0xca28a291859bbf93, 521}, {0xfcb2cb35e702af78, 524}, {0x9defbf01b061adab, 528}, {0xc56baec21c7a1916, 531}, {0xf6c69a72a3989f5b, 534}, {0x9a3c2087a63f6399, 538}, {0xc0cb28a98fcf3c7f, 541}, {0xf0fdf2d3f3c30b9f, 544}, {0x969eb7c47859e743, 548}, {0xbc4665b596706114, 551}, {0xeb57ff22fc0c7959, 554}, {0x9316ff75dd87cbd8, 558}, {0xb7dcbf5354e9bece, 561}, {0xe5d3ef282a242e81, 564}, {0x8fa475791a569d10, 568}, {0xb38d92d760ec4455, 571}, {0xe070f78d3927556a, 574}, {0x8c469ab843b89562, 578}, {0xaf58416654a6babb, 581}, {0xdb2e51bfe9d0696a, 584}, {0x88fcf317f22241e2, 588}, {0xab3c2fddeeaad25a, 591}, {0xd60b3bd56a5586f1, 594}, {0x85c7056562757456, 598}, {0xa738c6bebb12d16c, 601}, {0xd106f86e69d785c7, 604}, {0x82a45b450226b39c, 608}, {0xa34d721642b06084, 611}, {0xcc20ce9bd35c78a5, 614}, {0xff290242c83396ce, 617}, {0x9f79a169bd203e41, 621}, {0xc75809c42c684dd1, 624}, {0xf92e0c3537826145, 627}, {0x9bbcc7a142b17ccb, 631}, {0xc2abf989935ddbfe, 634}, {0xf356f7ebf83552fe, 637}, {0x98165af37b2153de, 641}, {0xbe1bf1b059e9a8d6, 644}, {0xeda2ee1c7064130c, 647}, {0x9485d4d1c63e8be7, 651}, {0xb9a74a0637ce2ee1, 654}, {0xe8111c87c5c1ba99, 657}, {0x910ab1d4db9914a0, 661}, {0xb54d5e4a127f59c8, 664}, {0xe2a0b5dc971f303a, 667}, {0x8da471a9de737e24, 671}, {0xb10d8e1456105dad, 674}, {0xdd50f1996b947518, 677}, {0x8a5296ffe33cc92f, 681}, {0xace73cbfdc0bfb7b, 684}, {0xd8210befd30efa5a, 687}, {0x8714a775e3e95c78, 691}, {0xa8d9d1535ce3b396, 694}, {0xd31045a8341ca07c, 697}, {0x83ea2b892091e44d, 701}, {0xa4e4b66b68b65d60, 704}, {0xce1de40642e3f4b9, 707}, {0x80d2ae83e9ce78f3, 711}, {0xa1075a24e4421730, 714}, {0xc94930ae1d529cfc, 717}, {0xfb9b7cd9a4a7443c, 720}, {0x9d412e0806e88aa5, 724}, {0xc491798a08a2ad4e, 727}, {0xf5b5d7ec8acb58a2, 730}, {0x9991a6f3d6bf1765, 734}, {0xbff610b0cc6edd3f, 737}, {0xeff394dcff8a948e, 740}, {0x95f83d0a1fb69cd9, 744}, {0xbb764c4ca7a4440f, 747}, {0xea53df5fd18d5513, 750}, {0x92746b9be2f8552c, 754}, {0xb7118682dbb66a77, 757}, {0xe4d5e82392a40515, 760}, {0x8f05b1163ba6832d, 764}, {0xb2c71d5bca9023f8, 767}, {0xdf78e4b2bd342cf6, 770}, {0x8bab8eefb6409c1a, 774}, {0xae9672aba3d0c320, 777}, {0xda3c0f568cc4f3e8, 780}, {0x8865899617fb1871, 784}, {0xaa7eebfb9df9de8d, 787}, {0xd51ea6fa85785631, 790}, {0x8533285c936b35de, 794}, {0xa67ff273b8460356, 797}, {0xd01fef10a657842c, 800}, {0x8213f56a67f6b29b, 804}, {0xa298f2c501f45f42, 807}, {0xcb3f2f7642717713, 810}, {0xfe0efb53d30dd4d7, 813}, {0x9ec95d1463e8a506, 817}, {0xc67bb4597ce2ce48, 820}, {0xf81aa16fdc1b81da, 823}, {0x9b10a4e5e9913128, 827}, {0xc1d4ce1f63f57d72, 830}, {0xf24a01a73cf2dccf, 833}, {0x976e41088617ca01, 837}, {0xbd49d14aa79dbc82, 840}, {0xec9c459d51852ba2, 843}, {0x93e1ab8252f33b45, 847}, {0xb8da1662e7b00a17, 850}, {0xe7109bfba19c0c9d, 853}, {0x906a617d450187e2, 857}, {0xb484f9dc9641e9da, 860}, {0xe1a63853bbd26451, 863}, {0x8d07e33455637eb2, 867}, {0xb049dc016abc5e5f, 870}, {0xdc5c5301c56b75f7, 873}, {0x89b9b3e11b6329ba, 877}, {0xac2820d9623bf429, 880}, {0xd732290fbacaf133, 883}, {0x867f59a9d4bed6c0, 887}, {0xa81f301449ee8c70, 890}, {0xd226fc195c6a2f8c, 893}, {0x83585d8fd9c25db7, 897}, {0xa42e74f3d032f525, 900}, {0xcd3a1230c43fb26f, 903}, {0x80444b5e7aa7cf85, 907}, {0xa0555e361951c366, 910}, {0xc86ab5c39fa63440, 913}, {0xfa856334878fc150, 916}, {0x9c935e00d4b9d8d2, 920}, {0xc3b8358109e84f07, 923}, {0xf4a642e14c6262c8, 926}, {0x98e7e9cccfbd7dbd, 930}, {0xbf21e44003acdd2c, 933}, {0xeeea5d5004981478, 936}, {0x95527a5202df0ccb, 940}, {0xbaa718e68396cffd, 943}, {0xe950df20247c83fd, 946}, {0x91d28b7416cdd27e, 950}, {0xb6472e511c81471d, 953}, {0xe3d8f9e563a198e5, 956}, {0x8e679c2f5e44ff8f, 960}};

  static const components power_of_five_components[] = {{0xe61acf033d1a45df, -715}, {0x8fd0c16206306bab, -712}, {0xb3c4f1ba87bc8696, -710}, {0xe0b62e2929aba83c, -708}, {0x8c71dcd9ba0b4925, -705}, {0xaf8e5410288e1b6f, -703}, {0xdb71e91432b1a24a, -701}, {0x892731ac9faf056e, -698}, {0xab70fe17c79ac6ca, -696}, {0xd64d3d9db981787d, -694}, {0x85f0468293f0eb4e, -691}, {0xa76c582338ed2621, -689}, {0xd1476e2c07286faa, -687}, {0x82cca4db847945ca, -684}, {0xa37fce126597973c, -682}, {0xcc5fc196fefd7d0c, -680}, {0xff77b1fcbebcdc4f, -678}, {0x9faacf3df73609b1, -675}, {0xc795830d75038c1d, -673}, {0xf97ae3d0d2446f25, -671}, {0x9becce62836ac577, -668}, {0xc2e801fb244576d5, -666}, {0xf3a20279ed56d48a, -664}, {0x9845418c345644d6, -661}, {0xbe5691ef416bd60c, -659}, {0xedec366b11c6cb8f, -657}, {0x94b3a202eb1c3f39, -654}, {0xb9e08a83a5e34f07, -652}, {0xe858ad248f5c22c9, -650}, {0x91376c36d99995be, -647}, {0xb58547448ffffb2d, -645}, {0xe2e69915b3fff9f9, -643}, {0x8dd01fad907ffc3b, -640}, {0xb1442798f49ffb4a, -638}, {0xdd95317f31c7fa1d, -636}, {0x8a7d3eef7f1cfc52, -633}, {0xad1c8eab5ee43b66, -631}, {0xd863b256369d4a40, -629}, {0x873e4f75e2224e68, -626}, {0xa90de3535aaae202, -624}, {0xd3515c2831559a83, -622}, {0x8412d9991ed58091, -619}, {0xa5178fff668ae0b6, -617}, {0xce5d73ff402d98e3, -615}, {0x80fa687f881c7f8e, -612}, {0xa139029f6a239f72, -610}, {0xc987434744ac874e, -608}, {0xfbe9141915d7a922, -606}, {0x9d71ac8fada6c9b5, -603}, {0xc4ce17b399107c22, -601}, {0xf6019da07f549b2b, -599}, {0x99c102844f94e0fb, -596}, {0xc0314325637a1939, -594}, {0xf03d93eebc589f88, -592}, {0x96267c7535b763b5, -589}, {0xbbb01b9283253ca2, -587}, {0xea9c227723ee8bcb, -585}, {0x92a1958a7675175f, -582}, {0xb749faed14125d36, -580}, {0xe51c79a85916f484, -578}, {0x8f31cc0937ae58d2, -575}, {0xb2fe3f0b8599ef07, -573}, {0xdfbdcece67006ac9, -571}, {0x8bd6a141006042bd, -568}, {0xaecc49914078536d, -566}, {0xda7f5bf590966848, -564}, {0x888f99797a5e012d, -561}, {0xaab37fd7d8f58178, -559}, {0xd5605fcdcf32e1d6, -557}, {0x855c3be0a17fcd26, -554}, {0xa6b34ad8c9dfc06f, -552}, {0xd0601d8efc57b08b, -550}, {0x823c12795db6ce57, -547}, {0xa2cb1717b52481ed, -545}, {0xcb7ddcdda26da268, -543}, {0xfe5d54150b090b02, -541}, {0x9efa548d26e5a6e1, -538}, {0xc6b8e9b0709f109a, -536}, {0xf867241c8cc6d4c0, -534}, {0x9b407691d7fc44f8, -531}, {0xc21094364dfb5636, -529}, {0xf294b943e17a2bc4, -527}, {0x979cf3ca6cec5b5a, -524}, {0xbd8430bd08277231, -522}, {0xece53cec4a314ebd, -520}, {0x940f4613ae5ed136, -517}, {0xb913179899f68584, -515}, {0xe757dd7ec07426e5, -513}, {0x9096ea6f3848984f, -510}, {0xb4bca50b065abe63, -508}, {0xe1ebce4dc7f16dfb, -506}, {0x8d3360f09cf6e4bd, -503}, {0xb080392cc4349dec, -501}, {0xdca04777f541c567, -499}, {0x89e42caaf9491b60, -496}, {0xac5d37d5b79b6239, -494}, {0xd77485cb25823ac7, -492}, {0x86a8d39ef77164bc, -489}, {0xa8530886b54dbdeb, -487}, {0xd267caa862a12d66, -485}, {0x8380dea93da4bc60, -482}, {0xa46116538d0deb78, -480}, {0xcd795be870516656, -478}, {0x806bd9714632dff6, -475}, {0xa086cfcd97bf97f3, -473}, {0xc8a883c0fdaf7df0, -471}, {0xfad2a4b13d1b5d6c, -469}, {0x9cc3a6eec6311a63, -466}, {0xc3f490aa77bd60fc, -464}, {0xf4f1b4d515acb93b, -462}, {0x991711052d8bf3c5, -459}, {0xbf5cd54678eef0b6, -457}, {0xef340a98172aace4, -455}, {0x9580869f0e7aac0e, -452}, {0xbae0a846d2195712, -450}, {0xe998d258869facd7, -448}, {0x91ff83775423cc06, -445}, {0xb67f6455292cbf08, -443}, {0xe41f3d6a7377eeca, -441}, {0x8e938662882af53e, -438}, {0xb23867fb2a35b28d, -436}, {0xdec681f9f4c31f31, -434}, {0x8b3c113c38f9f37e, -431}, {0xae0b158b4738705e, -429}, {0xd98ddaee19068c76, -427}, {0x87f8a8d4cfa417c9, -424}, {0xa9f6d30a038d1dbc, -422}, {0xd47487cc8470652b, -420}, {0x84c8d4dfd2c63f3b, -417}, {0xa5fb0a17c777cf09, -415}, {0xcf79cc9db955c2cc, -413}, {0x81ac1fe293d599bf, -410}, {0xa21727db38cb002f, -408}, {0xca9cf1d206fdc03b, -406}, {0xfd442e4688bd304a, -404}, {0x9e4a9cec15763e2e, -401}, {0xc5dd44271ad3cdba, -399}, {0xf7549530e188c128, -397}, {0x9a94dd3e8cf578b9, -394}, {0xc13a148e3032d6e7, -392}, {0xf18899b1bc3f8ca1, -390}, {0x96f5600f15a7b7e5, -387}, {0xbcb2b812db11a5de, -385}, {0xebdf661791d60f56, -383}, {0x936b9fcebb25c995, -380}, {0xb84687c269ef3bfb, -378}, {0xe65829b3046b0afa, -376}, {0x8ff71a0fe2c2e6dc, -373}, {0xb3f4e093db73a093, -371}, {0xe0f218b8d25088b8, -369}, {0x8c974f7383725573, -366}, {0xafbd2350644eeacf, -364}, {0xdbac6c247d62a583, -362}, {0x894bc396ce5da772, -359}, {0xab9eb47c81f5114f, -357}, {0xd686619ba27255a2, -355}, {0x8613fd0145877585, -352}, {0xa798fc4196e952e7, -350}, {0xd17f3b51fca3a7a0, -348}, {0x82ef85133de648c4, -345}, {0xa3ab66580d5fdaf5, -343}, {0xcc963fee10b7d1b3, -341}, {0xffbbcfe994e5c61f, -339}, {0x9fd561f1fd0f9bd3, -336}, {0xc7caba6e7c5382c8, -334}, {0xf9bd690a1b68637b, -332}, {0x9c1661a651213e2d, -329}, {0xc31bfa0fe5698db8, -327}, {0xf3e2f893dec3f126, -325}, {0x986ddb5c6b3a76b7, -322}, {0xbe89523386091465, -320}, {0xee2ba6c0678b597f, -318}, {0x94db483840b717ef, -315}, {0xba121a4650e4ddeb, -313}, {0xe896a0d7e51e1566, -311}, {0x915e2486ef32cd60, -308}, {0xb5b5ada8aaff80b8, -306}, {0xe3231912d5bf60e6, -304}, {0x8df5efabc5979c8f, -301}, {0xb1736b96b6fd83b3, -299}, {0xddd0467c64bce4a0, -297}, {0x8aa22c0dbef60ee4, -294}, {0xad4ab7112eb3929d, -292}, {0xd89d64d57a607744, -290}, {0x87625f056c7c4a8b, -287}, {0xa93af6c6c79b5d2d, -285}, {0xd389b47879823479, -283}, {0x843610cb4bf160cb, -280}, {0xa54394fe1eedb8fe, -278}, {0xce947a3da6a9273e, -276}, {0x811ccc668829b887, -273}, {0xa163ff802a3426a8, -271}, {0xc9bcff6034c13052, -269}, {0xfc2c3f3841f17c67, -267}, {0x9d9ba7832936edc0, -264}, {0xc5029163f384a931, -262}, {0xf64335bcf065d37d, -260}, {0x99ea0196163fa42e, -257}, {0xc06481fb9bcf8d39, -255}, {0xf07da27a82c37088, -253}, {0x964e858c91ba2655, -250}, {0xbbe226efb628afea, -248}, {0xeadab0aba3b2dbe5, -246}, {0x92c8ae6b464fc96f, -243}, {0xb77ada0617e3bbcb, -241}, {0xe55990879ddcaabd, -239}, {0x8f57fa54c2a9eab6, -236}, {0xb32df8e9f3546564, -234}, {0xdff9772470297ebd, -232}, {0x8bfbea76c619ef36, -229}, {0xaefae51477a06b03, -227}, {0xdab99e59958885c4, -225}, {0x88b402f7fd75539b, -222}, {0xaae103b5fcd2a881, -220}, {0xd59944a37c0752a2, -218}, {0x857fcae62d8493a5, -215}, {0xa6dfbd9fb8e5b88e, -213}, {0xd097ad07a71f26b2, -211}, {0x825ecc24c873782f, -208}, {0xa2f67f2dfa90563b, -206}, {0xcbb41ef979346bca, -204}, {0xfea126b7d78186bc, -202}, {0x9f24b832e6b0f436, -199}, {0xc6ede63fa05d3143, -197}, {0xf8a95fcf88747d94, -195}, {0x9b69dbe1b548ce7c, -192}, {0xc24452da229b021b, -190}, {0xf2d56790ab41c2a2, -188}, {0x97c560ba6b0919a5, -185}, {0xbdb6b8e905cb600f, -183}, {0xed246723473e3813, -181}, {0x9436c0760c86e30b, -178}, {0xb94470938fa89bce, -176}, {0xe7958cb87392c2c2, -174}, {0x90bd77f3483bb9b9, -171}, {0xb4ecd5f01a4aa828, -169}, {0xe2280b6c20dd5232, -167}, {0x8d590723948a535f, -164}, {0xb0af48ec79ace837, -162}, {0xdcdb1b2798182244, -160}, {0x8a08f0f8bf0f156b, -157}, {0xac8b2d36eed2dac5, -155}, {0xd7adf884aa879177, -153}, {0x86ccbb52ea94baea, -150}, {0xa87fea27a539e9a5, -148}, {0xd29fe4b18e88640e, -146}, {0x83a3eeeef9153e89, -143}, {0xa48ceaaab75a8e2b, -141}, {0xcdb02555653131b6, -139}, {0x808e17555f3ebf11, -136}, {0xa0b19d2ab70e6ed6, -134}, {0xc8de047564d20a8b, -132}, {0xfb158592be068d2e, -130}, {0x9ced737bb6c4183d, -127}, {0xc428d05aa4751e4c, -125}, {0xf53304714d9265df, -123}, {0x993fe2c6d07b7fab, -120}, {0xbf8fdb78849a5f96, -118}, {0xef73d256a5c0f77c, -116}, {0x95a8637627989aad, -113}, {0xbb127c53b17ec159, -111}, {0xe9d71b689dde71af, -109}, {0x9226712162ab070d, -106}, {0xb6b00d69bb55c8d1, -104}, {0xe45c10c42a2b3b05, -102}, {0x8eb98a7a9a5b04e3, -99}, {0xb267ed1940f1c61c, -97}, {0xdf01e85f912e37a3, -95}, {0x8b61313bbabce2c6, -92}, {0xae397d8aa96c1b77, -90}, {0xd9c7dced53c72255, -88}, {0x881cea14545c7575, -85}, {0xaa242499697392d2, -83}, {0xd4ad2dbfc3d07787, -81}, {0x84ec3c97da624ab4, -78}, {0xa6274bbdd0fadd61, -76}, {0xcfb11ead453994ba, -74}, {0x81ceb32c4b43fcf4, -71}, {0xa2425ff75e14fc31, -69}, {0xcad2f7f5359a3b3e, -67}, {0xfd87b5f28300ca0d, -65}, {0x9e74d1b791e07e48, -62}, {0xc612062576589dda, -60}, {0xf79687aed3eec551, -58}, {0x9abe14cd44753b52, -55}, {0xc16d9a0095928a27, -53}, {0xf1c90080baf72cb1, -51}, {0x971da05074da7bee, -48}, {0xbce5086492111aea, -46}, {0xec1e4a7db69561a5, -44}, {0x9392ee8e921d5d07, -41}, {0xb877aa3236a4b449, -39}, {0xe69594bec44de15b, -37}, {0x901d7cf73ab0acd9, -34}, {0xb424dc35095cd80f, -32}, {0xe12e13424bb40e13, -30}, {0x8cbccc096f5088cb, -27}, {0xafebff0bcb24aafe, -25}, {0xdbe6fecebdedd5be, -23}, {0x89705f4136b4a597, -20}, {0xabcc77118461cefc, -18}, {0xd6bf94d5e57a42bc, -16}, {0x8637bd05af6c69b5, -13}, {0xa7c5ac471b478423, -11}, {0xd1b71758e219652b, -9}, {0x83126e978d4fdf3b, -6}, {0xa3d70a3d70a3d70a, -4}, {0xcccccccccccccccc, -2}, {0x8000000000000000, 0}, {0xa000000000000000, 3}, {0xc800000000000000, 5}, {0xfa00000000000000, 7}, {0x9c40000000000000, 10}, {0xc350000000000000, 12}, {0xf424000000000000, 14}, {0x9896800000000000, 17}, {0xbebc200000000000, 19}, {0xee6b280000000000, 21}, {0x9502f90000000000, 24}};
  //static const uint64_t power_of_five_mantissas[] = {0x8e679c2f5e44ff8f, 0xe3d8f9e563a198e5, 0x16c8e5ca239028e3, 0x2474a2dd05b3749f, 0x3a5437c8091f20ff, 0x5d538c7341cb67fe, 0x95527a5202df0ccb, 0xeeea5d5004981478, 0x17e43c8800759ba5, 0x2639fa7333ef5f6f, 0x3d2990b8531898b2, 0x61dc1ac084f42783, 0x9c935e00d4b9d8d2, 0xfa856334878fc150, 0x190d56b873f4c688, 0x2815578d865470d9, 0x402225af3d53e7c2, 0x669d0918621fd937, 0xa42e74f3d032f525, 0x106b0bb1fb384bb6, 0x1a44df832b8d45f1, 0x2a07cc05127ba31c, 0x433facd4ea5f6b60, 0x6b991487dd657899, 0xac2820d9623bf429, 0x1137367c236c6537, 0x1b8b8a6038ad6ebe, 0x2c1277005aaf1797, 0x4683f19a2ab1bf59, 0x70d31c29dde93228, 0xb484f9dc9641e9da, 0x120d4c2fa8a030fc, 0x1ce2137f74338193, 0x2e368598b9ec0285, 0x49f0d5c129799da2, 0x764e22cea8c295d1, 0xbd49d14aa79dbc82, 0x12edc82110c2f940, 0x1e494034e79e5b99, 0x30753387d8fd5f5c, 0x4d885272f4c89894, 0x7c0d50b7ee0dc0ed, 0xc67bb4597ce2ce48, 0x13d92ba28c7d14a0, 0x1fc1df6a7a61ba9a, 0x32cfcbdd909c5dc4, 0x514c796280fa2fa1, 0x8213f56a67f6b29b, 0xd01fef10a657842c, 0x14cffe4e7708c06a, 0x214cca1724dacd77, 0x3547a9bea15e158c, 0x553f75fdcefcef46, 0x8865899617fb1871, 0xda3c0f568cc4f3e8, 0x15d2ce55747a1864, 0x22eae3bbed902706, 0x37de392caf4d0b3d, 0x59638eade54811fc, 0x8f05b1163ba6832d, 0xe4d5e82392a40515, 0x16e230d05b76cd4e, 0x249d1ae6f8be154b, 0x3a94f7d7f4635544, 0x5dbb262653d22207, 0x95f83d0a1fb69cd9, 0xeff394dcff8a948e, 0x17fec216198ddba7, 0x266469bcf5afc5d9, 0x3d6d75fb22b2d628, 0x6248bcc5045156a7, 0x9d412e0806e88aa5, 0xfb9b7cd9a4a7443c, 0x19292615c3aa539f, 0x2841d689391085cc, 0x40695741f4e73c79, 0x670ef2032171fa5c, 0xa4e4b66b68b65d60, 0x107d457124123c89, 0x1a6208b50683940f, 0x2a367454d738ece5, 0x438a53baf1f4ae3c, 0x6c1085f7e9877d2d, 0xace73cbfdc0bfb7b, 0x114a52dffc679925, 0x1baa1e332d728ea3, 0x2c4363851584176b, 0x46d238d4ef39bf12, 0x71505aee4b8f981d, 0xb54d5e4a127f59c8, 0x1221563a9b732294, 0x1d022390f8b83753, 0x2e69d2818df38bb8, 0x4a42ea68e31f45f3, 0x76d1770e38320986, 0xbe1bf1b059e9a8d6, 0x1302cb5e6f642a7b, 0x1e6adefd7f06aa5f, 0x30aafe6264d776ff, 0x4dde63d0a158be65, 0x7c97061a9bc130a2, 0xc75809c42c684dd1, 0x13ef342d37a407c8, 0x1fe52048590672d9, 0x330833a6f4d71e29, 0x51a6b90b21583042, 0x82a45b450226b39c, 0xd106f86e69d785c7, 0x14e718d7d7625a2d, 0x2171c159589d5d15, 0x3582cef55a9561bc, 0x559e17eef755692d, 0x88fcf317f22241e2, 0xdb2e51bfe9d0696a, 0x15eb082cca94d757, 0x2311a6ae10ee2558, 0x381c3de34e49d55a, 0x59c6c96bb076222a, 0x8fa475791a569d10, 0xe5d3ef282a242e81, 0x16fb97ea6a9d37d9, 0x24c5bfdd7761f2f6, 0x3ad5ffc8bf031e56, 0x5e2332dacb38308a, 0x969eb7c47859e743, 0xf0fdf2d3f3c30b9f, 0x1819651531f9e78f, 0x268f0821e98fd8e6, 0x3db1a69ca8e627d6, 0x62b5d7610e3d0c8b, 0x9defbf01b061adab, 0xfcb2cb35e702af78, 0x1945145230b377f2, 0x286e86e9e7858cb7, 0x40b0d7dca5a27abe, 0x678159610903f797, 0xa59bc234db398c25, 0x108f936baf85c136, 0x1a7f5245e5a2cebe, 0x2a65506fd5d14aca, 0x43d54d7fbc821143, 0x6c887bff94034ed2, 0xada72ccc20054ae9, 0x115d847ad000877d, 0x1bc8d3f7b3340bfc, 0x2c7486591eb9acc7, 0x4720d6f4fdf5e13e, 0x71ce24bb2fefceca, 0xb616a12b7fe617aa, 0x123576845997025d, 0x1d22573a28f19d62, 0x2e9d585d0e4f6237, 0x4a955a2e7d4bd059, 0x77555d172edfb3c2, 0xbeeefb584aff8603, 0x1317e5ef3ab32700, 0x1e8ca3185deb719a, 0x30e104f3c978b5c3, 0x4e34d4b9425abc6b, 0x7d21545b9d5dfa46, 0xc83553c5c8965d3d, 0x1405552d60dbd61f, 0x200888489af95699, 0x3340da0dc4c22428, 0x52015ce2d469d373, 0x8335616aed761f1f, 0xd1ef0244af2364ff, 0x14fe4d06de5056e6, 0x2196e1a496e6f170, 0x35be35d424a4b580, 0x55fd22ed076def34, 0x899504ae72497eba, 0xdc21a1171d42645d, 0x16035ce8b6203d3c, 0x233894a789cd2ec7, 0x385a8772761517a5, 0x5a2a7250bcee8c3b, 0x9043ea1ac7e41392, 0xe6d3102ad96cec1d, 0x17151b377c247e02, 0x24ee91f2603a6337, 0x3b174fea33909ebf, 0x5e8bb3105280fdff, 0x9745eb4d50ce6332, 0xf209787bb47d6b84, 0x183425a5f872f126, 0x26b9d5d65a5181d7, 0x3df622f090826959, 0x63236b1a80d0a88e, 0x9e9f11c4014dda7e, 0xfdcb4fa002162a63, 0x1961219000356aa3, 0x289b68e666bbddd2, 0x40f8a7d70ac62fb7, 0x67f43fbe77a37f8b, 0xa6539930bf6bff45, 0x10a1f5b813246653, 0x1a9cbc59b83a3d52, 0x2a94608f8d29fbb7, 0x44209a7f48432c59, 0x6d00f7320d3846f4, 0xae67f1e9aec07187, 0x1170cb642b133e8d, 0x1be7abd3781eca7c, 0x2ca5dfb8c03143f9, 0x476fcc5acd1b9ff6, 0x724c7a2ae1c5ccbd, 0xb6e0c377cfa2e12e, 0x1249ad2594c37ceb, 0x1d42aea2879f2e44, 0x2ed1176a72984a07, 0x4ae825771dc07672, 0x77d9d58b62cd8a51, 0xbfc2ef456ae276e8, 0x132d17ed577d0be4, 0x1eae8caef261aca0, 0x3117477e509c4766, 0x4e8ba596e760723d, 0x7dac3c24a5671d2f, 0xc913936dd571c84c, 0x141b8ebe2ef1c73a, 0x202c1796b182d85e, 0x3379bf57826af3c9, 0x525c6558d0ab1fa9, 0x83c7088e1aab65db, 0xd2d80db02aabd62b, 0x15159af804446237, 0x21bc2b266d3a36bf, 0x35f9dea3e1f6bdfe, 0x565c976c9cbdfccb, 0x8a2dbf142dfcc7ab, 0xdd15fe86affad912, 0x161bcca7119915b5, 0x235fadd81c2822bb, 0x3899162693736ac5, 0x5a8e89d75252446e, 0x90e40fbeea1d3a4a, 0xe7d34c64a9c85d44, 0x172ebad6ddc73c86, 0x25179157c93ec73e, 0x3b58e88c75313ec9, 0x5ef4a74721e86476, 0x97edd871cfda3a56, 0xf316271c7fc3908a, 0x184f03e93ff9f4da, 0x26e4d30eccc3215d, 0x3e3aeb4ae1383562, 0x63917877cec0556b, 0x9f4f2726179a2245, 0xfee50b7025c36a08, 0x197d4df19d605767, 0x28c87cb5c89a2571, 0x4140c78940f6a24f, 0x6867a5a867f103b2, 0xa70c3c40a64e6c51, 0x10b46c6cdd6e3e08, 0x1aba4714957d300d, 0x2ac3a4edbbfb8014, 0x446c3b15f9926687, 0x6d79f82328ea3da6, 0xaf298d050e4395d6, 0x118427b3b4a05bc8, 0x1c06a5ec5433c60d, 0x2cd76fe086b93ce2, 0x47bf19673df52e37, 0x72cb5bd86321e38c, 0xb7abc627050305ad, 0x125dfa371a19e6f7, 0x1d6329f1c35ca4bf, 0x2f050fe938943acc, 0x4b3b4ca85a86c47a, 0x785ee10d5da46d90, 0xc097ce7bc90715b3, 0x13426172c74d822b, 0x1ed09bead87c0378, 0x314dc6448d9338c1, 0x4ee2d6d415b85ace, 0x7e37be2022c0914b, 0xc9f2c9cd04674ede, 0x1431e0fae6d7217c, 0x204fce5e3e250261, 0x33b2e3c9fd0803ce, 0x52b7d2dcc80cd2e4, 0x84595161401484a0, 0xd3c21bcecceda100, 0x152d02c7e14af680, 0x21e19e0c9bab2400, 0x3635c9adc5dea000, 0x56bc75e2d6310000, 0x8ac7230489e80000, 0xde0b6b3a76400000, 0x16345785d8a00000, 0x2386f26fc1000000, 0x38d7ea4c68000000, 0x5af3107a40000000, 0x9184e72a00000000, 0xe8d4a51000000000, 0x174876e800000000, 0x2540be4000000000, 0x3b9aca0000000000, 0x5f5e100000000000, 0x9896800000000000, 0xf424000000000000, 0x186a000000000000, 0x2710000000000000, 0x3e80000000000000, 0x6400000000000000, 0xa000000000000000, 0x1000000000000000, 0x1999999999999999, 0x28f5c28f5c28f5c2, 0x4189374bc6a7ef9d, 0x68db8bac710cb295, 0xa7c5ac471b478423, 0x10c6f7a0b5ed8d36, 0x1ad7f29abcaf4857, 0x2af31dc4611873bf, 0x44b82fa09b5a52cb, 0x6df37f675ef6eadf, 0xafebff0bcb24aafe, 0x119799812dea1119, 0x1c25c268497681c2, 0x2d09370d42573603, 0x480ebe7b9d58566c, 0x734aca5f6226f0ad, 0xb877aa3236a4b449, 0x12725dd1d243aba0, 0x1d83c94fb6d2ac34, 0x2f394219248446ba, 0x4b8ed0283a6d3df7, 0x78e480405d7b9658, 0xc16d9a0095928a27, 0x1357c299a88ea76a, 0x1ef2d0f5da7dd8aa, 0x318481895d962776, 0x4f3a68dbc8f03f24, 0x7ec3daf941806506, 0xcad2f7f5359a3b3e, 0x14484bfeebc29f86, 0x2073accb12d0ff3d, 0x33ec47ab514e652e, 0x5313a5dee87d6eb0, 0x84ec3c97da624ab4, 0xd4ad2dbfc3d07787, 0x154484932d2e725a, 0x22073a8515171d5d, 0x3671f73b54f1c895, 0x571cbec554b60dbb, 0x8b61313bbabce2c6, 0xdf01e85f912e37a3, 0x164cfda3281e38c3, 0x23ae629ea696c138, 0x391704310a8acec1, 0x5b5806b4ddaae468, 0x9226712162ab070d, 0xe9d71b689dde71af, 0x17624f8a762fd82b, 0x256a18dd89e626ab, 0x3bdcf495a9703ddf, 0x5fc7edbc424d2fcb, 0x993fe2c6d07b7fab, 0xf53304714d9265df, 0x18851a0b548ea3c9, 0x273b5cdeedb1060f, 0x3ec56164af81a34b, 0x646f023ab2690545, 0xa0b19d2ab70e6ed6, 0x1011c2eaabe7d7e2, 0x19b604aaaca62636, 0x29233aaaadd6a38a, 0x41d1f7777c8a9f44, 0x694ff258c7443207, 0xa87fea27a539e9a5, 0x10d9976a5d52975d, 0x1af5bf109550f22e, 0x2b22cb4dbbb4b6b1, 0x4504787c5f878ab5, 0x6e6d8d93cc0c1122, 0xb0af48ec79ace837, 0x11ab20e472914a6b, 0x1c45016d841baa46, 0x2d3b357c0692aa0a, 0x485ebbf9a41ddcdc, 0x73cac65c39c96161, 0xb94470938fa89bce, 0x1286d80ec190dc61, 0x1da48ce468e7c702, 0x2f6dae3a4172d803, 0x4be2b05d35848cd2, 0x796ab3c855a0e151, 0xc24452da229b021b, 0x136d3b7c36a919cf, 0x1f152bf9f10e8fb2, 0x31bb798fe8174c50, 0x4f925c1973587a1b, 0x7f50935bebc0c35e, 0xcbb41ef979346bca, 0x145ecfe5bf520ac7, 0x2097b309321cde0b, 0x3425eb41e9c7c9ac, 0x536fdecfdc72dc47, 0x857fcae62d8493a5, 0xd59944a37c0752a2, 0x155c2076bf9a5510, 0x222d00bdff5d54e6, 0x36ae679665622171, 0x577d728a3bd03581, 0x8bfbea76c619ef36, 0xdff9772470297ebd, 0x1665bf1d3e6a8cac, 0x23d5fe9530aa7aad, 0x39566421e7772aaf, 0x5bbd6d030bf1dde5, 0x92c8ae6b464fc96f, 0xeadab0aba3b2dbe5, 0x177c44ddf6c515fd, 0x2593a163246e8995, 0x3c1f689ea0b0dc22, 0x603240fdcde7c69c, 0x99ea0196163fa42e, 0xf64335bcf065d37d, 0x18a0522c7e709526, 0x2766e9e0ca4dbb70, 0x3f0b0fce107c5f19, 0x64de7fb01a609829, 0xa163ff802a3426a8, 0x1023998cd1053710, 0x19d28f47b4d524e7, 0x2950e53f87bb6e3f, 0x421b0865a5f8b065, 0x69c4da3c3cc11a3c, 0xa93af6c6c79b5d2d, 0x10ec4be0ad8f8951, 0x1b13ac9aaf4c0ee8, 0x2b52adc44bace4a7, 0x45511606df7b0772, 0x6ee8233e325e7250, 0xb1736b96b6fd83b3, 0x11bebdf578b2f391, 0x1c6463225ab7ec1c, 0x2d6d6b6a2abfe02e, 0x48af1243779966b0, 0x744b506bf28f0ab3, 0xba121a4650e4ddeb, 0x129b69070816e2fd, 0x1dc574d80cf16b2f, 0x2fa2548ce1824519, 0x4c36edae359d3b5b, 0x79f17c49ef61f893, 0xc31bfa0fe5698db8, 0x1382cc34ca2427c5, 0x1f37ad21436d0c6f, 0x31f2ae9b9f14e0b2, 0x4feab0f8fe87cde9, 0x7fdde7f4ca72e30f, 0xcc963fee10b7d1b3, 0x14756ccb01abfb5e, 0x20bbe144cf799231, 0x345fced47f28e9e8, 0x53cc7e20cb74a973, 0x8613fd0145877585, 0xd686619ba27255a2, 0x1573d68f903ea229, 0x2252f0e5b39769dc, 0x36eb1b091f58a960, 0x57de91a832277567, 0x8c974f7383725573, 0xe0f218b8d25088b8, 0x167e9c127b6e7412, 0x23fdc683f8b0b9b7, 0x39960a6cc11ac2be, 0x5c2343e134f79dfd, 0x936b9fcebb25c995, 0xebdf661791d60f56, 0x179657025b6234bb, 0x25bd5803c569edf9, 0x3c62266c6f0fe328, 0x609d0a4718196b73, 0x9a94dd3e8cf578b9, 0xf7549530e188c128, 0x18bba884e35a79b7, 0x2792a73b055d8f8b, 0x3f510b91a22f4c12, 0x654e78e9037ee01d, 0xa21727db38cb002f, 0x103583fc527ab337, 0x19ef3993b72ab859, 0x297ec285f1ddf3c2, 0x42646a6fe9631f9d, 0x6a3a43e642383295, 0xa9f6d30a038d1dbc, 0x10ff151a99f482f9, 0x1b31bb5dc320d18e, 0x2b82c562d1ce1c17, 0x459e089e1c7cf9bf, 0x6f6340fcfa618f98, 0xb23867fb2a35b28d, 0x11d270cc51055ea7, 0x1c83e7ad4e6efdd9, 0x2d9fd9154a4b2fc2, 0x48ffc1bbaa11e603, 0x74cc692c434fd66b, 0xbae0a846d2195712, 0x12b010d3e1cf5581, 0x1de6815302e5559c, 0x2fd735519e3bbc2d, 0x4c8b888296c5f9e2, 0x7a78da6a8ad65c9d, 0xc3f490aa77bd60fc, 0x139874ddd8c6234c, 0x1f5a549627a36bad, 0x322a20f03f6bdf7c, 0x504367e6cbdfcbf9, 0x806bd9714632dff6, 0xcd795be870516656, 0x148c22ca71a1bd6f, 0x20e037aa4f692f18, 0x3499f2aa18a84b59, 0x542984435aa6def5, 0x86a8d39ef77164bc, 0xd77485cb25823ac7, 0x158ba6fab6f36c47, 0x22790b2abe5246d8, 0x372811ddfd507159, 0x58401c96621a4ef6, 0x8d3360f09cf6e4bd, 0xe1ebce4dc7f16dfb, 0x169794a160cb57cc, 0x2425ba9bce122613, 0x39d5f75fb01d09b9, 0x5c898bcc4cfb42c2, 0x940f4613ae5ed136, 0xece53cec4a314ebd, 0x17b08617a104ee46, 0x25e73cf29b3b16d6, 0x3ca52e50f85e8af1, 0x61084a1b26fdab1b, 0x9b407691d7fc44f8, 0xf867241c8cc6d4c0, 0x18d71d360e13e213, 0x27be952349b969b8, 0x3f97550542c242c0, 0x65beee6ed136d134, 0xa2cb1717b52481ed, 0x1047824f2bb6d9ca, 0x1a0c03b1df8af611, 0x29acd2b63277f01b, 0x42ae1df050bfe693, 0x6ab02fe6e79970eb, 0xaab37fd7d8f58178, 0x1111f32f2f4bc025, 0x1b4feb7eb212cd09, 0x2bb31264501e14db, 0x45eb50a08030215e, 0x6fdee76733803564, 0xb2fe3f0b8599ef07, 0x11e6398126f5cb1a, 0x1ca38f350b22de90, 0x2dd27ebb4504974d, 0x4950cac53b3a8baf, 0x754e113b91f745e5, 0xbbb01b9283253ca2, 0x12c4cf8ea6b6ec76, 0x1e07b27dd78b13f1, 0x300c50c958de864e, 0x4ce0814227ca707d, 0x7b00ced03faa4d95, 0xc4ce17b399107c22, 0x13ae3591f5b4d936, 0x1f7d228322baf524, 0x3261d0d1d12b21d3, 0x509c814fb511cfb9, 0x80fa687f881c7f8e, 0xce5d73ff402d98e3, 0x14a2f1ffecd15c16, 0x2104b66647b56024, 0x34d4570a0c5566a0, 0x5486f1a9ad557101, 0x873e4f75e2224e68, 0xd863b256369d4a40, 0x15a391d56bdc876c, 0x229f4fbbdfc73f14, 0x37654c5fcc71fe87, 0x58a213cc7a4ffda5, 0x8dd01fad907ffc3b, 0xe2e69915b3fff9f9, 0x16b0a8e891ffff65, 0x244ddb0db666656f, 0x3a162b4923d708b2, 0x5cf04541d2f1a783, 0x94b3a202eb1c3f39, 0xedec366b11c6cb8f, 0x17cad23de82d7ac1, 0x261150630d159135, 0x3ce8809e7b55b522, 0x617400fd9222bb6a, 0x9becce62836ac577, 0xf97ae3d0d2446f25, 0x18f2b061aea07183, 0x27eab3cf7dcd826c, 0x3fddec7f2faf3713, 0x662fe0cb7f7ebe86, 0xa37fce126597973c, 0x1059949b708f28b9, 0x1a28edc580e50df5, 0x29db1608ce3b4988, 0x42f8234149f875a7, 0x6b269ecedcc0bc3e, 0xab70fe17c79ac6ca, 0x1124e63593f5e0ad, 0x1b6e3d2286563449, 0x2be395040a2386db, 0x4638ee6cdd05a492, 0x705b171494d5d41e, 0xb3c4f1ba87bc8696, 0x11fa182c40c60d75, 0x1cc359e067a348bb};

  static size_t lz128(__uint128_t t) {
    uint64_t upper = t >> 64;
    if (upper) {
      return leading_zeroes(upper);
    }
    return 64 + leading_zeroes((uint64_t)t);
  }

  static char *itorange(uint64_t ll, uint64_t start, uint64_t length) {
    char *str = (char *)malloc(length);
    for (int i = 0; i < length; i++) {
      str[i] = '0' + ((ll >> (63 - (i + start))) & 1);
    }
    str[length] = '\0';
    return str;
  }

  static char *itob(uint64_t ll) {
    return itorange(ll, 0, 64);
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
static really_inline bool parse_number(const uint8_t *const buf, ParsedJson &pj,
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
    if (true) {
        components c = power_of_ten_components[power_index];
        uint64_t factor_mantissa = c.mantissa;
        // no need to worry about 10...0 case, as we are lowballing already
        __uint128_t large_mantissa = (__uint128_t)i * factor_mantissa;
        __uint128_t change = large_mantissa ^ (large_mantissa + i);
        size_t lz = lz128(large_mantissa);
        size_t shift = 128 - lz - 54;
        // we could use a mantissa that's a mix of powers of 5 and 10 to see if we can find the best one
        if (true) {//change >> shift == 0) {
          // we're good
          uint64_t mantissa = large_mantissa >> shift;
          mantissa += mantissa & 1;
          mantissa >>= 1;
          mantissa &= ~(1ULL << 52);
          uint64_t real_exponent = c.exp + 1023 + (127 - lz);
          mantissa |= real_exponent << 52; // lz > 64?
          mantissa |= ((uint64_t)negative) << 63; // is this safe? is this bool in [0, 1]?
          double d = *((double *)&mantissa);
          // double correct = strtod((char *)(buf + offset), NULL);
          // assert(d == correct);
          inc1();
          pj.write_tape_double(d);
        } else {
          inc2();
          pj.write_tape_double(strtod((char *)(buf + offset), NULL));
        }
    } else {
        double factor = power_of_ten[power_index];
        factor = negative ? -factor : factor;
        double d = i * factor;
        pj.write_tape_double(d);
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
} // simdjson
#endif
