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

#define UBIDI_MAX_EXPLICIT_LEVEL 125
#define FALSE 0
#define TRUE  1

class SkUnicodeBidi {
public:
    typedef int8_t UBool;
    typedef uint8_t UBiDiLevel;

    static void reorderVisual(const UBiDiLevel* levels, int32_t length, int32_t* indexMap);

private:
    static UBool prepareReorder(const UBiDiLevel* levels,
                                int32_t length,
                                int32_t* indexMap,
                                UBiDiLevel* pMinLevel,
                                UBiDiLevel* pMaxLevel);
};
