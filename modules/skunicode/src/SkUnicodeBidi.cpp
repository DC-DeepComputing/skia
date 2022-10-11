// Copyright 2022 Google LLC
// This code copied from third_party/externals/icu/source/common/ubidiln.cpp
// so we can drop the ICU dependency when we do not need it

#include "modules/skunicode/include/SkUnicode.h"

#define UBIDI_MAX_EXPLICIT_LEVEL 125

namespace {

bool prepareReorder(const SkUnicode::BidiLevel* levels,
                    int32_t length,
                    int32_t* indexMap,
                    SkUnicode::BidiLevel* pMinLevel,
                    SkUnicode::BidiLevel* pMaxLevel) {
    int32_t start;
    SkUnicode::BidiLevel level, minLevel, maxLevel;

    if (levels == NULL || length <= 0) {
        return false;
    }

    // Determine minLevel and maxLevel
    minLevel = UBIDI_MAX_EXPLICIT_LEVEL + 1;
    maxLevel = 0;
    for (start = length; start > 0;) {
        level = levels[--start];
        if (level > UBIDI_MAX_EXPLICIT_LEVEL + 1) {
            return false;
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

    // Initialize the index map
    for (start = length; start > 0;) {
        --start;
        indexMap[start] = start;
    }

    return true;
}
}  // namespace

void SkUnicode::reorderVisualRuns(const uint8_t* levels, int32_t length, int32_t* indexMap) {
    int32_t start, end, limit, temp;
    SkUnicode::BidiLevel minLevel = 0, maxLevel = 0;

    if (indexMap == NULL || !prepareReorder(levels, length, indexMap, &minLevel, &maxLevel)) {
        return;
    }

    // Nothing to do?
    if (minLevel == maxLevel && (minLevel & 1) == 0) {
        return;
    }

    // Reorder only down to the lowest odd level
    minLevel |= 1;

    // Loop maxLevel..minLevel
    do {
        start = 0;

        // Loop for all sequences of levels to reorder at the current maxLevel
        for (;;) {
            // Look for a sequence of levels that are all at >=maxLevel
            // Look for the first index of such a sequence
            while (start < length && levels[start] < maxLevel) {
                ++start;
            }
            if (start >= length) {
                // No more such runs
                break;
            }

            // Look for the limit of such a sequence (the index behind it)
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
                // No more such sequences
                break;
            } else {
                start = limit + 1;
            }
        }
    } while (--maxLevel >= minLevel);
}
