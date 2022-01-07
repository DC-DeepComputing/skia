/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Fixes linking issues with musl due to missing locale-specific string functions.
 *
 * The symbols defined here:
 * https://github.com/llvm/llvm-project/blob/main/libcxx/include/__support/musl/xlocale.h
 * are defined to be inlined, however they are missing during the final linking of a static
 * executable. By re-defining them in our own .a file, this makes the linker happy.
 */

#include <stdlib.h>

long long strtoll_l(const char *nptr, char **endptr, int base,
                    int locale_t) {
  return strtoll(nptr, endptr, base);
}

long long strtoull_l(const char *nptr, char **endptr, int base,
                     int locale_t) {
  return strtoull(nptr, endptr, base);
}
