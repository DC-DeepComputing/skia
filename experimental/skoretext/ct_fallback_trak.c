/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Does NSCTFontUnscaledTrackingAttribute=0 on the primary .SFNS font reach CoreText
// cascade fallback fonts? Measures the fallback run width with and without CTTracking
// Build: clang -framework CoreText -framework CoreFoundation ct_fallback_trak.c -o ct_fallback_trak
#include <CoreText/CoreText.h>
#include <stdio.h>

static CTFontRef with_attr(CTFontRef base, CGFloat size, int noTrak) {
    CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
                                                         &kCFTypeDictionaryValueCallBacks);
    if (noTrak) {
        int zero = 0;
        CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &zero);
        CFDictionarySetValue(d, CFSTR("NSCTFontUnscaledTrackingAttribute"), n);
        CFRelease(n);
    }
    CTFontDescriptorRef desc = CTFontDescriptorCreateWithAttributes(d);
    CTFontRef f = CTFontCreateCopyWithAttributes(base, size, NULL, desc);
    CFRelease(desc);
    CFRelease(d);
    return f;
}

// Width of the runs whose font is not the primary font, plus their font name
static double fallback_width(CFStringRef s, CTFontRef font, double tracking, CFStringRef* name) {
    CFMutableAttributedStringRef as = CFAttributedStringCreateMutable(NULL, 0);
    CFAttributedStringReplaceString(as, CFRangeMake(0, 0), s);
    CFRange all = CFRangeMake(0, CFStringGetLength(s));
    CFAttributedStringSetAttribute(as, all, kCTFontAttributeName, font);
    if (tracking != 0) {
        CFNumberRef t = CFNumberCreate(NULL, kCFNumberDoubleType, &tracking);
        CFAttributedStringSetAttribute(as, all, kCTTrackingAttributeName, t);
        CFRelease(t);
    }
    CTLineRef line = CTLineCreateWithAttributedString(as);
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    CFStringRef primary = CTFontCopyPostScriptName(font);
    double w = 0;
    for (CFIndex i = 0; i < CFArrayGetCount(runs); ++i) {
        CTRunRef r = CFArrayGetValueAtIndex(runs, i);
        CTFontRef rf = CFDictionaryGetValue(CTRunGetAttributes(r), kCTFontAttributeName);
        CFStringRef ps = CTFontCopyPostScriptName(rf);
        if (CFStringCompare(ps, primary, 0) != kCFCompareEqualTo) {
            w += CTRunGetTypographicBounds(r, CFRangeMake(0, 0), NULL, NULL, NULL);
            if (name && !*name) *name = CFRetain(ps);
        }
        CFRelease(ps);
    }
    CFRelease(primary);
    CFRelease(line);
    CFRelease(as);
    return w;
}

int main(void) {
    const char* samples[][2] = {
        {"CJK", "A\u6C34\u6C34\u6C34"},     {"Hangul", "A\uD55C\uAE00\uD55C"},
        {"Hebrew", "A\u05E9\u05DC\u05D5"},  {"Thai", "A\u0E01\u0E02\u0E03"},
        {"Devanagari", "A\u0915\u0916\u0917"}, {"Emoji", "A\U0001F600\U0001F601"},
        {"Kana", "A\u3042\u3044\u3046"},    {"Armenian", "A\u0531\u0532\u0533"},
    };
    const double kTrack = 2.0;
    for (int si = 0; si < 2; ++si) {
        CGFloat size = si == 0 ? 13 : 34;
        CTFontRef sys = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, NULL);
        CTFontRef plain = with_attr(sys, size, 0);
        CTFontRef noTrak = with_attr(sys, size, 1);
        printf("size %.0f\n", size);
        for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
            CFStringRef s = CFStringCreateWithCString(NULL, samples[i][1], kCFStringEncodingUTF8);
            CFIndex n = CFStringGetLength(s) - 1;
            CFStringRef fb = NULL;
            double w0 = fallback_width(s, plain, 0, &fb);
            double wN = fallback_width(s, noTrak, 0, NULL);
            double wT = fallback_width(s, noTrak, kTrack, NULL);
            char fbName[128] = "?";
            if (fb) CFStringGetCString(fb, fbName, sizeof(fbName), kCFStringEncodingUTF8);
            // Expected for no double tracking: wT - wN == kTrack per cluster
            printf("  %-11s %-28s plain=%8.3f noTrak=%8.3f tracked=%8.3f "
                   "trak_delta=%+.3f extra_vs_expected=%+.3f\n",
                   samples[i][0], fbName, w0, wN, wT, wN - w0,
                   wT - wN - kTrack * (double)(n));
            if (fb) CFRelease(fb);
            CFRelease(s);
        }
        CFRelease(plain);
        CFRelease(noTrak);
        CFRelease(sys);
    }
    return 0;
}
