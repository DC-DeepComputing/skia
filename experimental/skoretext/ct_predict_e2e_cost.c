/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Rejected variant B of the fallback trak fix: cost of the current two-pass path (typeset, copy
// each fallback font without trak, typeset again) vs predicting the fallback font per grapheme
// cluster with CTFontCreateForStringWithLanguage, binding its untracked copy in the attributed
// string and typesetting once. 17 pt system UI font, no language tag. Prints the median of 3
// runs, each the best of 5 x 2000 lines
// Build: clang -O2 -framework CoreText -framework CoreFoundation ct_predict_e2e_cost.c
#include <CoreText/CoreText.h>
#include <stdio.h>
#include <time.h>

static double now_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

static CTFontRef untrack(CTFontRef base) {
    CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(
            NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    int zero = 0;
    CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &zero);
    CFDictionarySetValue(attrs, CFSTR("NSCTFontUnscaledTrackingAttribute"), n);
    CFRelease(n);
    CTFontDescriptorRef desc = CTFontDescriptorCreateWithAttributes(attrs);
    CTFontRef font = CTFontCreateCopyWithAttributes(base, 0, NULL, desc);
    CFRelease(desc);
    CFRelease(attrs);
    return font;
}

static int covered(CTFontRef font, UTF32Char c) {
    UniChar u[2];
    CGGlyph g[2];
    CFIndex n = 1;
    if (c > 0xFFFF) {
        c -= 0x10000;
        u[0] = 0xD800 + (c >> 10);
        u[1] = 0xDC00 + (c & 0x3FF);
        n = 2;
    } else {
        u[0] = c;
    }
    return CTFontGetGlyphsForCharacters(font, u, g, n) && g[0];
}

// Same private attribute as CoreTextAttrStringBuilder::kUntrackedFontAttribute
static CFStringRef kReq;

static CFMutableAttributedStringRef make(CFStringRef s, CTFontRef primary) {
    CFIndex n = CFStringGetLength(s);
    CFMutableAttributedStringRef as = CFAttributedStringCreateMutable(NULL, 0);
    CFAttributedStringReplaceString(as, CFRangeMake(0, 0), s);
    CFAttributedStringSetAttribute(as, CFRangeMake(0, n), kCTFontAttributeName, primary);
    CFAttributedStringSetAttribute(as, CFRangeMake(0, n), kReq, primary);
    return as;
}

// The CoreTextLineShaper path: a run whose font differs from the tag gets its font copied
// without trak, then the whole line is typeset again
static CTLineRef two_pass(CFStringRef s, CTFontRef primary) {
    CFIndex n = CFStringGetLength(s);
    CFMutableAttributedStringRef as = make(s, primary);
    CTTypesetterRef ts = CTTypesetterCreateWithAttributedString(as);
    CTLineRef line = CTTypesetterCreateLine(ts, CFRangeMake(0, n));
    CFMutableAttributedStringRef as2 = NULL;
    CTFontRef cache[4], cacheCopy[4];
    int nc = 0;
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    for (CFIndex i = 0; i < CFArrayGetCount(runs); i++) {
        CTRunRef run = CFArrayGetValueAtIndex(runs, i);
        CFDictionaryRef ra = CTRunGetAttributes(run);
        CTFontRef rf = (CTFontRef)CFDictionaryGetValue(ra, kCTFontAttributeName);
        CFTypeRef rq = CFDictionaryGetValue(ra, kReq);
        if (!rq || CFEqual(rq, rf))
            continue;
        CTFontRef u = NULL;
        for (int k = 0; k < nc; k++) {
            if (CFEqual(cache[k], rf))
                u = cacheCopy[k];
        }
        if (!u && nc < 4) {
            u = untrack(rf);
            cache[nc] = rf;
            cacheCopy[nc++] = u;
        }
        if (!u)
            continue;
        if (!as2)
            as2 = CFAttributedStringCreateMutableCopy(NULL, 0, as);
        CFAttributedStringSetAttribute(as2, CTRunGetStringRange(run), kCTFontAttributeName, u);
    }
    if (as2) {
        CFRelease(line);
        CFRelease(ts);
        ts = CTTypesetterCreateWithAttributedString(as2);
        line = CTTypesetterCreateLine(ts, CFRangeMake(0, n));
        CFRelease(as2);
    }
    for (int k = 0; k < nc; k++) CFRelease(cacheCopy[k]);
    CFRelease(ts);
    CFRelease(as);
    return line;
}

// Variant B: predict the fallback font for each non-ASCII cluster the primary does not cover,
// bind its untracked copy, typeset once
static CTLineRef predict(CFStringRef s, CTFontRef primary) {
    CFIndex n = CFStringGetLength(s);
    CFMutableAttributedStringRef as = make(s, primary);
    CTFontRef cache[4], cacheCopy[4];
    int nc = 0;
    for (CFIndex pos = 0; pos < n;) {
        CFRange cl = CFStringGetRangeOfComposedCharactersAtIndex(s, pos);
        pos = cl.location + cl.length;
        UniChar u = CFStringGetCharacterAtIndex(s, cl.location);
        if (u < 0x80)
            continue;
        UTF32Char c = u;
        if (CFStringIsSurrogateHighCharacter(u)) {
            c = CFStringGetLongCharacterForSurrogatePair(
                    u, CFStringGetCharacterAtIndex(s, cl.location + 1));
        }
        if (covered(primary, c))
            continue;
        CTFontRef p = CTFontCreateForStringWithLanguage(primary, s, cl, NULL);
        if (!CFEqual(p, primary) && covered(p, c)) {
            CTFontRef pu = NULL;
            for (int k = 0; k < nc; k++) {
                if (CFEqual(cache[k], p))
                    pu = cacheCopy[k];
            }
            if (!pu && nc < 4) {
                pu = untrack(p);
                cache[nc] = p;
                CFRetain(p);
                cacheCopy[nc++] = pu;
            }
            if (pu) {
                CFAttributedStringSetAttribute(as, cl, kCTFontAttributeName, pu);
                CFAttributedStringSetAttribute(as, cl, kReq, pu);
            }
        }
        CFRelease(p);
    }
    CTTypesetterRef ts = CTTypesetterCreateWithAttributedString(as);
    CTLineRef line = CTTypesetterCreateLine(ts, CFRangeMake(0, n));
    for (int k = 0; k < nc; k++) {
        CFRelease(cacheCopy[k]);
        CFRelease(cache[k]);
    }
    CFRelease(ts);
    CFRelease(as);
    return line;
}

int main(void) {
    kReq = CFSTR("SkParagraphUntrackedFont");
    CTFontRef sys = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 17, NULL);
    CTFontRef primary = untrack(sys);
    CFStringRef texts[2] = {
            CFSTR("The quick \u6C34\u6C34 brown fox \U0001F600 jumps \u05E9\u05DC\u05D5 over"),
            CFSTR("The quick brown fox jumps over the lazy dog")};
    const char* textNames[2] = {"latin+cjk+emoji+hebrew", "latin"};
    for (int t = 0; t < 2; t++) {
        for (int m = 0; m < 2; m++) {
            double med[3];
            for (int r = 0; r < 3; r++) {
                double best = 1e9;
                for (int rep = 0; rep < 5; rep++) {
                    double t0 = now_us();
                    for (int it = 0; it < 2000; it++) {
                        CTLineRef l = m ? predict(texts[t], primary) : two_pass(texts[t], primary);
                        CFRelease(l);
                    }
                    double d = (now_us() - t0) / 2000;
                    if (d < best)
                        best = d;
                }
                med[r] = best;
            }
            double a = med[0], b = med[1], c = med[2];
            double mid = a > b ? (a < c ? a : (b > c ? b : c)) : (b < c ? b : (a > c ? a : c));
            printf("%-22s %-28s %.1f us\n",
                   textNames[t],
                   m ? "predict + one typeset" : "two-pass (current)",
                   mid);
        }
    }
    CFRelease(primary);
    CFRelease(sys);
    return 0;
}
