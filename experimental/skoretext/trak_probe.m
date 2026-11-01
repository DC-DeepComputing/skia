/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Does native kern / tracking replace the font's trak or add to it? For each case it prints the
// CTLine width with no attribute (c, trak on), with NSKernAttributeName and NSTrackingAttributeName
// (also via boundingRect and UILabel / NSTextField), and two models: e = c + x * graphemes (spacing
// stacks on trak) and d = u0 + x * graphemes, where u0 copies every run font with
// NSCTFontUnscaledTrackingAttribute=0 (spacing replaces trak, the earlier SkoreText scheme)
// Build and run for macOS and the iOS simulator: see README.md
#include <CoreText/CoreText.h>
#include <TargetConditionals.h>
#include <stdio.h>
#if TARGET_OS_IOS
#import <UIKit/UIKit.h>
#define XFont UIFont
#else
#import <AppKit/AppKit.h>
#define XFont NSFont
#endif

static CTFontRef copyNoTrak(CTFontRef f) {
    int zero = 0;
    CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &zero);
    NSDictionary* attrs = @{@"NSCTFontUnscaledTrackingAttribute" : (__bridge id)n};
    CTFontDescriptorRef d = CTFontDescriptorCreateWithAttributes((__bridge CFDictionaryRef)attrs);
    CTFontRef r = CTFontCreateCopyWithAttributes(f, 0.0, NULL, d);
    CFRelease(d);
    CFRelease(n);
    return r;
}

static CTLineRef makeLine(NSAttributedString* as) {
    return CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)as);
}

static double lineWidth(CTLineRef l) { return CTLineGetTypographicBounds(l, NULL, NULL, NULL); }

// SkoreText model: primary font without trak, then fallback runs rebound to their trak-free copy
static double skoretextUntrackedWidth(NSString* s, CTFontRef font, NSString** fontsOut) {
    CTFontRef nt = copyNoTrak(font);
    NSMutableAttributedString* as = [[NSMutableAttributedString alloc]
            initWithString:s
                attributes:@{(id)kCTFontAttributeName : (__bridge id)nt}];
    CTLineRef line = makeLine(as);
    CFArrayRef runs = CTLineGetGlyphRuns(line);
    bool changed = false;
    NSMutableString* fonts = [NSMutableString string];
    for (CFIndex i = 0; i < CFArrayGetCount(runs); ++i) {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runs, i);
        CTFontRef rf = (CTFontRef)CFDictionaryGetValue(CTRunGetAttributes(run),
                                                       kCTFontAttributeName);
        NSString* ps = CFBridgingRelease(CTFontCopyPostScriptName(rf));
        [fonts appendFormat:@"%@%@", i ? @"," : @"", ps];
        if (!CFEqual(rf, nt)) {
            CTFontRef u = copyNoTrak(rf);
            CFRange r = CTRunGetStringRange(run);
            [as addAttribute:(id)kCTFontAttributeName
                       value:(__bridge id)u
                       range:NSMakeRange(r.location, r.length)];
            CFRelease(u);
            changed = true;
        }
    }
    if (fontsOut) *fontsOut = fonts;
    if (changed) {
        CFRelease(line);
        line = makeLine(as);
    }
    double w = lineWidth(line);
    CFRelease(line);
    CFRelease(nt);
    return w;
}

static NSUInteger graphemes(NSString* s) {
    __block NSUInteger n = 0;
    [s enumerateSubstringsInRange:NSMakeRange(0, s.length)
                          options:NSStringEnumerationByComposedCharacterSequences
                       usingBlock:^(NSString* a, NSRange b, NSRange c, BOOL* d) { ++n; }];
    return n;
}

static double ctWidth(NSString* s, XFont* font, NSString* key, double x) {
    NSMutableDictionary* a = [@{NSFontAttributeName : font} mutableCopy];
    if (key) a[key] = @(x);
    NSAttributedString* as = [[NSAttributedString alloc] initWithString:s attributes:a];
    CTLineRef l = makeLine(as);
    double w = lineWidth(l);
    CFRelease(l);
    return w;
}

static double rectWidth(NSString* s, XFont* font, NSString* key, double x) {
    NSMutableDictionary* a = [@{NSFontAttributeName : font} mutableCopy];
    if (key) a[key] = @(x);
    NSAttributedString* as = [[NSAttributedString alloc] initWithString:s attributes:a];
    CGRect r = [as boundingRectWithSize:CGSizeMake(10000, 10000)
                                options:NSStringDrawingUsesLineFragmentOrigin
                                context:nil];
    return r.size.width;
}

static double viewWidth(NSString* s, XFont* font, NSString* key, double x) {
    NSMutableDictionary* a = [@{NSFontAttributeName : font} mutableCopy];
    if (key) a[key] = @(x);
    NSAttributedString* as = [[NSAttributedString alloc] initWithString:s attributes:a];
#if TARGET_OS_IOS
    UILabel* l = [[UILabel alloc] init];
    l.attributedText = as;
    return [l sizeThatFits:CGSizeMake(10000, 10000)].width;
#else
    NSTextField* t = [NSTextField labelWithAttributedString:as];
    return t.intrinsicContentSize.width;
#endif
}

static void runCase(const char* label, NSString* s, XFont* font, double x) {
    CTFontRef ct = (__bridge CTFontRef)font;
    NSUInteger g = graphemes(s);
    NSString* fonts = nil;
    double c = ctWidth(s, font, nil, 0);
    double u0 = skoretextUntrackedWidth(s, ct, &fonts);
    double a = ctWidth(s, font, NSKernAttributeName, x);
    double b = ctWidth(s, font, NSTrackingAttributeName, x);
    double d = u0 + x * g;
    double e = c + x * g;
    double ar = rectWidth(s, font, NSKernAttributeName, x);
    double br = rectWidth(s, font, NSTrackingAttributeName, x);
    double av = viewWidth(s, font, NSKernAttributeName, x);
    double bv = viewWidth(s, font, NSTrackingAttributeName, x);
    double cv = viewWidth(s, font, nil, 0);
    printf("%-6s %5.1f %-11s x=%5.2f g=%2lu | c=%8.3f u0=%8.3f trak=%7.3f | kern=%8.3f "
           "track=%8.3f | d_skt=%8.3f e_stack=%8.3f | kern-d=%7.3f kern-e=%7.3f "
           "track-d=%7.3f track-e=%7.3f | rect k=%8.3f t=%8.3f | view k=%7.2f t=%7.2f c=%7.2f "
           "| %s\n",
           label, CTFontGetSize(ct),
           [[CFBridgingRelease(CTFontCopyPostScriptName(ct)) substringToIndex:MIN(11,
               [CFBridgingRelease(CTFontCopyPostScriptName(ct)) length])] UTF8String],
           x, (unsigned long)g, c, u0, c - u0, a, b, d, e, a - d, a - e, b - d, b - e, ar, br,
           av, bv, cv, fonts.UTF8String);
}

int main(void) {
    @autoreleasepool {
        NSArray* texts = @[ @"Hello World", @"你好世界", @"שלום", @"Hi 你好 שלום" ];
        const char* labels[] = {"latin", "cjk", "hebrew", "mixed"};
        double sizes[] = {11, 13, 17, 28, 48};
        double xs[] = {0, -0.41, 1, 5};
        for (int si = 0; si < 5; ++si) {
            XFont* f = [XFont systemFontOfSize:sizes[si]];
            for (int ti = 0; ti < 4; ++ti)
                for (int xi = 0; xi < 4; ++xi) runCase(labels[ti], texts[ti], f, xs[xi]);
        }
        printf("\n# Cupertino styles (x = Flutter letterSpacing)\n");
        struct { const char* n; double size; int weight; double x; } cs[] = {
                {"body17", 17, 0, -0.41}, {"navT17", 17, 600, -0.41},
                {"act15", 15, 0, -0.23}, {"tab10", 10, 500, -0.24},
                {"large34", 34, 700, 0.38}, {"pick21", 21, 0, -0.6},
                {"dtpk21", 21, 0, 0.4}};
        for (int i = 0; i < 7; ++i) {
            double w = cs[i].weight == 600 ? 0.3 : cs[i].weight == 500 ? 0.23
                     : cs[i].weight == 700 ? 0.4 : 0.0;
            XFont* f = [XFont systemFontOfSize:cs[i].size weight:w];
            runCase(cs[i].n, @"Hello World", f, cs[i].x);
            runCase(cs[i].n, @"Settings", f, cs[i].x);
            runCase(cs[i].n, @"你好世界", f, cs[i].x);
        }
        printf("\n# per-glyph advances, latin 17pt\n");
        XFont* f17 = [XFont systemFontOfSize:17];
        NSString* keys[] = {@"none", NSKernAttributeName, NSTrackingAttributeName};
        for (int k = 0; k < 3; ++k) {
            NSMutableDictionary* a = [@{NSFontAttributeName : f17} mutableCopy];
            if (k) a[keys[k]] = @(1.0);
            CTLineRef l = makeLine([[NSAttributedString alloc] initWithString:@"AVAWAY"
                                                                   attributes:a]);
            CTRunRef r = (CTRunRef)CFArrayGetValueAtIndex(CTLineGetGlyphRuns(l), 0);
            CFIndex n = CTRunGetGlyphCount(r);
            CGSize adv[32];
            CTRunGetAdvances(r, CFRangeMake(0, n), adv);
            printf("%-22s:", k ? keys[k].UTF8String : "none");
            for (CFIndex j = 0; j < n; ++j) printf(" %.3f", adv[j].width);
            printf(" | width %.3f trailingWS %.3f\n", lineWidth(l),
                   CTLineGetTrailingWhitespaceWidth(l));
            CFRelease(l);
        }
        CTFontRef nt = copyNoTrak((__bridge CTFontRef)f17);
        CTLineRef l = makeLine([[NSAttributedString alloc]
                initWithString:@"AVAWAY"
                    attributes:@{(id)kCTFontAttributeName : (__bridge id)nt}]);
        CTRunRef r = (CTRunRef)CFArrayGetValueAtIndex(CTLineGetGlyphRuns(l), 0);
        CFIndex n = CTRunGetGlyphCount(r);
        CGSize adv[32];
        CTRunGetAdvances(r, CFRangeMake(0, n), adv);
        printf("%-22s:", "notrak");
        for (CFIndex j = 0; j < n; ++j) printf(" %.3f", adv[j].width);
        printf(" | width %.3f\n", lineWidth(l));
    }
    return 0;
}
