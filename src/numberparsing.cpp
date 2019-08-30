//Copyright (c) 2018 Michael Eisel. All rights reserved.

#include <stdio.h>
#include "simdjson/numberparsing.h"

static uint64_t case1 = 0, case2 = 0, case3 = 0;
static uint64_t z = 0;

void inc() {
  z = (z + 1) % 5;
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
}
