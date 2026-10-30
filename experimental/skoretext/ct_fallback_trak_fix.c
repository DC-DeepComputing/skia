/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Two candidate fixes for fallback fonts keeping their own trak: A puts the attribute on every
// default cascade list entry, B pins each fallback run to its CTRun font copied with the attribute
// argv[1] is the kCTTrackingAttributeName value: 2 by default (old scheme), 0 is the current one
// Build: clang -framework CoreText -framework CoreFoundation ct_fallback_trak_fix.c
#include <CoreText/CoreText.h>
#include <stdio.h>
#include <stdlib.h>

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

static CTFontRef with_no_trak_cascade(CTFontRef sys, CGFloat size) {
    CFDictionaryRef nt = no_trak_attrs();
    CFArrayRef cascade = CTFontCopyDefaultCascadeListForLanguages(sys, NULL);
    CFMutableArrayRef fixed = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    for (CFIndex i = 0; i < CFArrayGetCount(cascade); ++i) {
        CTFontDescriptorRef d = CTFontDescriptorCreateCopyWithAttributes(
                CFArrayGetValueAtIndex(cascade, i), nt);
        CFArrayAppendValue(fixed, d);
        CFRelease(d);
    }
    CFMutableDictionaryRef attrs = CFDictionaryCreateMutableCopy(NULL, 0, nt);
    CFDictionarySetValue(attrs, kCTFontCascadeListAttribute, fixed);
    CTFontRef f = copy_attrs(sys, size, attrs);
    CFRelease(attrs);
    CFRelease(fixed);
    CFRelease(cascade);
    CFRelease(nt);
    return f;
}

static CFMutableAttributedStringRef make(CFStringRef s, CTFontRef font, double tracking) {
    CFMutableAttributedStringRef as = CFAttributedStringCreateMutable(NULL, 0);
    CFAttributedStringReplaceString(as, CFRangeMake(0, 0), s);
    CFRange all = CFRangeMake(0, CFStringGetLength(s));
    CFAttributedStringSetAttribute(as, all, kCTFontAttributeName, font);
    CFNumberRef t = CFNumberCreate(NULL, kCFNumberDoubleType, &tracking);
    CFAttributedStringSetAttribute(as, all, kCTTrackingAttributeName, t);
    CFRelease(t);
    return as;
}

static double fallback_width(CTLineRef line, CTFontRef primary) {
    CFStringRef pps = CTFontCopyPostScriptName(primary);
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    double w = 0;
    for (CFIndex i = 0; i < CFArrayGetCount(runs); ++i) {
        CTRunRef r = CFArrayGetValueAtIndex(runs, i);
        CTFontRef rf = CFDictionaryGetValue(CTRunGetAttributes(r), kCTFontAttributeName);
        CFStringRef ps = CTFontCopyPostScriptName(rf);
        if (CFStringCompare(ps, pps, 0) != kCFCompareEqualTo) {
            w += CTRunGetTypographicBounds(r, CFRangeMake(0, 0), NULL, NULL, NULL);
        }
        CFRelease(ps);
    }
    CFRelease(pps);
    return w;
}

// Pins every non-primary run to its own font plus the attribute, then typesets again
static CTLineRef second_pass(CFMutableAttributedStringRef as, CTLineRef first, CTFontRef primary,
                             CGFloat size) {
    CFDictionaryRef nt = no_trak_attrs();
    CFStringRef pps = CTFontCopyPostScriptName(primary);
    CFArrayRef runs = CTLineGetGlyphRuns(first);
    for (CFIndex i = 0; i < CFArrayGetCount(runs); ++i) {
        CTRunRef r = CFArrayGetValueAtIndex(runs, i);
        CTFontRef rf = CFDictionaryGetValue(CTRunGetAttributes(r), kCTFontAttributeName);
        CFStringRef ps = CTFontCopyPostScriptName(rf);
        if (CFStringCompare(ps, pps, 0) != kCFCompareEqualTo) {
            CTFontRef pinned = copy_attrs(rf, size, nt);
            CFAttributedStringSetAttribute(as, CTRunGetStringRange(r), kCTFontAttributeName,
                                           pinned);
            CFRelease(pinned);
        }
        CFRelease(ps);
    }
    CFRelease(pps);
    CFRelease(nt);
    return CTLineCreateWithAttributedString(as);
}

int main(int argc, char** argv) {
    const char* samples[][2] = {{"CJK", "A\u6C34\u6C34\u6C34"},
                                {"Hebrew", "A\u05E9\u05DC\u05D5"},
                                {"Devanagari", "A\u0915\u0916\u0917"},
                                {"Armenian", "A\u0531\u0532\u0533"}};
    const double kTrack = argc > 1 ? atof(argv[1]) : 2.0;
    CFDictionaryRef nt = no_trak_attrs();
    for (int si = 0; si < 2; ++si) {
        CGFloat size = si == 0 ? 13 : 34;
        CTFontRef sys = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, NULL);
        CTFontRef current = copy_attrs(sys, size, nt);
        CTFontRef fixA = with_no_trak_cascade(sys, size);
        printf("size %.0f (target = fallback font with attribute, untracked, + 3*%.0f)\n", size,
               kTrack);
        for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
            CFStringRef s = CFStringCreateWithCString(NULL, samples[i][1], kCFStringEncodingUTF8);
            CFRange tailRange = CFRangeMake(1, CFStringGetLength(s) - 1);
            CFStringRef tail = CFStringCreateWithSubstring(NULL, s, tailRange);
            CFRange whole = CFRangeMake(0, CFStringGetLength(tail));
            CTFontRef fb = CTFontCreateForString(sys, tail, whole);
            CTFontRef fbNt = copy_attrs(fb, size, nt);
            CFMutableAttributedStringRef tas = make(tail, fbNt, 0);
            CTLineRef tl = CTLineCreateWithAttributedString(tas);
            double target = CTLineGetTypographicBounds(tl, NULL, NULL, NULL) + 3 * kTrack;

            CFMutableAttributedStringRef as0 = make(s, current, kTrack);
            CTLineRef l0 = CTLineCreateWithAttributedString(as0);
            CFMutableAttributedStringRef asA = make(s, fixA, kTrack);
            CTLineRef lA = CTLineCreateWithAttributedString(asA);
            CTLineRef lB = second_pass(as0, l0, current, size);
            printf("  %-11s target=%8.3f current=%8.3f fixA=%8.3f fixB=%8.3f\n", samples[i][0],
                   target, fallback_width(l0, current), fallback_width(lA, fixA),
                   fallback_width(lB, current));
            CFRelease(lB); CFRelease(lA); CFRelease(asA); CFRelease(l0); CFRelease(as0);
            CFRelease(tl); CFRelease(tas); CFRelease(fbNt); CFRelease(fb); CFRelease(tail);
            CFRelease(s);
        }
        CFRelease(fixA); CFRelease(current); CFRelease(sys);
    }
    CFRelease(nt);
    return 0;
}
