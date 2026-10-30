/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Control for ct_fallback_trak.c: does each fallback font have its own trak effect when used
// directly, and does the attribute change the primary .SFNS width?
// Build: clang -framework CoreText -framework CoreFoundation ct_fallback_trak_control.c
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

static double width(CFStringRef s, CTFontRef font) {
    CFMutableAttributedStringRef as = CFAttributedStringCreateMutable(NULL, 0);
    CFAttributedStringReplaceString(as, CFRangeMake(0, 0), s);
    CFAttributedStringSetAttribute(as, CFRangeMake(0, CFStringGetLength(s)),
                                   kCTFontAttributeName, font);
    CTLineRef line = CTLineCreateWithAttributedString(as);
    double w = CTLineGetTypographicBounds(line, NULL, NULL, NULL);
    CFRelease(line);
    CFRelease(as);
    return w;
}

int main(void) {
    const char* samples[] = {"AAAAAAAAAA", "\u6C34\u6C34\u6C34", "\uD55C\uAE00\uD55C",
                             "\u05E9\u05DC\u05D5", "\u0E01\u0E02\u0E03", "\u0915\u0916\u0917",
                             "\u0531\u0532\u0533"};
    for (int si = 0; si < 2; ++si) {
        CGFloat size = si == 0 ? 13 : 34;
        CTFontRef sys = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, NULL);
        printf("size %.0f\n", size);
        for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
            CFStringRef s = CFStringCreateWithCString(NULL, samples[i], kCFStringEncodingUTF8);
            CTFontRef fb = CTFontCreateForString(sys, s, CFRangeMake(0, CFStringGetLength(s)));
            CTFontRef plain = with_attr(fb, size, 0);
            CTFontRef noTrak = with_attr(fb, size, 1);
            CFStringRef ps = CTFontCopyPostScriptName(fb);
            char name[128];
            CFStringGetCString(ps, name, sizeof(name), kCFStringEncodingUTF8);
            CFDataRef trak = CTFontCopyTable(fb, 'trak', kCTFontTableOptionNoOptions);
            printf("  %-30s trak_table=%s plain=%8.3f noTrak=%8.3f delta=%+.3f\n", name,
                   trak ? "yes" : "no ", width(s, plain), width(s, noTrak),
                   width(s, plain) - width(s, noTrak));
            if (trak) CFRelease(trak);
            CFRelease(ps);
            CFRelease(plain);
            CFRelease(noTrak);
            CFRelease(fb);
            CFRelease(s);
        }
        CFRelease(sys);
    }
    return 0;
}
