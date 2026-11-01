/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Where does the fallback trak second pass spend its time: copying the fallback fonts with
// NSCTFontUnscaledTrackingAttribute=0, typesetting the line again, or copying the string?
// Build: clang -O2 -framework CoreText -framework CoreFoundation ct_second_pass_cost.c
#include <CoreText/CoreText.h>
#include <stdio.h>
#include <time.h>

static double now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

static CFDictionaryRef no_trak_attrs(void) {
    CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
                                                         &kCFTypeDictionaryValueCallBacks);
    int zero = 0;
    CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &zero);
    CFDictionarySetValue(d, CFSTR("NSCTFontUnscaledTrackingAttribute"), n);
    CFRelease(n);
    return d;
}

int main(void) {
    CFDictionaryRef nt = no_trak_attrs();
    CTFontDescriptorRef ntDesc = CTFontDescriptorCreateWithAttributes(nt);
    CTFontRef sys = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 17, NULL);
    CTFontRef primary = CTFontCreateCopyWithAttributes(sys, 0, NULL, ntDesc);
    CFStringRef s =
            CFSTR("The quick \u6C34\u6C34 brown fox \U0001F600 jumps \u05E9\u05DC\u05D5 over");
    CFMutableAttributedStringRef as = CFAttributedStringCreateMutable(NULL, 0);
    CFAttributedStringReplaceString(as, CFRangeMake(0, 0), s);
    CFAttributedStringSetAttribute(as, CFRangeMake(0, CFStringGetLength(s)), kCTFontAttributeName,
                                   primary);
    CTLineRef line = CTLineCreateWithAttributedString(as);
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    CTFontRef fb[8];
    int nfb = 0;
    for (CFIndex i = 0; i < CFArrayGetCount(runs); ++i) {
        CTRunRef r = CFArrayGetValueAtIndex(runs, i);
        CTFontRef rf = CFDictionaryGetValue(CTRunGetAttributes(r), kCTFontAttributeName);
        if (!CFEqual(rf, primary) && nfb < 8) fb[nfb++] = rf;
    }
    const int kIters = 2000;
    double t0 = now_us();
    for (int it = 0; it < kIters; ++it) {
        for (int i = 0; i < nfb; ++i) {
            CTFontDescriptorRef d = CTFontDescriptorCreateWithAttributes(nt);
            CTFontRef c = CTFontCreateCopyWithAttributes(fb[i], 0, NULL, d);
            CFRelease(c);
            CFRelease(d);
        }
    }
    double t1 = now_us();
    for (int it = 0; it < kIters; ++it) {
        CTTypesetterRef ts = CTTypesetterCreateWithAttributedString(as);
        CTLineRef l = CTTypesetterCreateLine(ts, CFRangeMake(0, CFStringGetLength(s)));
        CFRelease(l);
        CFRelease(ts);
    }
    double t2 = now_us();
    for (int it = 0; it < kIters; ++it) {
        CFMutableAttributedStringRef m = CFAttributedStringCreateMutableCopy(NULL, 0, as);
        CFRelease(m);
    }
    double t3 = now_us();
    printf("fallback fonts=%d copies(all)=%.2f us typeset=%.2f us mutableCopy=%.2f us\n", nfb,
           (t1 - t0) / kIters, (t2 - t1) / kIters, (t3 - t2) / kIters);
    return 0;
}
