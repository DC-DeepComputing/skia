/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Fixes linking issues with musl due to missing _chk versions of standard library calls.
 * https://stackoverflow.com/a/55246403
 * It is not feasible for now to re-build those libraries with musl, so we implement the missing
 * _chk version of the APIs here.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

typedef long int __fd_mask;

// musl does not define this macro, but glibc does
#define __NFDBITS	(8 * (int) sizeof (__fd_mask))

static void check_failed() {
    printf("musl_compat check failed\n");
    abort();
}

// https://elixir.bootlin.com/glibc/latest/source/debug/fdelt_chk.c
long int __fdelt_chk(long int d) {
    if (__builtin_expect(d < 0 || d >= FD_SETSIZE, 0)) {
        check_failed();
    }
    return d / __NFDBITS;
}

// http://refspecs.linux-foundation.org/LSB_4.1.0/LSB-Core-generic/LSB-Core-generic/libc---fprintf-chk-1.html
int __fprintf_chk(FILE* stream, int flag, const char* format, ...) {
    // In theory, one should check that format is read-only if it contains an %n, that is, it was
    // hard coded into the binary, and not user-provided. Search for "%n exploit" for more info.
    // We just forward the arguments on via vprintf https://stackoverflow.com/a/21540808
    va_list args;
    va_start(args, format);
    int rv = vfprintf(stream, format, args);
    va_end(args);
    return rv;
}

// http://refspecs.linux-foundation.org/LSB_4.1.0/LSB-Core-generic/LSB-Core-generic/libc---memcpy-chk-1.html
void* __memcpy_chk(void *dest, const void *src, size_t len, size_t dstlen) {
    if (__builtin_expect(dstlen < len, 0)) {
        check_failed();
    }
    return memcpy(dest, src, len);
}

// http://refspecs.linux-foundation.org/LSB_4.0.0/LSB-Core-generic/LSB-Core-generic/libc---snprintf-chk-1.html
int __snprintf_chk(char* str, size_t maxlen, int flag, size_t strlen, const char* format, ...) {
    if (__builtin_expect(maxlen > strlen, 0)) {
        check_failed();
    }
    // Like __fprintf_chk, we could check for %n here if we wanted.
    va_list args;
    va_start(args, format);
    int rv = vsnprintf(str, maxlen, format, args);
    va_end(args);
    return rv;
}

// http://refspecs.linux-foundation.org/LSB_4.0.0/LSB-Core-generic/LSB-Core-generic/libc---sprintf-chk-1.html
int __sprintf_chk(char* str, int flag, size_t strlen, const char* format, ...) {
    if (__builtin_expect(strlen == 0, 0)) {
        check_failed();
    }
    // Like __fprintf_chk, we could check for %n here if we wanted.
    va_list args;
    va_start(args, format);
    // Write no more than the destination buffer's length.
    int rv = vsnprintf(str, strlen, format, args);
    va_end(args);
    return rv;
}

// Aborts if the strings a and b overlap.
static void check_overlap(char* a, size_t alen, const char* b, size_t blen) {
    if (!a || !b || alen == 0 || blen == 0) {
        return;
    }
    if (__builtin_expect(a == b, 0)) {
        check_failed();
    } else if (a < b) {
        if (a + alen > b) { // b starts before a ends
            check_failed();
        }
    } else {
        if (b + blen > a) {  // a starts before b ends
            check_failed();
        }
    }
}

// https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib---strcat-chk-1.html
char* __strcat_chk(char* dest, const char* src, size_t destlen) {
    size_t srclen = strlen(src);
    if (__builtin_expect(srclen > destlen, 0)) {
        check_failed();
    }
    check_overlap(dest, destlen, src, srclen);
    return strcat(dest, src);
}

// https://refspecs.linuxbase.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib---strcpy-chk-1.html
char* __strcpy_chk(char* dest, const char * src, size_t destlen) {
    size_t srclen = strlen(src);
    if (__builtin_expect(srclen > destlen, 0)) {
        check_failed();
    }
    check_overlap(dest, destlen, src, srclen);
    return strcpy(dest, src);
}

// http://refspecs.linux-foundation.org/LSB_4.0.0/LSB-Core-generic/LSB-Core-generic/libc---strncpy-chk-1.html
char* __strncpy_chk(char* dest, const char* src, size_t n, size_t dstlen) {
    if (__builtin_expect(dstlen < n, 0)) {
        check_failed();
    }
    return strncpy(dest, src, n);
}

// http://refspecs.linux-foundation.org/LSB_4.1.0/LSB-Core-generic/LSB-Core-generic/libc---vfprintf-chk-1.html
int __vfprintf_chk(FILE* fp, int flag, const char* format, va_list ap) {
    // Like __fprintf_chk, we could check for %n here if we wanted.
    return vfprintf(fp, format, ap);
}
