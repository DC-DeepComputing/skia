/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Current letterSpacing scheme: no kCTTrackingAttributeName, the primary font carries
// NSCTFontUnscaledTrackingAttribute=0. Fallback runs keep their own trak unless a second pass
// pins them to their font copied with the attribute; "target" is that copy typeset alone
// Build: clang -framework CoreText -framework CoreFoundation ct_fallback_trak_v2.c
#include <CoreText/CoreText.h>
#include <stdio.h>
#include <string.h>

static CFDictionaryRef no_trak_attrs(void) {
    CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
                                                         &kCFTypeDictionaryValueCallBacks);
    int zero = 0;
    CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &zero);
    CFDictionarySetValue(d, CFSTR("NSCTFontUnscaledTrackingAttribute"), n);
    CFRelease(n);
    return d;
}

static CTFontRef copy_attrs(CTFontRef base, CGFloat size, CFDictionaryRef attrs) {
    CTFontDescriptorRef desc = CTFontDescriptorCreateWithAttributes(attrs);
    CTFontRef f = CTFontCreateCopyWithAttributes(base, size, NULL, desc);
    CFRelease(desc);
    return f;
}

static CFMutableAttributedStringRef make(CFStringRef s, CTFontRef font) {
    CFMutableAttributedStringRef as = CFAttributedStringCreateMutable(NULL, 0);
    CFAttributedStringReplaceString(as, CFRangeMake(0, 0), s);
    CFAttributedStringSetAttribute(as, CFRangeMake(0, CFStringGetLength(s)), kCTFontAttributeName,
                                   font);
    return as;
}

// Width of runs whose font has another PostScript name than the primary; the name of the first
// such font goes to name
static double fallback_width(CTLineRef line, CTFontRef primary, char* name, size_t nameLen) {
    CFStringRef pps = CTFontCopyPostScriptName(primary);
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    double w = 0;
    for (CFIndex i = 0; i < CFArrayGetCount(runs); ++i) {
        CTRunRef r = CFArrayGetValueAtIndex(runs, i);
        CTFontRef rf = CFDictionaryGetValue(CTRunGetAttributes(r), kCTFontAttributeName);
        CFStringRef ps = CTFontCopyPostScriptName(rf);
        if (CFStringCompare(ps, pps, 0) != kCFCompareEqualTo) {
            w += CTRunGetTypographicBounds(r, CFRangeMake(0, 0), NULL, NULL, NULL);
            if (name && !name[0]) CFStringGetCString(ps, name, nameLen, kCFStringEncodingUTF8);
        }
        CFRelease(ps);
    }
    CFRelease(pps);
    return w;
}

static CTLineRef second_pass(CFMutableAttributedStringRef as, CTLineRef first, CTFontRef primary,
                             CFDictionaryRef nt) {
    CFStringRef pps = CTFontCopyPostScriptName(primary);
    CFArrayRef runs = CTLineGetGlyphRuns(first);
    for (CFIndex i = 0; i < CFArrayGetCount(runs); ++i) {
        CTRunRef r = CFArrayGetValueAtIndex(runs, i);
        CTFontRef rf = CFDictionaryGetValue(CTRunGetAttributes(r), kCTFontAttributeName);
        CFStringRef ps = CTFontCopyPostScriptName(rf);
        if (CFStringCompare(ps, pps, 0) != kCFCompareEqualTo) {
            CTFontRef pinned = copy_attrs(rf, CTFontGetSize(rf), nt);
            CFAttributedStringSetAttribute(as, CTRunGetStringRange(r), kCTFontAttributeName,
                                           pinned);
            CFRelease(pinned);
        }
        CFRelease(ps);
    }
    CFRelease(pps);
    return CTLineCreateWithAttributedString(as);
}

int main(void) {
    const char* samples[][2] = {
            {"CJK", "A\u6C34\u6C34\u6C34"},      {"Kana", "A\u3042\u3044\u3046"},
            {"Hangul", "A\uD55C\uAE00\uD55C"},   {"Arabic", "A\u0645\u0631\u062D"},
            {"Hebrew", "A\u05E9\u05DC\u05D5"},   {"Thai", "A\u0E01\u0E02\u0E03"},
            {"Devanagari", "A\u0915\u0916\u0917"}, {"Armenian", "A\u0531\u0532\u0533"},
            {"Emoji", "A\U0001F600\U0001F601"},
    };
    const CGFloat sizes[] = {9, 12, 17, 24, 40};
    CFDictionaryRef nt = no_trak_attrs();
    printf("primary|size|script|fallback|plain|current|target|fixB|current-target\n");
    for (int pi = 0; pi < 2; ++pi) {
        for (size_t zi = 0; zi < sizeof(sizes) / sizeof(sizes[0]); ++zi) {
            CGFloat size = sizes[zi];
            CTFontRef base = pi == 0
                    ? CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, NULL)
                    : CTFontCreateWithName(CFSTR("Helvetica"), size, NULL);
            CTFontRef current = copy_attrs(base, size, nt);
            for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
                CFStringRef s =
                        CFStringCreateWithCString(NULL, samples[i][1], kCFStringEncodingUTF8);
                CFStringRef tail = CFStringCreateWithSubstring(
                        NULL, s, CFRangeMake(1, CFStringGetLength(s) - 1));
                char fbName[128] = "";
                CFMutableAttributedStringRef asPlain = make(s, base);
                CTLineRef lPlain = CTLineCreateWithAttributedString(asPlain);
                double wPlain = fallback_width(lPlain, base, fbName, sizeof(fbName));
                CFMutableAttributedStringRef as0 = make(s, current);
                CTLineRef l0 = CTLineCreateWithAttributedString(as0);
                double wCur = fallback_width(l0, current, NULL, 0);
                CTFontRef fb = CTFontCreateForString(current, tail,
                                                     CFRangeMake(0, CFStringGetLength(tail)));
                CTFontRef fbNt = copy_attrs(fb, size, nt);
                CFMutableAttributedStringRef tas = make(tail, fbNt);
                CTLineRef tl = CTLineCreateWithAttributedString(tas);
                double target = CTLineGetTypographicBounds(tl, NULL, NULL, NULL);
                CTLineRef lB = second_pass(as0, l0, current, nt);
                double wB = fallback_width(lB, current, NULL, 0);
                printf("%s|%.0f|%s|%s|%.3f|%.3f|%.3f|%.3f|%+.3f\n",
                       pi == 0 ? ".SFNS" : "Helvetica", size, samples[i][0], fbName, wPlain,
                       wCur, target, wB, wCur - target);
                CFRelease(lB); CFRelease(tl); CFRelease(tas); CFRelease(fbNt); CFRelease(fb);
                CFRelease(l0); CFRelease(as0); CFRelease(lPlain); CFRelease(asPlain);
                CFRelease(tail); CFRelease(s);
            }
            CFRelease(current);
            CFRelease(base);
        }
    }
    CFRelease(nt);
    return 0;
}
