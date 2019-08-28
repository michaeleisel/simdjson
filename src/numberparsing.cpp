//Copyright (c) 2018 Michael Eisel. All rights reserved.

#include <stdio.h>
#include <numberparsing.h>

static int z = 0;

void inc() {
  z = (z + 1) % 3;
}

static char buffer[40];

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
      if (likely((upper & 0x1FF) != 0x1FF)) {// || lower + i > i) {
        uint64_t mantissa = 0;
        if (upper & (1ULL << 63)) {
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
        pj.write_tape_double(d);
      } else {
        //d = strtod((char *)(buf + offset), NULL);
        //pj.write_tape_double(d);
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
      }
    } else if (z == 2) {
      double factor = power_of_ten[power_index];
      factor = negative ? -factor : factor;
      d = i * factor;
      pj.write_tape_double(d);
    } else if (z == 3) {
      //long double
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
