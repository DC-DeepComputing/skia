/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkPtrRecorder.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <cstddef>
#include <cstring>

static SkFont serialize_deserialize(const SkFont& font, skiatest::Reporter* reporter) {
    sk_sp<SkRefCntSet> typefaces = sk_make_sp<SkRefCntSet>();
    SkBinaryWriteBuffer wb({});
    wb.setTypefaceRecorder(typefaces);

    SkFontPriv::Flatten(font, wb);
    size_t size = wb.bytesWritten();
    SkAutoMalloc storage(size);
    wb.writeToMemory(storage.get());

    int count = typefaces->count();
    SkASSERT((!font.getTypeface() && count == 0) ||
             ( font.getTypeface() && count == 1));
    if (count) {
        SkTypeface* typeface;
        typefaces->copyToArray((SkRefCnt**)&typeface);
        SkASSERT(typeface == font.getTypeface());
    }

    SkReadBuffer rb(storage.get(), size);
    sk_sp<SkTypeface> cloneTypeface = font.refTypeface();
    if (count) {
        rb.setTypefaceArray(&cloneTypeface, 1);
    }
    SkFont clone;
    REPORTER_ASSERT(reporter, SkFontPriv::Unflatten(&clone, rb));
    return clone;
}

enum {
    kForceAutoHinting      = 1 << 0,
    kEmbeddedBitmaps       = 1 << 1,
    kSubpixel              = 1 << 2,
    kLinearMetrics         = 1 << 3,
    kEmbolden              = 1 << 4,
    kBaselineSnap          = 1 << 5,

    kAllBits = 0x3F,
};

static void apply_flags(SkFont* font, unsigned flags) {
    font->setForceAutoHinting(SkToBool(flags & kForceAutoHinting));
    font->setEmbeddedBitmaps( SkToBool(flags & kEmbeddedBitmaps));
    font->setSubpixel(        SkToBool(flags & kSubpixel));
    font->setLinearMetrics(   SkToBool(flags & kLinearMetrics));
    font->setEmbolden(        SkToBool(flags & kEmbolden));
    font->setBaselineSnap(    SkToBool(flags & kBaselineSnap));
}

DEF_TEST(Font_flatten, reporter) {
    const float sizes[] = {0, 0.001f, 1, 10, 10.001f, 100000.01f};
    const float scales[] = {-5, 0, 1, 5};
    const float skews[] = {-5, 0, 5};
    const SkFont::Edging edges[] = {
        SkFont::Edging::kAlias, SkFont::Edging::kSubpixelAntiAlias
    };
    const SkFontHinting hints[] = {
        SkFontHinting::kNone, SkFontHinting::kFull
    };
    const unsigned int flags[] = {
        kForceAutoHinting, kEmbeddedBitmaps, kSubpixel, kLinearMetrics, kEmbolden, kBaselineSnap,
        kAllBits,
    };
    const sk_sp<SkTypeface> typefaces[] = {
        nullptr, ToolUtils::SampleUserTypeface()
    };

    SkFont font;
    for (float size : sizes) {
        font.setSize(size);
        for (float scale : scales) {
            font.setScaleX(scale);
            for (float skew : skews) {
                font.setSkewX(skew);
                for (auto edge : edges) {
                    font.setEdging(edge);
                    for (auto hint : hints) {
                        font.setHinting(hint);
                        for (auto flag : flags) {
                            apply_flags(&font, flag);
                            for (const sk_sp<SkTypeface>& typeface : typefaces) {
                                font.setTypeface(typeface);
                                SkFont clone = serialize_deserialize(font, reporter);
                                REPORTER_ASSERT(reporter, font == clone);
                            }
                        }
                    }
                }
            }
        }
    }
}

namespace {
struct MalformedText {
    const void* data;
    size_t byteLength;
    SkTextEncoding encoding;
};

const uint16_t kLoneHigh[] = {0xD835};
const uint16_t kLoneLow[] = {0xDC00};
const uint16_t kValidThenLoneHigh[] = {'a', 0xD835};
const uint16_t kOddLength[] = {'a', 'b'};
const char kTruncated4Byte[] = "\xF0\x9F";
const char kStrayContinuation[] = "\x80";
const char kValidThenTruncated[] = "a\xC3";

// Each entry fails SkUTF validation: unpaired surrogates, odd UTF-16 byte length, and UTF-8
// sequences that are truncated or start with a continuation byte
const MalformedText kMalformed[] = {
        {kLoneHigh, sizeof(kLoneHigh), SkTextEncoding::kUTF16},
        {kLoneLow, sizeof(kLoneLow), SkTextEncoding::kUTF16},
        {kValidThenLoneHigh, sizeof(kValidThenLoneHigh), SkTextEncoding::kUTF16},
        {kOddLength, 3, SkTextEncoding::kUTF16},
        {kTruncated4Byte, 2, SkTextEncoding::kUTF8},
        {kStrayContinuation, 1, SkTextEncoding::kUTF8},
        {kValidThenTruncated, 2, SkTextEncoding::kUTF8},
};
}  // namespace

// Malformed UTF-8/UTF-16 has no glyph count; countText and textToGlyphs must report 0 and
// leave the buffer alone instead of returning a wrapped error code that callers allocate
DEF_TEST(Font_MalformedUTF_textToGlyphs, reporter) {
    const SkFont font = ToolUtils::DefaultPortableFont();
    for (const MalformedText& t : kMalformed) {
        REPORTER_ASSERT(reporter, font.countText(t.data, t.byteLength, t.encoding) == 0);

        const SkGlyphID kUntouched[4] = {7, 7, 7, 7};
        SkGlyphID glyphs[4] = {7, 7, 7, 7};
        REPORTER_ASSERT(reporter, font.textToGlyphs(t.data, t.byteLength, t.encoding, glyphs) == 0);
        REPORTER_ASSERT(reporter, memcmp(glyphs, kUntouched, sizeof(glyphs)) == 0);
    }
}

// measureText sizes its glyph storage from countText, so malformed input must measure as
// empty text instead of aborting in the allocator
DEF_TEST(Font_MalformedUTF_measureText, reporter) {
    const SkFont font = ToolUtils::DefaultPortableFont();
    for (const MalformedText& t : kMalformed) {
        SkRect bounds = SkRect::MakeXYWH(1, 2, 3, 4);
        REPORTER_ASSERT(reporter, font.measureText(t.data, t.byteLength, t.encoding, &bounds) == 0);
        REPORTER_ASSERT(reporter, bounds.isEmpty());
    }
}
