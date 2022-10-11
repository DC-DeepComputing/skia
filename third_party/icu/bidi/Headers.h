// This code is an exact copy of third_party/externals/icu/source/common/ubidiln.cpp
// so we can drop the ICU dependency when we do not need it
// License & terms of use: http://www.unicode.org/copyright.html
/*
******************************************************************************
*
*   Copyright (C) 1999-2015, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
******************************************************************************
*/
#include "unicode/ubidi.h"

namespace Bidi {
    typedef uint8_t UBiDiLevel;
    static void reorderVisual(const UBiDiLevel* levels, int32_t length, int32_t* indexMap) {
        ubidi_reorderVisual(levels, length, indexMap);
    }
};
