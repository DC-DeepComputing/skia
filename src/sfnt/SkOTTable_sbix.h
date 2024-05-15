/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_sbix_DEFINED
#define SkOTTable_sbix_DEFINED

#include "src/sfnt/SkOTTableTypes.h"
#include "include/private/base/SkSpan_impl.h"

#include <cstdint>

#pragma pack(push, 1)

struct SkOTTableGlyphData;

struct SkOTTableStandardBitmapGraphics {
    static const SK_OT_CHAR TAG0 = 's';
    static const SK_OT_CHAR TAG1 = 'b';
    static const SK_OT_CHAR TAG2 = 'i';
    static const SK_OT_CHAR TAG3 = 'x';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableStandardBitmapGraphics>::value;

    SK_OT_USHORT version; // 1
    SK_OT_USHORT flags;	// Bit 0: 1. Bit 1: Draw outlines. Bits 2 to 15: reserved (set to 0)
    SK_OT_ULONG numStrikes;
    //SK_OT_ULONG strikeOffsets[/*numStrikes*/]; // offset from sbix table to Strike
    SkSpan<SK_OT_ULONG> strikeOffsets() {
        uint32_t strikeCount = SkEndian_SwapBE32(numStrikes);
        return SkSpan<SK_OT_ULONG>(SkTAfter<SK_OT_ULONG>(&numStrikes), strikeCount);
    }
    SkSpan<const SK_OT_ULONG> strikeOffsets() const {
        uint32_t strikeCount = SkEndian_SwapBE32(numStrikes);
        return SkSpan<const SK_OT_ULONG>(SkTAfter<const SK_OT_ULONG>(&numStrikes), strikeCount);
    }

    struct Strike {
        SK_OT_USHORT ppem; // pixels for em
        SK_OT_USHORT ppi; // design pixel density
        //SK_OT_ULONG glyphDataOffsets[/*numGlyphs+1*/]; // offset from Strike to GlyphData
        SK_OT_ULONG* glyphDataOffsets() { return SkTAfter<SK_OT_ULONG>(&ppi); }
        const SK_OT_ULONG* glyphDataOffsets() const { return SkTAfter<const SK_OT_ULONG>(&ppi); }
    };

    struct GlyphData {
        SK_OT_SHORT originOffsetX; // x offset of bitmap in pixels
        SK_OT_SHORT originOffsetY; // y offset of bitmap in pixels (y-up)
        SK_OT_ULONG graphicType; // 'jpg ', 'png ', 'tiff', or 'dupe'
        //SK_OT_BYTE data[]; // length is to next glyphDataOffsets entry
        SK_OT_BYTE* glyphDataOffsets() { return SkTAfter<SK_OT_BYTE>(&graphicType); }
        const SK_OT_BYTE* glyphDataOffsets() const {
            return SkTAfter<const SK_OT_BYTE>(&graphicType);
        }
    };
};

#pragma pack(pop)

#endif
