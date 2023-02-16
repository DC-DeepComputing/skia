/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_icu_bidi_DEFINED
#define SkUnicode_icu_bidi_DEFINED

#include <unicode/ubidi.h>
#include <unicode/utypes.h>

class SkUnicode_IcuBidi {
public:
    static const char* errorName(UErrorCode status);
    static void close(UBiDi* bidi);
    static UBiDiDirection getDirection(const UBiDi* bidi);
    static SkBidiIterator::Position getLength(const UBiDi* bidi);
    static SkBidiIterator::Level getLevelAt(const UBiDi* bidi, int pos);
    static UBiDi* openSized(int32_t maxLength, int32_t maxRunCount, UErrorCode* pErrorCode);
    static void setPara(UBiDi* bidi,
                        const UChar* text,
                        int32_t length,
                        UBiDiLevel paraLevel,
                        UBiDiLevel* embeddingLevels,
                        UErrorCode* status);
    static void reorderVisual(const SkUnicode::BidiLevel runLevels[],
                              int levelsCount,
                              int32_t logicalFromVisual[]);
};

#endif  // SkUnicode_icu_bidi_DEFINED
