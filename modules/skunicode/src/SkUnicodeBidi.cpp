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

#include "modules/skunicode/src/SkUnicodeBidi.h"

SkUnicodeBidi::UBool SkUnicodeBidi::prepareReorder(const UBiDiLevel* levels,
                                                   int32_t length,
                                                   int32_t* indexMap,
                                                   UBiDiLevel* pMinLevel,
                                                   UBiDiLevel* pMaxLevel) {
    int32_t start;
    UBiDiLevel level, minLevel, maxLevel;

    if (levels == NULL || length <= 0) {
        return FALSE;
    }

    /* determine minLevel and maxLevel */
    minLevel = UBIDI_MAX_EXPLICIT_LEVEL + 1;
    maxLevel = 0;
    for (start = length; start > 0;) {
        level = levels[--start];
        if (level > UBIDI_MAX_EXPLICIT_LEVEL + 1) {
            return FALSE;
        }
        if (level < minLevel) {
            minLevel = level;
        }
        if (level > maxLevel) {
            maxLevel = level;
        }
    }
    *pMinLevel = minLevel;
    *pMaxLevel = maxLevel;

    /* initialize the index map */
    for (start = length; start > 0;) {
        --start;
        indexMap[start] = start;
    }

    return TRUE;
}

void SkUnicodeBidi::reorderVisual(const UBiDiLevel* levels, int32_t length, int32_t* indexMap) {
    int32_t start, end, limit, temp;
    UBiDiLevel minLevel = 0, maxLevel = 0;

    if (indexMap == NULL || !prepareReorder(levels, length, indexMap, &minLevel, &maxLevel)) {
        return;
    }

    /* nothing to do? */
    if (minLevel == maxLevel && (minLevel & 1) == 0) {
        return;
    }

    /* reorder only down to the lowest odd level */
    minLevel |= 1;

    /* loop maxLevel..minLevel */
    do {
        start = 0;

        /* loop for all sequences of levels to reorder at the current maxLevel */
        for (;;) {
            /* look for a sequence of levels that are all at >=maxLevel */
            /* look for the first index of such a sequence */
            while (start < length && levels[start] < maxLevel) {
                ++start;
            }
            if (start >= length) {
                break; /* no more such runs */
            }

            /* look for the limit of such a sequence (the index behind it) */
            for (limit = start; ++limit < length && levels[limit] >= maxLevel;) {
            }

            /*
             * Swap the entire interval of indexes from start to limit-1.
             * We don't need to swap the levels for the purpose of this
             * algorithm: the sequence of levels that we look at does not
             * move anyway.
             */
            end = limit - 1;
            while (start < end) {
                temp = indexMap[start];
                indexMap[start] = indexMap[end];
                indexMap[end] = temp;

                ++start;
                --end;
            }

            if (limit == length) {
                break; /* no more such sequences */
            } else {
                start = limit + 1;
            }
        }
    } while (--maxLevel >= minLevel);
}
