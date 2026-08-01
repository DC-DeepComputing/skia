/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_loca_DEFINED
#include <array>

#define SkOTTable_loca_DEFINED

#include "src/core/SkEndian.h"
#include "src/sfnt/SkOTTableTypes.h"

#pragma pack(push, 1)

struct SkOTTableIndexToLocation {
    static const SK_OT_CHAR TAG0 = 'l';
    static const SK_OT_CHAR TAG1 = 'o';
    static const SK_OT_CHAR TAG2 = 'c';
    static const SK_OT_CHAR TAG3 = 'a';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableIndexToLocation>::value;

    union Offsets {
        std::array<SK_OT_USHORT, 1> shortOffset;
        std::array<SK_OT_ULONG, 1> longOffset;
    } offsets;
};

#pragma pack(pop)

#endif
