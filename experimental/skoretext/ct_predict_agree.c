/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Rejected variant B of the fallback trak fix: does the fallback font predicted by
// CTFontCreateForStringWithLanguage for each grapheme cluster match the font CoreText binds to
// the CTRun when it typesets the line with the untracked primary font? Corpus: 41 texts x 5 base
// fonts x 3 sizes x 4 language tags (none, ja, zh-Hans, ko). Prints per-cluster disagreements
// (DIS), per-script tallies, and an end-to-end check: bind the predicted fonts, typeset once,
// count lines that would still need the second pass and silent font differences vs plain
// typesetting (SILENT, printed with any argument). Env: EXCL=1 adds the exclusion rules (skip
// non-keycap emoji presentation on ASCII, skip Han under a language tag), STICK=1/2 reuses the
// previous fallback font, NOGUARD=1 binds predictions that do not cover the character
// Build: clang -O1 -framework CoreText -framework CoreFoundation ct_predict_agree.c
#include <CoreText/CoreText.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static CFDictionaryRef no_trak_attrs(void) {
    CFMutableDictionaryRef d = CFDictionaryCreateMutable(
            NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    int zero = 0;
    CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &zero);
    CFDictionarySetValue(d, CFSTR("NSCTFontUnscaledTrackingAttribute"), n);
    CFRelease(n);
    return d;
}

static CTFontRef untrack(CTFontRef base, CFDictionaryRef attrs) {
    CTFontDescriptorRef desc = CTFontDescriptorCreateWithAttributes(attrs);
    CTFontRef f = CTFontCreateCopyWithAttributes(base, 0, NULL, desc);
    CFRelease(desc);
    return f;
}

static void ps(CTFontRef f, char* out, size_t n) {
    CFStringRef s = CTFontCopyPostScriptName(f);
    CFStringGetCString(s, out, n, kCFStringEncodingUTF8);
    CFRelease(s);
}

typedef struct {
    const char* script;
    const char* utf8;
} Sample;

static const Sample kSamples[] = {
        {"Latin", "Hello, world 123"},
        {"Latin-ext", "Caf\u00E9 na\u00EFve \u0141\u00F3d\u017A \u0110"},
        {"Latin+combining", "e\u0301 a\u0308 n\u0303"},
        {"Greek", "A \u03B1\u03B2\u03B3 \u0391\u0392"},
        {"Cyrillic", "A \u041F\u0440\u0438\u0432\u0435\u0442"},
        {"CJK-Han", "A\u6C34\u6C34\u6C34 \u65E5\u672C\u8A9E"},
        {"Han-SC-spaced", "A \u4F60 \u597D \u4E16 \u754C B"},
        {"Han-alternating", "\u4F60\u6C34\u597D\u65E5\u4E16\u672C"},
        {"Han-kana-mix", "\u4F60\u597D\u3053\u3093\u306B\u3061\u306F\u4E16\u754C"},
        {"Han-punct-mix", "\u4F60\uFF0C\u597D\u3002\u4E16"},
        {"CJK-Han-SC", "A\u4F60\u597D\u4E16\u754C"},
        {"CJK-ExtB", "A\U00020BB7\U00020BB7"},
        {"Kana", "A\u3042\u3044\u3046\u30AB\u30BF"},
        {"Hangul", "A\uD55C\uAE00\uD55C"},
        {"CJK-punct", "A\u3002\u3001\uFF0C\uFF01\u300C\u300D"},
        {"Hebrew", "A\u05E9\u05DC\u05D5\u05DD"},
        {"Arabic", "A\u0645\u0631\u062D\u0628\u0627"},
        {"Thai", "A\u0E01\u0E02\u0E04 \u0E2A\u0E27\u0E31\u0E2A\u0E14\u0E35"},
        {"Devanagari", "A\u0915\u0916\u0917 \u0928\u092E\u0938\u094D\u0924\u0947"},
        {"Armenian", "A\u0531\u0532\u0533"},
        {"Georgian", "A\u10D0\u10D1\u10D2"},
        {"Tamil", "A\u0B85\u0B86\u0B87"},
        {"Bengali", "A\u0985\u0986\u0987"},
        {"Ethiopic", "A\u12A0\u12A1"},
        {"Khmer", "A\u1780\u1781"},
        {"Emoji-simple", "A\U0001F600\U0001F601\U0001F680"},
        {"Emoji-VS16", "A\u263A\uFE0F\u2764\uFE0F\u00A9\uFE0F"},
        {"Emoji-VS15", "A\u263A\uFE0E\u2764\uFE0E"},
        {"Emoji-textdefault-noVS", "A\u263A\u2764\u2708\u270C"},
        {"Emoji-keycap",
         "A1\uFE0F\u20E3"
         "2\uFE0F\u20E3#\uFE0F\u20E3"},
        {"Emoji-ZWJ",
         "A\U0001F469\u200D\U0001F4BB\U0001F3F3\uFE0F\u200D\U0001F308\u2764\uFE0F\u200D\U0001F525"},
        {"Emoji-skin", "A\U0001F44D\U0001F3FD\U0001F44D\U0001F3FF"},
        {"Emoji-flag", "A\U0001F1FA\U0001F1F8\U0001F1EF\U0001F1F5"},
        {"Emoji-family", "A\U0001F468\u200D\U0001F469\u200D\U0001F467\u200D\U0001F466"},
        {"Symbols", "A\u2192\u2211\u222B\u2260\u20B9\u20BF\u2713\u2714"},
        {"Box/Braille", "A\u2500\u2502\u2801\u2803"},
        {"Punct-common", "A \u2026 \u2014 \u201C\u201D \u00A0 \u2022"},
        {"Ctrl/format",
         "A\u200B"
         "B\u200E"
         "C\u200F"
         "D\u00AD"
         "E\u200D"
         "F\uFE0F"},
        {"PUA", "A\uE000\uF8FF"},
        {"Mixed-latin-cjk-emoji-hebrew", "Hi \u4F60\u597D \U0001F600 \u05E9\u05DC\u05D5\u05DD end"},
        {"Mixed-all",
         "Abc \u65E5\u672C\u8A9E \uD55C\uAE00 \u0639\u0631\u0628\u064A "
         "\u0E20\u0E32\u0E29\u0E32 \u0939\u093F\u0928\u094D\u0926\u0940 "
         "\U0001F469\u200D\U0001F4BB \u03B1\u03B2 \u0416"},
};

static CFMutableAttributedStringRef make(CFStringRef s, CTFontRef font, CFStringRef lang) {
    CFMutableAttributedStringRef as = CFAttributedStringCreateMutable(NULL, 0);
    CFAttributedStringReplaceString(as, CFRangeMake(0, 0), s);
    CFRange all = CFRangeMake(0, CFStringGetLength(s));
    CFAttributedStringSetAttribute(as, all, kCTFontAttributeName, font);
    if (lang)
        CFAttributedStringSetAttribute(as, all, kCTLanguageAttributeName, lang);
    return as;
}

typedef struct {
    long total, agreeName, agreeEq, split, bothSame, bothDiff, predOnly, actOnly, noFb;
} Tally;

static int default_ignorable(UTF32Char c) {
    return c == 0xAD || c == 0x34F || c == 0x61C || (c >= 0x115F && c <= 0x1160) ||
           (c >= 0x17B4 && c <= 0x17B5) || (c >= 0x180B && c <= 0x180F) ||
           (c >= 0x200B && c <= 0x200F) || (c >= 0x202A && c <= 0x202E) ||
           (c >= 0x2060 && c <= 0x206F) || c == 0x3164 || (c >= 0xFE00 && c <= 0xFE0F) ||
           c == 0xFEFF || c == 0xFFA0 || (c >= 0xFFF0 && c <= 0xFFF8) ||
           (c >= 0xE0000 && c <= 0xE0FFF) || (c >= 0x1BCA0 && c <= 0x1BCA3);
}

static UTF32Char first_cp(CFStringRef s, CFRange r) {
    UniChar hi = CFStringGetCharacterAtIndex(s, r.location);
    if (CFStringIsSurrogateHighCharacter(hi) && r.length > 1) {
        UniChar lo = CFStringGetCharacterAtIndex(s, r.location + 1);
        return CFStringGetLongCharacterForSurrogatePair(hi, lo);
    }
    return hi;
}

static int covered(CTFontRef f, UTF32Char c) {
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
    return CTFontGetGlyphsForCharacters(f, u, g, n) && g[0] != 0;
}

typedef struct {
    long clusters, predicted, lines, linesSecondPass, silentDiff, silentDiffLines;
} E2E;

static void e2e(CFStringRef s,
                CTFontRef primary,
                CFDictionaryRef nt,
                CFStringRef lang,
                E2E* out,
                const char* tag,
                const char* base,
                CGFloat size,
                const char* langs,
                int verbose) {
    CFIndex n = CFStringGetLength(s);
    CFMutableAttributedStringRef plain = make(s, primary, lang);
    CTLineRef pl = CTLineCreateWithAttributedString(plain);
    char truth[512][64];
    CGFloat truthSize[512];
    CFArrayRef runs = CTLineGetGlyphRuns(pl);
    for (CFIndex r = 0; r < CFArrayGetCount(runs); ++r) {
        CTRunRef run = CFArrayGetValueAtIndex(runs, r);
        CFRange rr = CTRunGetStringRange(run);
        CTFontRef rf = CFDictionaryGetValue(CTRunGetAttributes(run), kCTFontAttributeName);
        for (CFIndex k = rr.location; k < rr.location + rr.length && k < 512; ++k) {
            ps(rf, truth[k], 64);
            truthSize[k] = CTFontGetSize(rf);
        }
    }
    CFMutableAttributedStringRef as = make(s, primary, lang);
    CFStringRef reqKey = CFSTR("SkParagraphUntrackedFont");
    CFAttributedStringSetAttribute(as, CFRangeMake(0, n), reqKey, primary);
    CTFontRef stick = NULL;
    int stickMode = getenv("STICK") ? atoi(getenv("STICK")) : 0;
    for (CFIndex pos = 0; pos < n;) {
        CFRange cl = CFStringGetRangeOfComposedCharactersAtIndex(s, pos);
        pos = cl.location + cl.length;
        out->clusters++;
        UTF32Char c = first_cp(s, cl);
        int emojiPres = 0;
        for (CFIndex q = cl.location; q < cl.location + cl.length; ++q) {
            UniChar u = CFStringGetCharacterAtIndex(s, q);
            if (u == 0xFE0F || u == 0x20E3)
                emojiPres = 1;
        }
        if (default_ignorable(c))
            continue;
        if (getenv("EXCL")) {
            // ASCII bases only join emoji presentation as keycaps
            if (c < 0x80 && emojiPres) {
                int keycap = 0;
                for (CFIndex q = cl.location; q < cl.location + cl.length; ++q)
                    if (CFStringGetCharacterAtIndex(s, q) == 0x20E3)
                        keycap = 1;
                if (!keycap || !((c >= '0' && c <= '9') || c == '#' || c == '*'))
                    continue;
            }
            // With a language tag CoreText keeps one Han fallback font across a run
            int han = (c >= 0x2E80 && c <= 0x9FFF) || (c >= 0xF900 && c <= 0xFAFF) ||
                      (c >= 0x20000 && c <= 0x3FFFF) || (c >= 0xFF00 && c <= 0xFFEF);
            if (lang && han)
                continue;
        }
        if (!emojiPres && (c < 0x80 || covered(primary, c))) {
            if (stickMode == 2) {
                if (stick)
                    CFRelease(stick);
                stick = NULL;
            }
            continue;
        }
        CTFontRef pred = NULL;
        if (stickMode && stick && covered(stick, c))
            pred = (CTFontRef)CFRetain(stick);
        else
            pred = CTFontCreateForStringWithLanguage(primary, s, cl, lang);
        if (!pred)
            continue;
        if (stickMode) {
            if (stick)
                CFRelease(stick);
            stick = CFEqual(pred, primary) ? NULL : (CTFontRef)CFRetain(pred);
        }
        if (!CFEqual(pred, primary) && (getenv("NOGUARD") || covered(pred, c))) {
            CTFontRef pn = untrack(pred, nt);
            CFAttributedStringSetAttribute(as, cl, kCTFontAttributeName, pn);
            CFAttributedStringSetAttribute(as, cl, reqKey, pn);
            CFRelease(pn);
            out->predicted++;
        }
        CFRelease(pred);
    }
    if (stick)
        CFRelease(stick);
    CTLineRef l = CTLineCreateWithAttributedString(as);
    CFArrayRef runs2 = CTLineGetGlyphRuns(l);
    int need2 = 0, diffLine = 0;
    for (CFIndex r = 0; r < CFArrayGetCount(runs2); ++r) {
        CTRunRef run = CFArrayGetValueAtIndex(runs2, r);
        CFRange rr = CTRunGetStringRange(run);
        CFDictionaryRef ra = CTRunGetAttributes(run);
        CTFontRef rf = CFDictionaryGetValue(ra, kCTFontAttributeName);
        CFTypeRef req = CFDictionaryGetValue(ra, reqKey);
        if (!req || !CFEqual(req, rf)) {
            char nm[64];
            ps(rf, nm, 64);
            if (strcmp(nm, "LastResort"))
                need2 = 1;
        }
        // The second pass binds each such run to its own font, so the font after it is rf
        char nm[64];
        ps(rf, nm, 64);
        for (CFIndex k = rr.location; k < rr.location + rr.length && k < 512; ++k) {
            if (strcmp(nm, truth[k]) || CTFontGetSize(rf) != truthSize[k]) {
                out->silentDiff++;
                diffLine = 1;
                if (verbose) {
                    printf("SILENT base=%s size=%.0f lang=%s [%s] idx=%ld U+%04X final=%s "
                           "truth=%s\n",
                           base, size, langs, tag, (long)k, CFStringGetCharacterAtIndex(s, k), nm,
                           truth[k]);
                }
            }
        }
    }
    out->lines++;
    out->linesSecondPass += need2;
    out->silentDiffLines += diffLine;
    CFRelease(l);
    CFRelease(as);
    CFRelease(pl);
    CFRelease(plain);
}

// Per-cluster agreement for one sample under one base font, size and language; mode 0 also
// runs the end-to-end check
static void agree_one(const char* utf8, const char* script, int mode, const char* baseName,
                      CGFloat size, const char* langName, CTFontRef primary, CFDictionaryRef nt,
                      CFStringRef lang, int verbose, Tally* t, Tally* perMode, E2E* e) {
    CFStringRef s = CFStringCreateWithCString(NULL, utf8, kCFStringEncodingUTF8);
    CFIndex n = CFStringGetLength(s);
    CFMutableAttributedStringRef as = make(s, primary, lang);
    CTLineRef line = CTLineCreateWithAttributedString(as);
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    CTFontRef actual[512] = {0};
    for (CFIndex r = 0; r < CFArrayGetCount(runs); ++r) {
        CTRunRef run = CFArrayGetValueAtIndex(runs, r);
        CFRange rr = CTRunGetStringRange(run);
        CTFontRef rf = CFDictionaryGetValue(CTRunGetAttributes(run), kCTFontAttributeName);
        for (CFIndex k = rr.location; k < rr.location + rr.length && k < 512; ++k) {
            actual[k] = rf;
        }
    }
    char pprim[128];
    ps(primary, pprim, sizeof pprim);
    for (CFIndex pos = 0; pos < n;) {
        CFRange cl = CFStringGetRangeOfComposedCharactersAtIndex(s, pos);
        CTFontRef pred;
        if (mode == 0) {
            pred = CTFontCreateForStringWithLanguage(primary, s, cl, lang);
        } else if (mode == 1) {
            CFStringRef sub = CFStringCreateWithSubstring(NULL, s, cl);
            pred = CTFontCreateForStringWithLanguage(primary, sub, CFRangeMake(0, cl.length),
                                                     lang);
            CFRelease(sub);
        } else {
            pred = CTFontCreateForStringWithLanguage(
                    primary, s, CFRangeMake(cl.location, n - cl.location), lang);
        }
        CTFontRef act = actual[cl.location];
        char pn[128], an[128];
        ps(pred, pn, sizeof pn);
        ps(act, an, sizeof an);
        // Untracked copy of the prediction is what the builder would bind
        CTFontRef predNt = untrack(pred, nt);
        int sameName = strcmp(pn, an) == 0 && CTFontGetSize(pred) == CTFontGetSize(act);
        int sameEq = CFEqual(predNt, act);
        int split = 0;
        for (CFIndex k = cl.location; k < cl.location + cl.length; ++k) {
            if (actual[k] != act) split = 1;
        }
        int predFb = !(strcmp(pn, pprim) == 0 && CTFontGetSize(pred) == CTFontGetSize(primary));
        int actFb = !(strcmp(an, pprim) == 0 && CTFontGetSize(act) == CTFontGetSize(primary));
        if (mode == 0) {
            if (predFb && actFb) {
                if (sameName) {
                    t->bothSame++;
                } else {
                    t->bothDiff++;
                }
            } else if (predFb) {
                t->predOnly++;
            } else if (actFb) {
                t->actOnly++;
            } else {
                t->noFb++;
            }
            t->total++;
            t->agreeName += sameName;
            t->agreeEq += sameEq;
            t->split += split;
        }
        perMode->total++;
        perMode->agreeName += sameName;
        perMode->agreeEq += sameEq;
        perMode->split += split;
        if ((predFb || actFb) && !sameName && (mode == 0 || verbose > 1)) {
            printf("DIS mode=%d base=%s size=%.0f lang=%s [%s] cp=U+%04X len=%ld pred=%s "
                   "act=%s\n",
                   mode, baseName, size, langName, script,
                   CFStringGetCharacterAtIndex(s, cl.location), (long)cl.length, pn, an);
        }
        CFRelease(predNt);
        CFRelease(pred);
        pos = cl.location + cl.length;
    }
    if (mode == 0) e2e(s, primary, nt, lang, e, script, baseName, size, langName, verbose);
    CFRelease(line);
    CFRelease(as);
    CFRelease(s);
}

int main(int argc, char** argv) {
    int verbose = argc > 1;
    const char* bases[] = {".AppleSystemUIFont", "Helvetica", "Times", "Zapfino", "Menlo"};
    const CGFloat sizes[] = {12, 17, 40};
    const char* langs[] = {NULL, "ja", "zh-Hans", "ko"};
    CFDictionaryRef nt = no_trak_attrs();
    size_t nSamples = sizeof(kSamples) / sizeof(kSamples[0]);
    Tally perScript[64] = {0};
    E2E perE2E[64] = {0};
    // Predict inside the full string / in a substring of the cluster / in the rest of the string
    Tally perMode[3] = {0};
    for (int mode = 0; mode < 3; ++mode) {
        for (size_t b = 0; b < 5; ++b) {
            for (size_t zi = 0; zi < 3; ++zi) {
                for (size_t li = 0; li < 4; ++li) {
                    CGFloat size = sizes[zi];
                    CFStringRef bn =
                            CFStringCreateWithCString(NULL, bases[b], kCFStringEncodingUTF8);
                    CTFontRef base = CTFontCreateWithName(bn, size, NULL);
                    CTFontRef primary = untrack(base, nt);
                    CFStringRef lang =
                            langs[li] ? CFStringCreateWithCString(
                                                NULL, langs[li], kCFStringEncodingUTF8)
                                      : NULL;
                    const char* langName = langs[li] ? langs[li] : "-";
                    for (size_t i = 0; i < nSamples; ++i) {
                        agree_one(kSamples[i].utf8, kSamples[i].script, mode, bases[b], size,
                                  langName, primary, nt, lang, verbose, &perScript[i],
                                  &perMode[mode], &perE2E[i]);
                    }
                    if (lang) CFRelease(lang);
                    CFRelease(primary);
                    CFRelease(base);
                    CFRelease(bn);
                }
            }
        }
    }
    printf("\n== per-script (mode 0: predict inside full string) ==\n");
    for (size_t i = 0; i < nSamples; ++i) {
        Tally* t = &perScript[i];
        printf("%-30s clusters=%5ld both-fb-same=%5ld both-fb-DIFF=%4ld pred-fb-only(BAD)=%4ld "
               "act-fb-only(safe)=%4ld no-fb=%5ld split=%ld\n",
               kSamples[i].script, t->total, t->bothSame, t->bothDiff, t->predOnly, t->actOnly,
               t->noFb, t->split);
    }
    printf("\n== end-to-end: predicted fonts bound in builder, typeset once ==\n");
    for (size_t i = 0; i < nSamples; ++i) {
        E2E* e = &perE2E[i];
        printf("%-30s lines=%4ld predicted-clusters=%5ld lines-needing-2nd-pass=%4ld "
               "SILENT-font-diffs=%4ld (lines %ld)\n",
               kSamples[i].script, e->lines, e->predicted, e->linesSecondPass, e->silentDiff,
               e->silentDiffLines);
    }
    const char* mn[] = {"inside-full-string", "substring", "rest-of-string"};
    for (int m = 0; m < 3; ++m) {
        printf("mode %-20s total=%ld name-agree=%.3f%% CFEqual-agree=%.3f%%\n", mn[m],
               perMode[m].total, 100.0 * perMode[m].agreeName / perMode[m].total,
               100.0 * perMode[m].agreeEq / perMode[m].total);
    }
    CFRelease(nt);
    return 0;
}
