/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCharToGlyphCache_DEFINED
#define SkCharToGlyphCache_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTo.h"

class SkCharToGlyphCache {
public:
    SkCharToGlyphCache();
    ~SkCharToGlyphCache();

    // return number of unichars cached
    int count() const {
        return fKUnichar.size();
    }

    void reset();       // forget all cache entries (to save memory)

    /**
     *  Given a unichar, return its glyphID (if the return value is nonnegative), else return
     *  ~index of where to insert the computed glyphID.
     *
     *  int64_t result = cache.findGlyphIndex(unichar);
     *  if (result >= 0) {
     *      glyphID = result;
     *  } else {
     *      glyphID = compute_glyph_using_typeface(unichar);
     *      cache.insertCharAndGlyph(SkTo<int>(~result), unichar, glyphID);
     *  }
     */
    int64_t findGlyphIndex(SkUnichar c) const;

    /**
     *  Insert a new char/glyph pair into the cache at the specified index.
     *  See findGlyphIndex() for how to compute the bit-not of the index.
     */
    void insertCharAndGlyph(int index, SkUnichar, SkGlyphID);

    // helper to pre-seed an entry in the cache
    void addCharAndGlyph(SkUnichar unichar, SkGlyphID glyph) {
        int64_t index = this->findGlyphIndex(unichar);
        if (index >= 0) {
            SkASSERT(SkTo<SkGlyphID>(index) == glyph);
        } else {
            this->insertCharAndGlyph(SkTo<int>(~index), unichar, glyph);
        }
    }

private:
    SkTDArray<SkUnichar> fKUnichar;
    SkTDArray<SkGlyphID> fVGlyph;
    double               fDenom;
};

#endif
