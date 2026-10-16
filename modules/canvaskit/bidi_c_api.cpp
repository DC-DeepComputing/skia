/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "modules/skunicode/include/SkUnicode.h"

#if defined(SK_UNICODE_BIDI_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_bidi.h"
#else
#error "SkUnicode bidi component is required but missing"
#endif

#include <emscripten.h>

static SkUnicode* getBidiUnicode() {
    static SkUnicode* unicode = SkUnicodes::Bidi::Make().release();
    return unicode;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int skbidi_getBidiRegions(const uint16_t* text, int textCount, int dir, int32_t* out_regions) {
    SkBidiIterator::Direction direction =
            dir == 1 ? SkBidiIterator::Direction::kLTR : SkBidiIterator::Direction::kRTL;
    int regionCount = 0;
    getBidiUnicode()->forEachBidiRegion(
            text,
            textCount,
            direction,
            [&](uint16_t start, uint16_t end, SkBidiIterator::Level level) {
                out_regions[regionCount * 3 + 0] = start;
                out_regions[regionCount * 3 + 1] = end;
                out_regions[regionCount * 3 + 2] = level;
                regionCount++;
            });
    return regionCount;
}

EMSCRIPTEN_KEEPALIVE
void skbidi_reorderVisual(const SkUnicode::BidiLevel* runLevels,
                          int levelsCount,
                          int32_t* logicalFromVisual) {
    getBidiUnicode()->reorderVisual(runLevels, levelsCount, logicalFromVisual);
}

EMSCRIPTEN_KEEPALIVE
int skbidi_computeCodeUnitFlags(const uint16_t* text, int textCount, uint16_t* out_flags) {
    skia_private::TArray<SkUnicode::CodeUnitFlags, true> flags;
    flags.resize(textCount);
    // SkUnicode::computeCodeUnitFlags expects char16_t* but does not mutate the buffer when
    // replaceTabs is false
    char16_t* utf16 = reinterpret_cast<char16_t*>(const_cast<uint16_t*>(text));
    if (!getBidiUnicode()->computeCodeUnitFlags(utf16, textCount, false, &flags)) {
        return 0;
    }
    for (int i = 0; i < textCount; i++) {
        out_flags[i] = static_cast<uint16_t>(flags[i]);
    }
    return 1;
}

}  // extern "C"
