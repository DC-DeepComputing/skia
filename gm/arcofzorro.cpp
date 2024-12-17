/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"

#include <vector>

// To use this test framework:
//   run ThreadedCompilePrecompileTest
//   copy the second part of the output to the "Original" spot and reorder it
//   copy the first part of the output to the "kOutputFromRealLogging" section
//   recompile
//   run the arcofzorro GM
//   copy the output to the "Reconstructed" spot
//   copy both the "Original" and "Reconstructed" sections to Webdiff
//       - they should match modulo minor differences in the times (due to different output paths)
//
// The goal of this process is to ensure that the tracing output method has all the information
// of the intrusive logging method.
// Additionally, this process verifies that all the logging satisfies the expected FSM (i.e., the
// pipeline lifetime is behaving as expected.

static const struct LoggingOutput {
    int fID;
    uint64_t fTime;
    const char* fTag;
    uint32_t fHash;
    uint32_t fPipelineID1;
} kOutputFromRealLogging[] = {
        { 0, 3236, "CacheMissForP", 477900207, 0 },
        { 0, 3251, "CacheMissForP", 477900207, 1 },
        { 0, 3263, "BeginBuildP", 477900207, 0 },
        { 0, 3263, "BeginBuildP", 477900207, 1 },
        { 0, 41152, "CacheMissForN", 477900207, 2 },
        { 0, 41173, "CacheMissForN", 477900207, 3 },
        { 0, 41176, "BeginBuildN", 477900207, 2 },
        { 0, 41176, "BeginBuildN", 477900207, 3 },
        { 0, 87615, "AddedN", 477900207, 3 },
        { 0, 87631, "NWonRaceOverN", 477900207, 2 },
        { 0, 87640, "WaitBeginN", 477900207, 2 },
        { 1, 88650, "CacheMissForN", 2189112322, 4 },
        { 1, 88665, "BeginBuildN", 2189112322, 4 },
        { 0, 89031, "WaitBeginN", 477900207, 3 },
        { 1, 123147, "AddedN", 2189112322, 4 },
        { 2, 123536, "CacheMissForN", 1147256140, 5 },
        { 2, 123540, "BeginBuildN", 1147256140, 5 },
        { 0, 159129, "PurgedN", 477900207, 3 },
        { 2, 159144, "AddedN", 1147256140, 5 },
        { 3, 159636, "CacheMissForN", 883660955, 6 },
        { 3, 159644, "BeginBuildN", 883660955, 6 },
        { 1, 193874, "PurgedN", 2189112322, 4 },
        { 3, 193887, "AddedN", 883660955, 6 },
        { 4, 194261, "CacheMissForN", 644542265, 7 },
        { 4, 194266, "BeginBuildN", 644542265, 7 },
        { 2, 230389, "PurgedN", 1147256140, 5 },
        { 4, 230399, "AddedN", 644542265, 7 },
        { 5, 230763, "CacheMissForN", 2491771, 8 },
        { 5, 230768, "BeginBuildN", 2491771, 8 },
        { 0, 258594, "WaitEndN", 477900207, 2 },
        { 0, 258607, "DeletionN", 477900207, 2 },
        { 3, 258653, "PurgedN", 883660955, 6 },
        { 0, 258659, "AddedP", 477900207, 1 },
        { 1, 258693, "CacheMissForP", 2189112322, 9 },
        { 1, 258695, "BeginBuildP", 2189112322, 9 },
        { 1, 259037, "CacheMissForN", 2189112322, 10 },
        { 1, 259042, "BeginBuildN", 2189112322, 10 },
        { 0, 259131, "PWonRaceOverP", 477900207, 0 },
        { 0, 259156, "DeletionP", 477900207, 0 },
        { 1, 259226, "CacheMissForP", 2189112322, 11 },
        { 1, 259234, "BeginBuildP", 2189112322, 11 },
        { 4, 266446, "PurgedN", 644542265, 7 },
        { 5, 266456, "AddedN", 2491771, 8 },
        { 6, 266820, "CacheMissForN", 3234710487, 12 },
        { 6, 266825, "BeginBuildN", 3234710487, 12 },
        { 0, 266916, "WaitEndN", 477900207, 3 },
        { 1, 268067, "WaitBeginN", 2189112322, 4 },
        { 1, 296934, "WaitEndN", 2189112322, 4 },
        { 2, 297574, "WaitBeginN", 1147256140, 5 },
        { 0, 298131, "BadPurgedP", 477900207, 1 },
        { 0, 298137, "DeletionP", 477900207, 1 },
        { 1, 298139, "AddedP", 2189112322, 11 },
        { 2, 298178, "CacheMissForP", 1147256140, 13 },
        { 2, 298181, "BeginBuildP", 1147256140, 13 },
        { 1, 299425, "PWonRaceOverP", 2189112322, 9 },
        { 1, 299433, "DeletionP", 2189112322, 9 },
        { 2, 299449, "CacheMissForP", 1147256140, 14 },
        { 2, 299451, "BeginBuildP", 1147256140, 14 },
        { 5, 302022, "PurgedN", 2491771, 8 },
        { 6, 302032, "AddedN", 3234710487, 12 },
        { 7, 302459, "CacheMissForN", 1997496709, 15 },
        { 7, 302463, "BeginBuildN", 1997496709, 15 },
        { 1, 302628, "PWonRaceOverN", 2189112322, 10 },
        { 1, 302632, "WaitBeginN", 2189112322, 10 },
        { 1, 302652, "WaitEndN", 2189112322, 10 },
        { 1, 302655, "DeletionN", 2189112322, 10 },
        { 2, 302879, "CacheMissForN", 1147256140, 16 },
        { 2, 302885, "BeginBuildN", 1147256140, 16 },
        { 2, 331177, "WaitEndN", 1147256140, 5 },
        { 3, 332029, "WaitBeginN", 883660955, 6 },
        { 6, 338628, "PurgedN", 3234710487, 12 },
        { 2, 338643, "AddedN", 1147256140, 16 },
        { 3, 339080, "CacheMissForN", 883660955, 17 },
        { 3, 339085, "BeginBuildN", 883660955, 17 },
        { 1, 339385, "BadPurgedP", 2189112322, 11 },
        { 7, 339387, "AddedN", 1997496709, 15 },
        { 8, 339630, "CacheMissForN", 1301841557, 18 },
        { 8, 339633, "BeginBuildN", 1301841557, 18 },
        { 2, 342870, "NWonRaceOverP", 1147256140, 13 },
        { 2, 342883, "DeletionP", 1147256140, 13 },
        { 3, 342963, "CacheMissForP", 883660955, 19 },
        { 3, 342966, "BeginBuildP", 883660955, 19 },
        { 2, 343835, "NWonRaceOverP", 1147256140, 14 },
        { 2, 343842, "DeletionP", 1147256140, 14 },
        { 3, 343889, "CacheMissForP", 883660955, 20 },
        { 3, 343891, "BeginBuildP", 883660955, 20 },
        { 3, 354418, "WaitEndN", 883660955, 6 },
        { 4, 354691, "WaitBeginN", 644542265, 7 },
        { 7, 373614, "PurgedN", 1997496709, 15 },
        { 3, 373628, "AddedN", 883660955, 17 },
        { 4, 374060, "CacheMissForN", 644542265, 21 },
        { 4, 374065, "BeginBuildN", 644542265, 21 },
        { 3, 379878, "NWonRaceOverP", 883660955, 19 },
        { 3, 379895, "DeletionP", 883660955, 19 },
        { 4, 379939, "CacheMissForP", 644542265, 22 },
        { 4, 379942, "BeginBuildP", 644542265, 22 },
        { 3, 380675, "NWonRaceOverP", 883660955, 20 },
        { 3, 380682, "DeletionP", 883660955, 20 },
        { 4, 380698, "CacheMissForP", 644542265, 23 },
        { 4, 380701, "BeginBuildP", 644542265, 23 },
        { 2, 385057, "PurgedN", 1147256140, 16 },
        { 8, 385067, "AddedN", 1301841557, 18 },
        { 9, 385580, "CacheMissForN", 2709710687, 24 },
        { 9, 385585, "BeginBuildN", 2709710687, 24 },
        { 4, 402499, "WaitEndN", 644542265, 7 },
        { 5, 402914, "WaitBeginN", 2491771, 8 },
        { 3, 410390, "PurgedN", 883660955, 17 },
        { 4, 410403, "AddedN", 644542265, 21 },
        { 5, 410847, "CacheMissForN", 2491771, 25 },
        { 5, 410853, "BeginBuildN", 2491771, 25 },
        { 4, 417817, "NWonRaceOverP", 644542265, 23 },
        { 4, 417830, "DeletionP", 644542265, 23 },
        { 5, 417861, "CacheMissForP", 2491771, 26 },
        { 5, 417864, "BeginBuildP", 2491771, 26 },
        { 4, 418437, "NWonRaceOverP", 644542265, 22 },
        { 4, 418445, "DeletionP", 644542265, 22 },
        { 5, 418459, "CacheMissForP", 2491771, 27 },
        { 5, 418461, "BeginBuildP", 2491771, 27 },
        { 8, 425723, "PurgedN", 1301841557, 18 },
        { 9, 425735, "AddedN", 2709710687, 24 },
        { 10, 426120, "CacheMissForN", 138923328, 28 },
        { 10, 426125, "BeginBuildN", 138923328, 28 },
        { 5, 434255, "WaitEndN", 2491771, 8 },
        { 6, 434857, "WaitBeginN", 3234710487, 12 },
        { 4, 446582, "PurgedN", 644542265, 21 },
        { 5, 446594, "AddedN", 2491771, 25 },
        { 6, 446977, "CacheMissForN", 3234710487, 29 },
        { 6, 446986, "BeginBuildN", 3234710487, 29 },
        { 5, 454259, "NWonRaceOverP", 2491771, 26 },
        { 5, 454271, "DeletionP", 2491771, 26 },
        { 5, 454272, "NWonRaceOverP", 2491771, 27 },
        { 5, 454277, "DeletionP", 2491771, 27 },
        { 6, 454345, "CacheMissForP", 3234710487, 30 },
        { 6, 454348, "CacheMissForP", 3234710487, 31 },
        { 6, 454351, "BeginBuildP", 3234710487, 30 },
        { 6, 454351, "BeginBuildP", 3234710487, 31 },
        { 9, 464080, "PurgedN", 2709710687, 24 },
        { 10, 464091, "AddedN", 138923328, 28 },
        { 11, 464448, "CacheMissForN", 2121723309, 32 },
        { 11, 464453, "BeginBuildN", 2121723309, 32 },
        { 6, 471074, "WaitEndN", 3234710487, 12 },
        { 2, 471924, "WaitBeginN", 1147256140, 16 },
        { 2, 471982, "WaitEndN", 1147256140, 16 },
        { 7, 472312, "WaitBeginN", 1997496709, 15 },
        { 5, 482225, "PurgedN", 2491771, 25 },
        { 6, 482243, "AddedN", 3234710487, 29 },
        { 7, 482612, "CacheMissForN", 1997496709, 33 },
        { 7, 482617, "BeginBuildN", 1997496709, 33 },
        { 6, 489510, "NWonRaceOverP", 3234710487, 30 },
        { 6, 489521, "DeletionP", 3234710487, 30 },
        { 7, 489546, "CacheMissForP", 1997496709, 34 },
        { 7, 489549, "BeginBuildP", 1997496709, 34 },
        { 6, 489632, "NWonRaceOverP", 3234710487, 31 },
        { 6, 489643, "DeletionP", 3234710487, 31 },
        { 7, 489676, "CacheMissForP", 1997496709, 35 },
        { 7, 489678, "BeginBuildP", 1997496709, 35 },
        { 10, 501768, "PurgedN", 138923328, 28 },
        { 11, 501777, "AddedN", 2121723309, 32 },
        { 6, 519787, "PurgedN", 3234710487, 29 },
        { 7, 519798, "AddedN", 1997496709, 33 },
        { 8, 520243, "CacheMissForN", 1301841557, 36 },
        { 8, 520247, "BeginBuildN", 1301841557, 36 },
        { 7, 524005, "WaitEndN", 1997496709, 15 },
        { 3, 524597, "WaitBeginN", 883660955, 17 },
        { 3, 524651, "WaitEndN", 883660955, 17 },
        { 8, 524951, "WaitBeginN", 1301841557, 18 },
        { 7, 526536, "NWonRaceOverP", 1997496709, 34 },
        { 7, 526545, "DeletionP", 1997496709, 34 },
        { 8, 526574, "CacheMissForP", 1301841557, 37 },
        { 8, 526576, "BeginBuildP", 1301841557, 37 },
        { 7, 528286, "NWonRaceOverP", 1997496709, 35 },
        { 7, 528299, "DeletionP", 1997496709, 35 },
        { 8, 528524, "CacheMissForP", 1301841557, 38 },
        { 8, 528528, "BeginBuildP", 1301841557, 38 },
        { 11, 558015, "PurgedN", 2121723309, 32 },
        { 8, 558026, "AddedN", 1301841557, 36 },
        { 9, 558511, "CacheMissForN", 2709710687, 39 },
        { 9, 558517, "BeginBuildN", 2709710687, 39 },
        { 8, 561632, "WaitEndN", 1301841557, 18 },
        { 4, 562157, "WaitBeginN", 644542265, 21 },
        { 4, 562188, "WaitEndN", 644542265, 21 },
        { 9, 562321, "WaitBeginN", 2709710687, 24 },
        { 8, 563303, "NWonRaceOverP", 1301841557, 37 },
        { 8, 563317, "DeletionP", 1301841557, 37 },
        { 9, 563418, "CacheMissForP", 2709710687, 40 },
        { 9, 563422, "BeginBuildP", 2709710687, 40 },
        { 8, 570819, "NWonRaceOverP", 1301841557, 38 },
        { 8, 570831, "DeletionP", 1301841557, 38 },
        { 9, 570913, "CacheMissForP", 2709710687, 41 },
        { 9, 570916, "BeginBuildP", 2709710687, 41 },
        { 9, 595832, "WaitEndN", 2709710687, 24 },
        { 5, 596119, "WaitBeginN", 2491771, 25 },
        { 5, 596148, "WaitEndN", 2491771, 25 },
        { 10, 596272, "WaitBeginN", 138923328, 28 },
        { 7, 599152, "PurgedN", 1997496709, 33 },
        { 9, 599163, "AddedN", 2709710687, 39 },
        { 10, 599591, "CacheMissForN", 138923328, 42 },
        { 10, 599595, "BeginBuildN", 138923328, 42 },
        { 9, 599845, "NWonRaceOverP", 2709710687, 40 },
        { 9, 599856, "DeletionP", 2709710687, 40 },
        { 10, 599894, "CacheMissForP", 138923328, 43 },
        { 10, 599896, "BeginBuildP", 138923328, 43 },
        { 9, 609154, "NWonRaceOverP", 2709710687, 41 },
        { 9, 609169, "DeletionP", 2709710687, 41 },
        { 10, 609206, "CacheMissForP", 138923328, 44 },
        { 10, 609209, "BeginBuildP", 138923328, 44 },
        { 8, 638657, "PurgedN", 1301841557, 36 },
        { 10, 638669, "AddedN", 138923328, 42 },
        { 11, 639128, "CacheMissForN", 2121723309, 45 },
        { 11, 639133, "BeginBuildN", 2121723309, 45 },
        { 10, 654111, "WaitEndN", 138923328, 28 },
        { 6, 654434, "WaitBeginN", 3234710487, 29 },
        { 6, 654457, "WaitEndN", 3234710487, 29 },
        { 11, 654575, "WaitBeginN", 2121723309, 32 },
        { 11, 675082, "WaitEndN", 2121723309, 32 },
        { 7, 675317, "WaitBeginN", 1997496709, 33 },
        { 9, 677138, "PurgedN", 2709710687, 39 },
        { 11, 677146, "AddedN", 2121723309, 45 },
        { 7, 700828, "WaitEndN", 1997496709, 33 },
        { 8, 701033, "WaitBeginN", 1301841557, 36 },
        { 8, 728133, "WaitEndN", 1301841557, 36 },
        { 9, 728367, "WaitBeginN", 2709710687, 39 },
        { 9, 728393, "WaitEndN", 2709710687, 39 },
        { 10, 728539, "WaitBeginN", 138923328, 42 },
        { 10, 813506, "WaitEndN", 138923328, 42 },
        { 11, 813832, "WaitBeginN", 2121723309, 45 },
        { 11, 813865, "WaitEndN", 2121723309, 45 },
        { 10, 814672, "NWonRaceOverP", 138923328, 43 },
        { 10, 814682, "DeletionP", 138923328, 43 },
        { 11, 814724, "CacheHitForP", 2121723309, 45 },
        { 10, 824529, "NWonRaceOverP", 138923328, 44 },
        { 10, 824543, "DeletionP", 138923328, 44 },
        { 11, 824581, "CacheHitForP", 2121723309, 45 },
        { 1, 833239, "DeletionN", 2189112322, 4 },
        { 2, 833263, "DeletionN", 1147256140, 5 },
        { 3, 833283, "DeletionN", 883660955, 6 },
        { 4, 833304, "DeletionN", 644542265, 7 },
        { 0, 833365, "DeletionN", 477900207, 3 },
        { 5, 833387, "DeletionN", 2491771, 8 },
        { 6, 833406, "DeletionN", 3234710487, 12 },
        { 1, 833442, "DeletionP", 2189112322, 11 },
        { 2, 833476, "DeletionN", 1147256140, 16 },
        { 7, 833496, "DeletionN", 1997496709, 15 },
        { 3, 833529, "DeletionN", 883660955, 17 },
        { 8, 833548, "DeletionN", 1301841557, 18 },
        { 4, 833579, "DeletionN", 644542265, 21 },
        { 9, 833596, "DeletionN", 2709710687, 24 },
        { 5, 833628, "DeletionN", 2491771, 25 },
        { 10, 833646, "DeletionN", 138923328, 28 },
        { 6, 833684, "DeletionN", 3234710487, 29 },
        { 11, 833725, "DeletionN", 2121723309, 32 },
        { 7, 833777, "DeletionN", 1997496709, 33 },
        { 8, 833807, "DeletionN", 1301841557, 36 },
        { 9, 833835, "DeletionN", 2709710687, 39 },
        { 11, 833916, "DeletionN", 2121723309, 45 },
        { 10, 833952, "DeletionN", 138923328, 42 },
};

static char TagType(const char* tag) {
    int len = strlen(tag);
    SkASSERT(len >= 1);
    char tmp = tag[len-1];
    SkASSERT(tmp == 'N' || tmp == 'P');
    return tmp;
}

struct FSMNode {
    // The ThreadedCompilePrecompileTest uses two Precompile threads and two Normal threads so,
    // at worst, we can get 4 Pipelines for the same uniqueID.
    static const int kMaxPipelinesInAGivenState = 4;

    bool isInState(uint32_t hash, uint32_t dude) const {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState1[i].fCompileID == dude) {
                SkASSERT(fInState1[i].fHash == hash);
                SkASSERT(fInState1[i].isFilledIn());
                return true;
            }
        }

        return false;
    }

    bool isEmpty() const {
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (!fInState1[i].isntSet()) {
                return false;
            }
        }

        return true;
    }

    struct Flooble {
        Flooble(uint32_t hash, uint32_t compileID, uint32_t entryTime, char type)
            : fHash(hash)
            , fCompileID(compileID)
            , fEntryTime(entryTime)
            , fType(type) {
            SkASSERT(this->isFilledIn());
        }

        Flooble() { }

        bool isntSet() const {
            return fHash == UINT32_MAX && fCompileID == UINT32_MAX && fEntryTime == 0 && fType == 'U';
        }
        bool isFilledIn() const {
            return fHash != UINT32_MAX && fCompileID != UINT32_MAX && fEntryTime != 0 &&
                   (fType == 'P' || fType == 'N');
        }

        bool matches(uint32_t hash, uint32_t compileID, char type) const {
            return fHash == hash && fCompileID == compileID && fType == type;
        }

        void reset() {
            fHash = UINT32_MAX;
            fCompileID = UINT32_MAX;
            fEntryTime = 0;
            fType = 'U';
        }

        uint32_t fHash = UINT32_MAX;
        uint32_t fCompileID = UINT32_MAX;
        uint32_t fEntryTime = 0;
        char fType = 'U';
        int64_t fWaitStart = 0;
    };

    Flooble fInState1[kMaxPipelinesInAGivenState];

    void addToState(const Flooble& flooble) {
        SkASSERT(flooble.isFilledIn());

        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            SkASSERT(fInState1[i].fCompileID != flooble.fCompileID);  // had better not have already been in this state
        }

        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState1[i].fCompileID == UINT32_MAX) {
                SkASSERT(fInState1[i].isntSet());

                fInState1[i] = flooble;
                return;
            }
        }

        SkASSERT(0);
    }

    FSMNode::Flooble removeFromState1(uint32_t hash, uint32_t dude, char type) {
        SkASSERT(hash != UINT32_MAX && dude != UINT32_MAX && (type == 'P' || type == 'N'));
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState1[i].fCompileID == dude) {
                SkASSERT(fInState1[i].isFilledIn());
                SkASSERT(fInState1[i].matches(hash, dude, type));

                Flooble tmp = fInState1[i];

                fInState1[i].reset();
                return tmp;
            }
        }

        SkASSERT(0);
        return {};
    }

    Flooble& getSoleSet() {
        SkASSERT(fInState1[0].isFilledIn());
        for (int i = 1; i < kMaxPipelinesInAGivenState; ++i) {
            SkASSERT(fInState1[i].isntSet());
        }

        return fInState1[0];
    }

    Flooble& gimme(uint32_t hash, uint32_t dude) {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState1[i].fCompileID == dude) {
                SkASSERT(fInState1[i].fHash == hash);
                SkASSERT(fInState1[i].isFilledIn());
                return fInState1[i];
            }
        }

        SkASSERT(0);
        return fInState1[0];
    }

};

enum Node {
    // Entry: "CacheMissForP" or "CacheMissForN"
    // Exit:  "BeginBuildP" or "BeginBuildN" --> kBeginBuild
    //
    // The usual first node in the FSM. This is where a given pipeline is first detected as a
    // cache miss. It can be either 'N' or 'P'.
    // There can be multiple misses for the same UniqueID and multiple Pipeline IDs generated.
    // This node always transitions to kBeginBuild
    kMiss = 0,
    // Entry: "BeginBuildP" or "BeginBuildN"
    // Exit:  "AddedN" or "AddedP" --> kEndBuild --> kAddedToCache
    //        "NWonRaceOverN", "NWonRaceOverP", "PWonRaceOverN" or "PWonRaceOverP --> kEndBuild --> kToBeDeleted
    //
    // This node marks the beginning of compilation. It can have multiple 'P' and 'N' pipelines.
    // It can
    kBeginBuild = 1,
    // A transitory state - no Pipeline should actually stay here
    kEndBuild = 2,
    // Entry: "AddedN" or "AddedP"
    // Exit:
    kAddedToCache = 3,

    // A purgatory for the pipelines waiting for a deletion message
    kToBeDeleted = 4,
    kDeleted = 5,

    kLast = kDeleted,
};

#if 0
Original
 0:  477900207 [ 3251 CacheMissForP 0][ 3254 CacheMissForP 1][ 3263 BeginBuildP 0][ 3263 BeginBuildP 1][41172 CacheMissForN 2][41176 CacheMissForN 3][41176 BeginBuildN 2][41176 BeginBuildN 3][87615 AddedN 3][87631 NWonRaceOverN 2][87640 WaitBeginN 2][89031 WaitBeginN 3][159129 PurgedN 3][258594 WaitEndN 2][258607 DeletionN 2][258658 AddedP 1][259131 PWonRaceOverP 0][259156 DeletionP 0][266915 WaitEndN 3][298123 BadPurgedP 1][298137 DeletionP 1][833364 DeletionN 3]
 1: 2189112322 [88664 CacheMissForN 4][88665 BeginBuildN 4][123147 AddedN 4][193874 PurgedN 4][258694 CacheMissForP 9][258695 BeginBuildP 9][259042 CacheMissForN 10][259042 BeginBuildN 10][259232 CacheMissForP 11][259233 BeginBuildP 11][268067 WaitBeginN 4][296934 WaitEndN 4][298139 AddedP 11][299425 PWonRaceOverP 9][299433 DeletionP 9][302628 PWonRaceOverN 10][302632 WaitBeginN 10][302652 WaitEndN 10][302655 DeletionN 10][339381 BadPurgedP 11][833239 DeletionN 4][833442 DeletionP 11]
 2: 1147256140 [123539 CacheMissForN 5][123540 BeginBuildN 5][159144 AddedN 5][230389 PurgedN 5][297574 WaitBeginN 5][298180 CacheMissForP 13][298181 BeginBuildP 13][299451 CacheMissForP 14][299451 BeginBuildP 14][302885 CacheMissForN 16][302885 BeginBuildN 16][331177 WaitEndN 5][338643 AddedN 16][342870 NWonRaceOverP 13][342883 DeletionP 13][343835 NWonRaceOverP 14][343842 DeletionP 14][385057 PurgedN 16][471924 WaitBeginN 16][471981 WaitEndN 16][833263 DeletionN 5][833476 DeletionN 16]
 3:  883660955 [159644 CacheMissForN 6][159644 BeginBuildN 6][193887 AddedN 6][258653 PurgedN 6][332029 WaitBeginN 6][339085 CacheMissForN 17][339085 BeginBuildN 17][342965 CacheMissForP 19][342966 BeginBuildP 19][343891 CacheMissForP 20][343891 BeginBuildP 20][354418 WaitEndN 6][373628 AddedN 17][379878 NWonRaceOverP 19][379895 DeletionP 19][380675 NWonRaceOverP 20][380682 DeletionP 20][410390 PurgedN 17][524596 WaitBeginN 17][524651 WaitEndN 17][833283 DeletionN 6][833529 DeletionN 17]
 4:  644542265 [194266 CacheMissForN 7][194266 BeginBuildN 7][230399 AddedN 7][266446 PurgedN 7][354690 WaitBeginN 7][374064 CacheMissForN 21][374065 BeginBuildN 21][379942 CacheMissForP 22][379942 BeginBuildP 22][380700 CacheMissForP 23][380701 BeginBuildP 23][402499 WaitEndN 7][410403 AddedN 21][417817 NWonRaceOverP 23][417830 DeletionP 23][418437 NWonRaceOverP 22][418445 DeletionP 22][446582 PurgedN 21][562157 WaitBeginN 21][562188 WaitEndN 21][833304 DeletionN 7][833579 DeletionN 21]
 5:    2491771 [230767 CacheMissForN 8][230768 BeginBuildN 8][266456 AddedN 8][302022 PurgedN 8][402914 WaitBeginN 8][410853 CacheMissForN 25][410853 BeginBuildN 25][417863 CacheMissForP 26][417864 BeginBuildP 26][418461 CacheMissForP 27][418461 BeginBuildP 27][434255 WaitEndN 8][446594 AddedN 25][454259 NWonRaceOverP 26][454270 DeletionP 26][454272 NWonRaceOverP 27][454277 DeletionP 27][482225 PurgedN 25][596119 WaitBeginN 25][596148 WaitEndN 25][833387 DeletionN 8][833627 DeletionN 25]
 6: 3234710487 [266825 CacheMissForN 12][266825 BeginBuildN 12][302032 AddedN 12][338628 PurgedN 12][434857 WaitBeginN 12][446986 CacheMissForN 29][446986 BeginBuildN 29][454347 CacheMissForP 30][454350 CacheMissForP 31][454351 BeginBuildP 30][454351 BeginBuildP 31][471073 WaitEndN 12][482243 AddedN 29][489510 NWonRaceOverP 30][489521 DeletionP 30][489632 NWonRaceOverP 31][489643 DeletionP 31][519787 PurgedN 29][654434 WaitBeginN 29][654457 WaitEndN 29][833406 DeletionN 12][833684 DeletionN 29]
 7: 1997496709 [302462 CacheMissForN 15][302463 BeginBuildN 15][339387 AddedN 15][373614 PurgedN 15][472312 WaitBeginN 15][482617 CacheMissForN 33][482617 BeginBuildN 33][489548 CacheMissForP 34][489549 BeginBuildP 34][489678 CacheMissForP 35][489678 BeginBuildP 35][519798 AddedN 33][524005 WaitEndN 15][526536 NWonRaceOverP 34][526545 DeletionP 34][528286 NWonRaceOverP 35][528299 DeletionP 35][599152 PurgedN 33][675317 WaitBeginN 33][700828 WaitEndN 33][833496 DeletionN 15][833777 DeletionN 33]
 8: 1301841557 [339632 CacheMissForN 18][339633 BeginBuildN 18][385067 AddedN 18][425723 PurgedN 18][520246 CacheMissForN 36][520247 BeginBuildN 36][524950 WaitBeginN 18][526576 CacheMissForP 37][526576 BeginBuildP 37][528528 CacheMissForP 38][528528 BeginBuildP 38][558026 AddedN 36][561632 WaitEndN 18][563303 NWonRaceOverP 37][563316 DeletionP 37][570819 NWonRaceOverP 38][570831 DeletionP 38][638657 PurgedN 36][701033 WaitBeginN 36][728133 WaitEndN 36][833548 DeletionN 18][833807 DeletionN 36]
 9: 2709710687 [385584 CacheMissForN 24][385585 BeginBuildN 24][425735 AddedN 24][464080 PurgedN 24][558516 CacheMissForN 39][558517 BeginBuildN 39][562321 WaitBeginN 24][563421 CacheMissForP 40][563422 BeginBuildP 40][570916 CacheMissForP 41][570916 BeginBuildP 41][595832 WaitEndN 24][599163 AddedN 39][599845 NWonRaceOverP 40][599856 DeletionP 40][609154 NWonRaceOverP 41][609169 DeletionP 41][677138 PurgedN 39][728367 WaitBeginN 39][728393 WaitEndN 39][833595 DeletionN 24][833835 DeletionN 39]
10:  138923328 [426125 CacheMissForN 28][426125 BeginBuildN 28][464091 AddedN 28][501768 PurgedN 28][596272 WaitBeginN 28][599595 CacheMissForN 42][599595 BeginBuildN 42][599896 CacheMissForP 43][599896 BeginBuildP 43][609208 CacheMissForP 44][609209 BeginBuildP 44][638669 AddedN 42][654111 WaitEndN 28][728539 WaitBeginN 42][813506 WaitEndN 42][814671 NWonRaceOverP 43][814682 DeletionP 43][824529 NWonRaceOverP 44][824543 DeletionP 44][833646 DeletionN 28][833952 DeletionN 42]
11: 2121723309 [464453 CacheMissForN 32][464453 BeginBuildN 32][501777 AddedN 32][558015 PurgedN 32][639133 CacheMissForN 45][639133 BeginBuildN 45][654575 WaitBeginN 32][675082 WaitEndN 32][677146 AddedN 45][813832 WaitBeginN 45][813865 WaitEndN 45][814724 CacheHitForP 45][824581 CacheHitForP 45][833725 DeletionN 32][833916 DeletionN 45]
#endif

#if 0
Reconstructed
 0:  477900207 [ 3236 CacheMissForP 0][ 3251 CacheMissForP 1][ 3263 BeginBuildP 0][ 3263 BeginBuildP 1][41152 CacheMissForN 2][41173 CacheMissForN 3][41176 BeginBuildN 2][41176 BeginBuildN 3][87615 AddedN 3][87631 NWonRaceOverN 2][87640 WaitBeginN 2][89031 WaitBeginN 3][159129 PurgedN 3][258594 WaitEndN 2][258607 DeletionN 2][258659 AddedP 1][259131 PWonRaceOverP 0][259156 DeletionP 0][266916 WaitEndN 3][298131 BadPurgedP 1][298137 DeletionP 1][833365 DeletionN 3]
 1: 2189112322 [88650 CacheMissForN 4][88665 BeginBuildN 4][123147 AddedN 4][193874 PurgedN 4][258693 CacheMissForP 9][258695 BeginBuildP 9][259037 CacheMissForN 10][259042 BeginBuildN 10][259226 CacheMissForP 11][259234 BeginBuildP 11][268067 WaitBeginN 4][296934 WaitEndN 4][298139 AddedP 11][299425 PWonRaceOverP 9][299433 DeletionP 9][302628 PWonRaceOverN 10][302632 WaitBeginN 10][302652 WaitEndN 10][302655 DeletionN 10][339385 BadPurgedP 11][833239 DeletionN 4][833442 DeletionP 11]
 2: 1147256140 [123536 CacheMissForN 5][123540 BeginBuildN 5][159144 AddedN 5][230389 PurgedN 5][297574 WaitBeginN 5][298178 CacheMissForP 13][298181 BeginBuildP 13][299449 CacheMissForP 14][299451 BeginBuildP 14][302879 CacheMissForN 16][302885 BeginBuildN 16][331177 WaitEndN 5][338643 AddedN 16][342870 NWonRaceOverP 13][342883 DeletionP 13][343835 NWonRaceOverP 14][343842 DeletionP 14][385057 PurgedN 16][471924 WaitBeginN 16][471982 WaitEndN 16][833263 DeletionN 5][833476 DeletionN 16]
 3:  883660955 [159636 CacheMissForN 6][159644 BeginBuildN 6][193887 AddedN 6][258653 PurgedN 6][332029 WaitBeginN 6][339080 CacheMissForN 17][339085 BeginBuildN 17][342963 CacheMissForP 19][342966 BeginBuildP 19][343889 CacheMissForP 20][343891 BeginBuildP 20][354418 WaitEndN 6][373628 AddedN 17][379878 NWonRaceOverP 19][379895 DeletionP 19][380675 NWonRaceOverP 20][380682 DeletionP 20][410390 PurgedN 17][524597 WaitBeginN 17][524651 WaitEndN 17][833283 DeletionN 6][833529 DeletionN 17]
 4:  644542265 [194261 CacheMissForN 7][194266 BeginBuildN 7][230399 AddedN 7][266446 PurgedN 7][354691 WaitBeginN 7][374060 CacheMissForN 21][374065 BeginBuildN 21][379939 CacheMissForP 22][379942 BeginBuildP 22][380698 CacheMissForP 23][380701 BeginBuildP 23][402499 WaitEndN 7][410403 AddedN 21][417817 NWonRaceOverP 23][417830 DeletionP 23][418437 NWonRaceOverP 22][418445 DeletionP 22][446582 PurgedN 21][562157 WaitBeginN 21][562188 WaitEndN 21][833304 DeletionN 7][833579 DeletionN 21]
 5:    2491771 [230763 CacheMissForN 8][230768 BeginBuildN 8][266456 AddedN 8][302022 PurgedN 8][402914 WaitBeginN 8][410847 CacheMissForN 25][410853 BeginBuildN 25][417861 CacheMissForP 26][417864 BeginBuildP 26][418459 CacheMissForP 27][418461 BeginBuildP 27][434255 WaitEndN 8][446594 AddedN 25][454259 NWonRaceOverP 26][454271 DeletionP 26][454272 NWonRaceOverP 27][454277 DeletionP 27][482225 PurgedN 25][596119 WaitBeginN 25][596148 WaitEndN 25][833387 DeletionN 8][833628 DeletionN 25]
 6: 3234710487 [266820 CacheMissForN 12][266825 BeginBuildN 12][302032 AddedN 12][338628 PurgedN 12][434857 WaitBeginN 12][446977 CacheMissForN 29][446986 BeginBuildN 29][454345 CacheMissForP 30][454348 CacheMissForP 31][454351 BeginBuildP 30][454351 BeginBuildP 31][471074 WaitEndN 12][482243 AddedN 29][489510 NWonRaceOverP 30][489521 DeletionP 30][489632 NWonRaceOverP 31][489643 DeletionP 31][519787 PurgedN 29][654434 WaitBeginN 29][654457 WaitEndN 29][833406 DeletionN 12][833684 DeletionN 29]
 7: 1997496709 [302459 CacheMissForN 15][302463 BeginBuildN 15][339387 AddedN 15][373614 PurgedN 15][472312 WaitBeginN 15][482612 CacheMissForN 33][482617 BeginBuildN 33][489546 CacheMissForP 34][489549 BeginBuildP 34][489676 CacheMissForP 35][489678 BeginBuildP 35][519798 AddedN 33][524005 WaitEndN 15][526536 NWonRaceOverP 34][526545 DeletionP 34][528286 NWonRaceOverP 35][528299 DeletionP 35][599152 PurgedN 33][675317 WaitBeginN 33][700828 WaitEndN 33][833496 DeletionN 15][833777 DeletionN 33]
 8: 1301841557 [339630 CacheMissForN 18][339633 BeginBuildN 18][385067 AddedN 18][425723 PurgedN 18][520243 CacheMissForN 36][520247 BeginBuildN 36][524951 WaitBeginN 18][526574 CacheMissForP 37][526576 BeginBuildP 37][528524 CacheMissForP 38][528528 BeginBuildP 38][558026 AddedN 36][561632 WaitEndN 18][563303 NWonRaceOverP 37][563317 DeletionP 37][570819 NWonRaceOverP 38][570831 DeletionP 38][638657 PurgedN 36][701033 WaitBeginN 36][728133 WaitEndN 36][833548 DeletionN 18][833807 DeletionN 36]
 9: 2709710687 [385580 CacheMissForN 24][385585 BeginBuildN 24][425735 AddedN 24][464080 PurgedN 24][558511 CacheMissForN 39][558517 BeginBuildN 39][562321 WaitBeginN 24][563418 CacheMissForP 40][563422 BeginBuildP 40][570913 CacheMissForP 41][570916 BeginBuildP 41][595832 WaitEndN 24][599163 AddedN 39][599845 NWonRaceOverP 40][599856 DeletionP 40][609154 NWonRaceOverP 41][609169 DeletionP 41][677138 PurgedN 39][728367 WaitBeginN 39][728393 WaitEndN 39][833596 DeletionN 24][833835 DeletionN 39]
10:  138923328 [426120 CacheMissForN 28][426125 BeginBuildN 28][464091 AddedN 28][501768 PurgedN 28][596272 WaitBeginN 28][599591 CacheMissForN 42][599595 BeginBuildN 42][599894 CacheMissForP 43][599896 BeginBuildP 43][609206 CacheMissForP 44][609209 BeginBuildP 44][638669 AddedN 42][654111 WaitEndN 28][728539 WaitBeginN 42][813506 WaitEndN 42][814672 NWonRaceOverP 43][814682 DeletionP 43][824529 NWonRaceOverP 44][824543 DeletionP 44][833646 DeletionN 28][833952 DeletionN 42]
11: 2121723309 [464448 CacheMissForN 32][464453 BeginBuildN 32][501777 AddedN 32][558015 PurgedN 32][639128 CacheMissForN 45][639133 BeginBuildN 45][654575 WaitBeginN 32][675082 WaitEndN 32][677146 AddedN 45][813832 WaitBeginN 45][813865 WaitEndN 45][814724 CacheHitForP 45][824581 CacheHitForP 45][833725 DeletionN 32][833916 DeletionN 45]
#endif

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override { return SkString("arcofzorro"); }

    SkISize getISize() override { return SkISize::Make(1000, 1000); }

    void onDraw(SkCanvas* canvas) override {

        int64_t compileTimeTotals[2] = { 0, 0 };
        int compileCounts[2] = { 0, 0 };

        // The ThreadedCompilePrecompileTest exercises 4 gradients - each of which has 3
        // variations
        static const int kNumActualUniqueIDs = 12;

        // For each uniqueID we reassemble the log from the tracing output and print it out.
        // We then, for that uniqueID, replay the log through the FSM to validate the
        // Pipelines' behavior. Note that we can have multiple Pipelines created for each
        // uniqueID.
        for (int i = 0; i < kNumActualUniqueIDs; ++i) {
            std::vector<LoggingOutput> log;

            bool first = true;
            SkDebugf("%2d: ", i);
            for (auto& b : kOutputFromRealLogging) {
                if (b.fID == i) {
                    if (first) {
                        first = false;
                        SkDebugf("%10u ", b.fHash);
                    }
                    SkDebugf("[%5lld %s %u]", b.fTime, b.fTag, b.fPipelineID1);

                    log.push_back(b);
                }
            }
            SkDebugf("\n");

            FSMNode fsm[Node::kLast+1];


            for (auto l : log) {
                char tagType = TagType(l.fTag);

                if (!strcmp(l.fTag, "CacheMissForP") ||
                    !strcmp(l.fTag, "CacheMissForN")) {

                    if (fsm[Node::kDeleted].isInState(l.fHash, l.fPipelineID1)) {
                        // This Pipeline was purged and is being re-created
                        SkASSERT(0);
                        // We should never get here bc the same compileID should be reused!!
                        //fsm[Node::kDeleted].removeFromState(l.fHash, l.fPipelineID1, tagType); // tagType could be different $$
                                     // it seems like this should just be based on the hash - not the compileID
                    }

                    fsm[Node::kMiss].addToState(FSMNode::Flooble(l.fHash, l.fPipelineID1, l.fTime, tagType));
                } else if (!strcmp(l.fTag, "BeginBuildP") ||
                           !strcmp(l.fTag, "BeginBuildN")) {
                    FSMNode::Flooble flooble = fsm[Node::kMiss].removeFromState1(l.fHash, l.fPipelineID1, tagType); // should match
                    flooble.fEntryTime = l.fTime;
                    fsm[Node::kBeginBuild].addToState(flooble);
                } else if (!strcmp(l.fTag, "AddedN") ||
                           !strcmp(l.fTag, "AddedP")) {

                    FSMNode::Flooble flooble = fsm[Node::kBeginBuild].removeFromState1(l.fHash, l.fPipelineID1, tagType);
                    int64_t beginTime = flooble.fEntryTime;

                    flooble.fEntryTime = l.fTime;
                    fsm[Node::kEndBuild].addToState(flooble);
                    // compute compile time here
                    int64_t diff = l.fTime - beginTime;
                    compileTimeTotals[tagType == 'P'] += diff;
                    compileCounts[tagType == 'P']++;

                    flooble = fsm[Node::kEndBuild].removeFromState1(l.fHash, l.fPipelineID1, tagType);
                    flooble.fEntryTime = l.fTime;
                    SkASSERT(fsm[Node::kAddedToCache].isEmpty());
                    fsm[Node::kAddedToCache].addToState(flooble);
                } else if (!strcmp(l.fTag, "NWonRaceOverN") ||
                           !strcmp(l.fTag, "NWonRaceOverP") ||
                           !strcmp(l.fTag, "PWonRaceOverN") ||
                           !strcmp(l.fTag, "PWonRaceOverP")) {
                    char winnerType = l.fTag[0];

                    const FSMNode::Flooble& winner = fsm[Node::kAddedToCache].getSoleSet();
                    SkASSERT(winner.fHash == l.fHash);
                    SkASSERT(winner.fType == winnerType);

//                    SkASSERT(fsm[Node::kAddedToCache].isInState(l.fHash, l.fPipelineID1)); // fPipelineID1 won !!!!!!!!!
                    FSMNode::Flooble flooble = fsm[Node::kBeginBuild].removeFromState1(l.fHash, l.fPipelineID1, tagType);
                    uint64_t beginTime = flooble.fEntryTime;

                    flooble.fEntryTime = l.fTime;
                    fsm[Node::kEndBuild].addToState(flooble);

                    int64_t diff = l.fTime - beginTime;
                    compileTimeTotals[tagType == 'P'] += diff;
                    compileCounts[tagType == 'P']++;

                    // compute compile time here
                    // State A
                    // fPipelineID2 lost and gets destroyed a total waste of time!!
                    flooble = fsm[Node::kEndBuild].removeFromState1(l.fHash, l.fPipelineID1, tagType);
                    flooble.fEntryTime = l.fTime;
                    fsm[Node::kToBeDeleted].addToState(flooble);
                } else if (!strcmp(l.fTag, "CacheHitForP") ||
                           !strcmp(l.fTag, "CacheHitForN")) {
                    // saved work - esp for 'P'
                } else if (!strcmp(l.fTag, "WaitBeginN")) {
                    // There is a mystery going on here! How are two Dawn pipelines both
                    // getting waited on!!
                    // What! This assert should be true!!!
//                    SkASSERT(fsm[Node::kAddedToCache].isInState(l.fPipelineID1));

                    SkASSERT(l.fTime != 0);
                    if (fsm[Node::kAddedToCache].isInState(l.fHash, l.fPipelineID1)) {
                        FSMNode::Flooble& winner = fsm[Node::kAddedToCache].getSoleSet();
                        SkASSERT(winner.matches(l.fHash, l.fPipelineID1, tagType));

                        winner.fWaitStart = l.fTime;
                    } else {
                        SkASSERT(fsm[Node::kToBeDeleted].isInState(l.fHash, l.fPipelineID1));

                        FSMNode::Flooble& flooble = fsm[Node::kToBeDeleted].gimme(l.fHash, l.fPipelineID1);

                        flooble.fWaitStart = l.fTime;
                    }
                } else if (!strcmp(l.fTag, "WaitEndN")) {
                    // As above, this assert should be true!!!
//                    SkASSERT(fsm[Node::kAddedToCache].isInState(l.fPipelineID1));

                    int64_t waitStart = 0;
                    if (fsm[Node::kAddedToCache].isInState(l.fHash, l.fPipelineID1)) {
                        FSMNode::Flooble& winner = fsm[Node::kAddedToCache].getSoleSet();
                        SkASSERT(winner.matches(l.fHash, l.fPipelineID1, tagType));

                        waitStart = winner.fWaitStart;
                        winner.fWaitStart = 0;
                    } else {
                        SkASSERT(fsm[Node::kToBeDeleted].isInState(l.fHash, l.fPipelineID1));

                        FSMNode::Flooble& flooble = fsm[Node::kToBeDeleted].gimme(l.fHash, l.fPipelineID1);

                        waitStart = flooble.fWaitStart;
                        flooble.fWaitStart = 0;
                    }

                    SkASSERT(waitStart != 0);
                    int64_t diff = l.fTime - waitStart;
                    SkDebugf("Bad wait %lld\n", diff);
                } else if (!strcmp(l.fTag, "DeletionP") ||
                           !strcmp(l.fTag, "DeletionN")) {

                    if (fsm[Node::kAddedToCache].isInState(l.fHash, l.fPipelineID1)) {
                        const FSMNode::Flooble winner = fsm[Node::kAddedToCache].getSoleSet();
                        SkASSERT(winner.matches(l.fHash, l.fPipelineID1, tagType));
                        FSMNode::Flooble flooble = fsm[Node::kAddedToCache].removeFromState1(l.fHash, l.fPipelineID1, tagType);
                        flooble.fEntryTime = l.fTime;
                        fsm[Node::kDeleted].addToState(flooble);
                    } else {
                        SkASSERT(fsm[Node::kToBeDeleted].isInState(l.fHash, l.fPipelineID1));
                        FSMNode::Flooble flooble = fsm[Node::kToBeDeleted].removeFromState1(l.fHash, l.fPipelineID1, tagType);
                        flooble.fEntryTime = l.fTime;
                        fsm[Node::kDeleted].addToState(flooble);
                    }
                } else if (!strcmp(l.fTag, "PurgedP") || !strcmp(l.fTag, "BadPurgedP") ||
                           !strcmp(l.fTag, "PurgedN") || !strcmp(l.fTag, "BadPurgedN")) {
                    // TODO: do something for bad purges

                    const FSMNode::Flooble winner = fsm[Node::kAddedToCache].getSoleSet();
                    SkASSERT(winner.matches(l.fHash, l.fPipelineID1, tagType));
                    FSMNode::Flooble flooble = fsm[Node::kAddedToCache].removeFromState1(l.fHash, l.fPipelineID1, tagType);
                    flooble.fEntryTime = l.fTime;
                    fsm[Node::kToBeDeleted].addToState(flooble);
                } else {
                    SkDebugf("--------------- Unhandled %s\n", l.fTag);
                }
            }

            // At the end all the Pipelines should've been deleted
            SkASSERT(kLast == kDeleted);
            for (int j = 0; j < kLast; ++j) {
                SkASSERT(fsm[j].isEmpty());
            }
        }

        SkDebugf("Avg Normal compile time %d: %lld\n", compileCounts[0], compileTimeTotals[0]/compileCounts[0]);
        SkDebugf("Avg Precompile compile time %d: %lld\n", compileCounts[1], compileTimeTotals[1]/compileCounts[1]);
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
