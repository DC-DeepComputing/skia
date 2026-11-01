/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Does native kCTTrackingAttributeName / kCTKernAttributeName disable ligatures, and for which
// values? Prints the glyph count of "fi", "ffi" and "office fluffy" per font and value, then the
// glyph positions of combining mark clusters under tracking 1 and kern 1
// Build: clang -O2 -framework CoreText -framework CoreFoundation liga_probe.c
#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CoreText.h>
#include <stdio.h>

static CTLineRef make_line(CFStringRef text, CTFontRef font, CFStringRef key, double value) {
    CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(
            NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(attrs, kCTFontAttributeName, font);
    if (key) {
        CFNumberRef n = CFNumberCreate(NULL, kCFNumberDoubleType, &value);
        CFDictionarySetValue(attrs, key, n);
        CFRelease(n);
    }
    CFAttributedStringRef s = CFAttributedStringCreate(NULL, text, attrs);
    CTLineRef line = CTLineCreateWithAttributedString(s);
    CFRelease(s);
    CFRelease(attrs);
    return line;
}

static CFIndex glyph_count(CTLineRef line) { return CTLineGetGlyphCount(line); }

static void dump_positions(CTLineRef line) {
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    for (CFIndex r = 0; r < CFArrayGetCount(runs); ++r) {
        CTRunRef run = CFArrayGetValueAtIndex(runs, r);
        CFIndex n = CTRunGetGlyphCount(run);
        CGPoint pos[64];
        CGSize adv[64];
        CFIndex idx[64];
        if (n > 64) n = 64;
        CTRunGetPositions(run, CFRangeMake(0, n), pos);
        CTRunGetAdvances(run, CFRangeMake(0, n), adv);
        CTRunGetStringIndices(run, CFRangeMake(0, n), idx);
        for (CFIndex k = 0; k < n; ++k) {
            printf("    [%ld] x=%.4f adv=%.4f\n", (long)idx[k], pos[k].x, adv[k].width);
        }
    }
}

int main(void) {
    const char* fonts[] = {"Helvetica", "Times", "Times New Roman", "Hoefler Text",
                           "Baskerville", "Didot", "Georgia", ".AppleSystemUIFont",
                           "Helvetica Neue", "Palatino"};
    CFStringRef texts[] = {CFSTR("fi"), CFSTR("ffi"), CFSTR("office fluffy")};
    double values[] = {-2, -1, -0.41, -0.01, 0, 0.01, 0.41, 1};
    CFStringRef keys[] = {kCTTrackingAttributeName, kCTKernAttributeName};
    const char* keyNames[] = {"tracking", "kern"};
    printf("== liga: glyph count (chars: fi=2 ffi=3 'office fluffy'=13)\n");
    for (size_t f = 0; f < sizeof(fonts) / sizeof(fonts[0]); ++f) {
        CFStringRef name = CFStringCreateWithCString(NULL, fonts[f], kCFStringEncodingUTF8);
        CTFontRef font = CTFontCreateWithName(name, 24, NULL);
        for (size_t t = 0; t < 3; ++t) {
            CTLineRef none = make_line(texts[t], font, NULL, 0);
            printf("%-20s %-14s none=%ld", fonts[f],
                   CFStringGetCStringPtr(texts[t], kCFStringEncodingUTF8), glyph_count(none));
            CFRelease(none);
            for (int k = 0; k < 2; ++k) {
                printf(" | %s:", keyNames[k]);
                for (size_t v = 0; v < sizeof(values) / sizeof(values[0]); ++v) {
                    CTLineRef l = make_line(texts[t], font, keys[k], values[v]);
                    printf(" %g=%ld", values[v], glyph_count(l));
                    CFRelease(l);
                }
            }
            printf("\n");
        }
        CFRelease(font);
        CFRelease(name);
    }

    printf("\n== combining marks / clusters under tracking 1 vs 0 (17pt system font)\n");
    CTFontRef sys = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 17, NULL);
    UniChar comb[] = {'a', 'e', 0x0301, 'x'};
    UniChar comb2[] = {'a', 'e', 0x0301, 0x0323, 'x'};
    UniChar hebrew[] = {0x05E9, 0x05DC, 0x05D5, 0x05DD};
    UniChar hebMarks[] = {0x05E9, 0x05B8, 0x05DC, 0x05D5, 0x05B9, 0x05DD};
    UniChar emojiZwj[] = {0xD83D, 0xDC68, 0x200D, 0xD83D, 0xDC69, 'a'};
    UniChar precomposed[] = {'a', 0x00E9, 'x'};
    struct { const char* label; UniChar* u; CFIndex n; } cases[] = {
            {"a e U+0301 x", comb, 4},
            {"a e U+0301 U+0323 x", comb2, 5},
            {"a U+00E9 x", precomposed, 3},
            {"Hebrew 4 letters", hebrew, 4},
            {"Hebrew + 2 points", hebMarks, 6},
            {"man ZWJ woman a", emojiZwj, 6},
    };
    for (size_t c = 0; c < sizeof(cases) / sizeof(cases[0]); ++c) {
        CFStringRef s = CFStringCreateWithCharacters(NULL, cases[c].u, cases[c].n);
        CTLineRef l0 = make_line(s, sys, NULL, 0);
        double w0 = CTLineGetTypographicBounds(l0, NULL, NULL, NULL);
        printf("%s: width0=%.4f glyphs=%ld\n", cases[c].label, w0, glyph_count(l0));
        dump_positions(l0);
        for (int k = 0; k < 2; ++k) {
            CTLineRef l1 = make_line(s, sys, keys[k], 1.0);
            double w1 = CTLineGetTypographicBounds(l1, NULL, NULL, NULL);
            double ws = CTLineGetTrailingWhitespaceWidth(l1);
            printf("  %s=1: width=%.4f delta=%.4f trailingWS=%.4f\n", keyNames[k], w1, w1 - w0,
                   ws);
            dump_positions(l1);
            CFRelease(l1);
        }
        CFRelease(l0);
        CFRelease(s);
    }
    return 0;
}
