/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Does a kept CTTypesetter make re-breaking at a new width cheap, or does every width pay for
// shaping again? Compares: typesetter creation, re-break with a kept typesetter, and SkParagraph
// style (one CTLine for the whole text, breaking done elsewhere)
// Build: clang -O2 -framework CoreText -framework CoreFoundation ct_relayout_cost.c
#include <CoreText/CoreText.h>
#include <mach/mach_time.h>
#include <stdio.h>

static double now_ms(void) {
    static mach_timebase_info_data_t tb;
    if (tb.denom == 0) mach_timebase_info(&tb);
    return (double)mach_absolute_time() * tb.numer / tb.denom / 1e6;
}

static int break_all(CTTypesetterRef ts, CFIndex len, double width, int makeLines) {
    CFIndex start = 0;
    int lines = 0;
    for (int guard = 0; start < len && guard < 100000; ++guard) {
        CFIndex n = CTTypesetterSuggestLineBreak(ts, start, width);
        if (n <= 0) break;
        if (makeLines) {
            CTLineRef l = CTTypesetterCreateLine(ts, CFRangeMake(start, n));
            CFRelease(l);
        }
        start += n;
        ++lines;
    }
    return lines;
}

int main(int argc, char** argv) {
    const char* words = "The quick brown fox jumps over the lazy dog. Съешь же ещё этих мягких "
                        "французских булок. 敏捷的棕色狐狸 ";
    CFMutableStringRef s = CFStringCreateMutable(NULL, 0);
    CFStringRef w = CFStringCreateWithCString(NULL, words, kCFStringEncodingUTF8);
    for (int i = 0; i < 40; ++i) CFStringAppend(s, w);
    CFIndex len = CFStringGetLength(s);
    CTFontRef font;
    if (argc > 1) {
        CFStringRef name = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8);
        font = CTFontCreateWithName(name, 15, NULL);
        CFRelease(name);
    } else {
        font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 15, NULL);
    }
    CFMutableAttributedStringRef as = CFAttributedStringCreateMutable(NULL, 0);
    CFAttributedStringReplaceString(as, CFRangeMake(0, 0), s);
    CFAttributedStringSetAttribute(as, CFRangeMake(0, len), kCTFontAttributeName, font);

    const int kIters = 200;
    const double widths[] = {120, 200, 320, 480, 800};
    const int kW = sizeof(widths) / sizeof(widths[0]);

    // Warm up font caches so the first measurement does not pay for them
    CFRelease(CTTypesetterCreateWithAttributedString(as));

    double t0 = now_ms();
    for (int i = 0; i < kIters; ++i) {
        CTTypesetterRef ts = CTTypesetterCreateWithAttributedString(as);
        CFRelease(ts);
    }
    double createMs = (now_ms() - t0) / kIters;

    t0 = now_ms();
    for (int i = 0; i < kIters; ++i) {
        CTTypesetterRef ts = CTTypesetterCreateWithAttributedString(as);
        CTLineRef l = CTTypesetterCreateLine(ts, CFRangeMake(0, len));
        CFRelease(l);
        CFRelease(ts);
    }
    double endlessMs = (now_ms() - t0) / kIters;

    CTTypesetterRef kept = CTTypesetterCreateWithAttributedString(as);
    int lines = 0;
    t0 = now_ms();
    for (int i = 0; i < kIters; ++i) lines += break_all(kept, len, widths[i % kW], 0);
    double suggestOnlyMs = (now_ms() - t0) / kIters;

    t0 = now_ms();
    for (int i = 0; i < kIters; ++i) break_all(kept, len, widths[i % kW], 1);
    double keptRelayoutMs = (now_ms() - t0) / kIters;

    t0 = now_ms();
    for (int i = 0; i < kIters; ++i) {
        CTTypesetterRef ts = CTTypesetterCreateWithAttributedString(as);
        break_all(ts, len, widths[i % kW], 1);
        CFRelease(ts);
    }
    double freshRelayoutMs = (now_ms() - t0) / kIters;

    printf("text: %ld UTF-16 units, avg %.1f lines per width\n", (long)len,
           (double)lines / kIters);
    printf("typesetter create only:               %8.3f ms\n", createMs);
    printf("typesetter + one endless CTLine:      %8.3f ms\n", endlessMs);
    printf("kept typesetter, SuggestLineBreak:    %8.3f ms per width\n", suggestOnlyMs);
    printf("kept typesetter, Suggest + CTLines:   %8.3f ms per width\n", keptRelayoutMs);
    printf("new typesetter, Suggest + CTLines:    %8.3f ms per width\n", freshRelayoutMs);
    return 0;
}
