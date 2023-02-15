/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_icu_bidi_DEFINED
#define SkUnicode_icu_bidi_DEFINED

#ifdef SK_UNICODE_ICU_IMPLEMENTATION
#include "modules/skunicode/src/SkUnicode_icu.h"
#else
#include "modules/skunicode/src/SkUnicode_client.h"
#endif

class SkUnicode_CommonBidi {
public:
    static const char* common_errorName(UErrorCode status);
    static void bidi_close(UBiDi* bidi);
    static UBiDiDirection common_bidi_getDirection(const UBiDi* bidi);
    static SkBidiIterator::Position common_bidi_getLength(const UBiDi* bidi);
    static SkBidiIterator::Level common_bidi_getLevelAt(const UBiDi* bidi, int pos);
    static UBiDi* common_bidi_openSized(int32_t maxLength, int32_t maxRunCount, UErrorCode* pErrorCode);
    static void common_bidi_setPara(UBiDi* bidi,
                                    const UChar* text,
                                    int32_t length,
                                    UBiDiLevel paraLevel,
                                    UBiDiLevel* embeddingLevels,
                                    UErrorCode* status);
    static void common_bidi_reorderVisual(const SkUnicode::BidiLevel runLevels[],
                                          int levelsCount,
                                          int32_t logicalFromVisual[]);
};

#endif // SkUnicode_icu_bidi_DEFINED
