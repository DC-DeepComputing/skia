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
// TODO:
//   Do we actually need the pipeline ID? It seems low cost.
//   Think more about the FSM - add more nodes

static const struct LoggingOutput {
    int fID;
    uint64_t fTime;
    const char* fTag;
    uint32_t fHash;
    int fExtra;
    uint32_t fPipelineID1;
    uint32_t fPipelineID2;
} kOutputFromRealLogging[] = {
        { 0, 1004, "CacheMissForP", 477900207, -1, 0, UINT32_MAX },
        { 0, 1020, "CacheMissForP", 477900207, -1, 1, UINT32_MAX },
        { 0, 1032, "BeginBuildP", 477900207, -1, 0, UINT32_MAX },
        { 0, 1032, "BeginBuildP", 477900207, -1, 1, UINT32_MAX },
        { 0, 36380, "CacheMissForN", 477900207, -1, 2, UINT32_MAX },
        { 0, 36392, "CacheMissForN", 477900207, -1, 3, UINT32_MAX },
        { 0, 36396, "BeginBuildN", 477900207, -1, 2, UINT32_MAX },
        { 0, 36397, "BeginBuildN", 477900207, -1, 3, UINT32_MAX },
        { 0, 76368, "AddedN", 477900207, -1, 2, UINT32_MAX },
        { 0, 76384, "NWonRaceOverN", 477900207, -1, 2, 3 },
        { 0, 76394, "WaitBeginN", 477900207, -1, 3, UINT32_MAX },
        { 1, 77436, "CacheMissForN", 2189112322, -1, 4, UINT32_MAX },
        { 1, 77443, "BeginBuildN", 2189112322, -1, 4, UINT32_MAX },
        { 0, 77595, "WaitBeginN", 477900207, -1, 2, UINT32_MAX },
        { 1, 115628, "AddedN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 116243, "CacheMissForN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 116249, "BeginBuildN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 154134, "AddedN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 154611, "CacheMissForN", 883660955, -1, 6, UINT32_MAX },
        { 3, 154617, "BeginBuildN", 883660955, -1, 6, UINT32_MAX },
        { 3, 191593, "AddedN", 883660955, -1, 6, UINT32_MAX },
        { 4, 192107, "CacheMissForN", 644542265, -1, 7, UINT32_MAX },
        { 4, 192114, "BeginBuildN", 644542265, -1, 7, UINT32_MAX },
        { 4, 237454, "AddedN", 644542265, -1, 7, UINT32_MAX },
        { 5, 237923, "CacheMissForN", 2491771, -1, 8, UINT32_MAX },
        { 5, 237927, "BeginBuildN", 2491771, -1, 8, UINT32_MAX },
        { 0, 255348, "NWonRaceOverP", 477900207, -1, 2, 1 },
        { 0, 255380, "DeletionP", 477900207, -1, 1, UINT32_MAX },
        { 1, 45064868, "CacheHitForP", 2189112322, -1, 4, UINT32_MAX },
        { 2, 45064919, "CacheHitForP", 1147256140, -1, 5, UINT32_MAX },
        { 3, 45065109, "CacheHitForP", 883660955, -1, 6, UINT32_MAX },
        { 4, 45065133, "CacheHitForP", 644542265, -1, 7, UINT32_MAX },
        { 5, 45065153, "CacheMissForP", 2491771, -1, 9, UINT32_MAX },
        { 5, 45065158, "BeginBuildP", 2491771, -1, 9, UINT32_MAX },
        { 0, 45069728, "WaitEndN", 477900207, -1, 2, UINT32_MAX },
        { 1, 45070601, "WaitBeginN", 2189112322, -1, 4, UINT32_MAX },
        { 0, 45077308, "WaitEndN", 477900207, -1, 3, UINT32_MAX },
        { 0, 45077333, "DeletionN", 477900207, -1, 3, UINT32_MAX },
        { 1, 45078344, "CacheHitForN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 45078928, "CacheHitForN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 45079377, "CacheHitForN", 883660955, -1, 6, UINT32_MAX },
        { 4, 45079694, "CacheHitForN", 644542265, -1, 7, UINT32_MAX },
        { 5, 45079980, "CacheMissForN", 2491771, -1, 10, UINT32_MAX },
        { 5, 45079984, "BeginBuildN", 2491771, -1, 10, UINT32_MAX },
        { 0, 45086983, "NWonRaceOverP", 477900207, -1, 2, 0 },
        { 0, 45087004, "DeletionP", 477900207, -1, 0, UINT32_MAX },
        { 1, 45087044, "CacheHitForP", 2189112322, -1, 4, UINT32_MAX },
        { 2, 45087060, "CacheHitForP", 1147256140, -1, 5, UINT32_MAX },
        { 3, 45087135, "CacheHitForP", 883660955, -1, 6, UINT32_MAX },
        { 4, 45087150, "CacheHitForP", 644542265, -1, 7, UINT32_MAX },
        { 5, 45087163, "CacheMissForP", 2491771, -1, 11, UINT32_MAX },
        { 5, 45087166, "BeginBuildP", 2491771, -1, 11, UINT32_MAX },
        { 5, 45096111, "AddedN", 2491771, -1, 8, UINT32_MAX },
        { 6, 45096609, "CacheMissForN", 3234710487, -1, 12, UINT32_MAX },
        { 6, 45096618, "BeginBuildN", 3234710487, -1, 12, UINT32_MAX },
        { 6, 45136671, "AddedN", 3234710487, -1, 12, UINT32_MAX },
        { 7, 45137099, "CacheMissForN", 1997496709, -1, 13, UINT32_MAX },
        { 7, 45137103, "BeginBuildN", 1997496709, -1, 13, UINT32_MAX },
        { 5, 45138143, "NWonRaceOverN", 2491771, -1, 8, 10 },
        { 5, 45138156, "WaitBeginN", 2491771, -1, 10, UINT32_MAX },
        { 1, 45144863, "WaitEndN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 45145500, "WaitBeginN", 1147256140, -1, 5, UINT32_MAX },
        { 7, 45174789, "AddedN", 1997496709, -1, 13, UINT32_MAX },
        { 8, 45175257, "CacheMissForN", 1301841557, -1, 14, UINT32_MAX },
        { 8, 45175260, "BeginBuildN", 1301841557, -1, 14, UINT32_MAX },
        { 8, 45212555, "AddedN", 1301841557, -1, 14, UINT32_MAX },
        { 9, 45213032, "CacheMissForN", 2709710687, -1, 15, UINT32_MAX },
        { 9, 45213038, "BeginBuildN", 2709710687, -1, 15, UINT32_MAX },
        { 9, 45253409, "AddedN", 2709710687, -1, 15, UINT32_MAX },
        { 10, 45254441, "CacheMissForN", 138923328, -1, 16, UINT32_MAX },
        { 10, 45254453, "BeginBuildN", 138923328, -1, 16, UINT32_MAX },
        { 10, 45313657, "AddedN", 138923328, -1, 16, UINT32_MAX },
        { 11, 45314133, "CacheMissForN", 2121723309, -1, 17, UINT32_MAX },
        { 11, 45314137, "BeginBuildN", 2121723309, -1, 17, UINT32_MAX },
        { 2, 45314804, "WaitEndN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 45315050, "WaitBeginN", 883660955, -1, 6, UINT32_MAX },
        { 3, 45315075, "WaitEndN", 883660955, -1, 6, UINT32_MAX },
        { 4, 45315197, "WaitBeginN", 644542265, -1, 7, UINT32_MAX },
        { 4, 45315215, "WaitEndN", 644542265, -1, 7, UINT32_MAX },
        { 5, 45315855, "WaitBeginN", 2491771, -1, 8, UINT32_MAX },
        { 11, 45353552, "AddedN", 2121723309, -1, 17, UINT32_MAX },
        { 5, 45399255, "WaitEndN", 2491771, -1, 8, UINT32_MAX },
        { 5, 45399276, "WaitEndN", 2491771, -1, 10, UINT32_MAX },
        { 5, 45399286, "DeletionN", 2491771, -1, 10, UINT32_MAX },
        { 5, 45399358, "NWonRaceOverP", 2491771, -1, 8, 9 },
        { 5, 45399364, "DeletionP", 2491771, -1, 9, UINT32_MAX },
        { 6, 45399442, "CacheHitForP", 3234710487, -1, 12, UINT32_MAX },
        { 7, 45399453, "CacheHitForP", 1997496709, -1, 13, UINT32_MAX },
        { 8, 45399462, "CacheHitForP", 1301841557, -1, 14, UINT32_MAX },
        { 9, 45399505, "CacheHitForP", 2709710687, -1, 15, UINT32_MAX },
        { 10, 45399517, "CacheHitForP", 138923328, -1, 16, UINT32_MAX },
        { 11, 45399526, "CacheHitForP", 2121723309, -1, 17, UINT32_MAX },
        { 6, 45399537, "WaitBeginN", 3234710487, -1, 12, UINT32_MAX },
        { 6, 45399572, "WaitEndN", 3234710487, -1, 12, UINT32_MAX },
        { 7, 45399692, "WaitBeginN", 1997496709, -1, 13, UINT32_MAX },
        { 7, 45399712, "WaitEndN", 1997496709, -1, 13, UINT32_MAX },
        { 5, 45399717, "NWonRaceOverP", 2491771, -1, 8, 11 },
        { 5, 45399724, "DeletionP", 2491771, -1, 11, UINT32_MAX },
        { 6, 45399740, "CacheHitForN", 3234710487, -1, 12, UINT32_MAX },
        { 6, 45399771, "CacheHitForP", 3234710487, -1, 12, UINT32_MAX },
        { 7, 45399783, "CacheHitForP", 1997496709, -1, 13, UINT32_MAX },
        { 8, 45399792, "CacheHitForP", 1301841557, -1, 14, UINT32_MAX },
        { 8, 45399827, "WaitBeginN", 1301841557, -1, 14, UINT32_MAX },
        { 9, 45399832, "CacheHitForP", 2709710687, -1, 15, UINT32_MAX },
        { 10, 45399898, "CacheHitForP", 138923328, -1, 16, UINT32_MAX },
        { 11, 45399912, "CacheHitForP", 2121723309, -1, 17, UINT32_MAX },
        { 7, 45399948, "CacheHitForN", 1997496709, -1, 13, UINT32_MAX },
        { 8, 45400150, "CacheHitForN", 1301841557, -1, 14, UINT32_MAX },
        { 9, 45400344, "CacheHitForN", 2709710687, -1, 15, UINT32_MAX },
        { 10, 45400525, "CacheHitForN", 138923328, -1, 16, UINT32_MAX },
        { 11, 45400698, "CacheHitForN", 2121723309, -1, 17, UINT32_MAX },
        { 8, 45460947, "WaitEndN", 1301841557, -1, 14, UINT32_MAX },
        { 9, 45461235, "WaitBeginN", 2709710687, -1, 15, UINT32_MAX },
        { 9, 45461272, "WaitEndN", 2709710687, -1, 15, UINT32_MAX },
        { 10, 45461389, "WaitBeginN", 138923328, -1, 16, UINT32_MAX },
        { 10, 45489215, "WaitEndN", 138923328, -1, 16, UINT32_MAX },
        { 11, 45489435, "WaitBeginN", 2121723309, -1, 17, UINT32_MAX },
        { 11, 45587086, "WaitEndN", 2121723309, -1, 17, UINT32_MAX },
        { 0, 45596392, "DeletionN", 477900207, -1, 2, UINT32_MAX },
        { 1, 45596431, "DeletionN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 45596461, "DeletionN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 45596492, "DeletionN", 883660955, -1, 6, UINT32_MAX },
        { 4, 45596524, "DeletionN", 644542265, -1, 7, UINT32_MAX },
        { 5, 45596704, "DeletionN", 2491771, -1, 8, UINT32_MAX },
        { 6, 45596733, "DeletionN", 3234710487, -1, 12, UINT32_MAX },
        { 7, 45596762, "DeletionN", 1997496709, -1, 13, UINT32_MAX },
        { 8, 45596800, "DeletionN", 1301841557, -1, 14, UINT32_MAX },
        { 9, 45596835, "DeletionN", 2709710687, -1, 15, UINT32_MAX },
        { 10, 45596868, "DeletionN", 138923328, -1, 16, UINT32_MAX },
        { 11, 45596922, "DeletionN", 2121723309, -1, 17, UINT32_MAX },
};

static bool IsP(const char* tag) {
    if (!strcmp(tag, "AddedN")) return false;
    if (!strcmp(tag, "AddedP")) return true;
    if (!strcmp(tag, "NWonRaceOverN")) return false;
    if (!strcmp(tag, "NWonRaceOverP")) return true;
    if (!strcmp(tag, "PWonRaceOverN")) return false;
    if (!strcmp(tag, "PWonRaceOverP")) return true;

    if (!strcmp(tag, "DeletionN")) return false;
    if (!strcmp(tag, "DeletionP")) return true;

    SkASSERT(0);
    return false;
}

struct FSMNode {
    // The ThreadedCompilePrecompileTest uses two Precompile threads and two Normal threads so,
    // at worst, we can get 4 Pipelines for the same uniqueID.
    static const int kMaxPipelinesInAGivenState = 4;

    void addToState(uint32_t dude, uint64_t entryTime) {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState1[i] == UINT32_MAX) {
                SkASSERT(fEntryTime[i] == 0);
                fInState1[i] = dude;
                fEntryTime[i] = entryTime;
                return;
            }
        }

        SkASSERT(0);
    }

    uint64_t removeFromState(uint32_t dude) {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState1[i] == dude) {
                SkASSERT(fEntryTime[i] != 0);
                fInState1[i] = UINT32_MAX;
                uint64_t tmp = fEntryTime[i];
                fEntryTime[i] = 0;
                return tmp;
            }
        }

        SkASSERT(0);
        return 0;
    }

    bool isInState(uint32_t dude) const {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState1[i] == dude) {
                SkASSERT(fEntryTime[i] != 0);
                return true;
            }
        }

        return false;
    }

    bool isEmpty() const {
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState1[i] != UINT32_MAX) {
                return false;
            }
        }

        return true;
    }

    uint32_t fInState1[kMaxPipelinesInAGivenState] = {
            UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX
    };
    uint64_t fEntryTime[kMaxPipelinesInAGivenState] = { 0, 0, 0, 0 };
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
    //        NWonRaceOverN, NWonRaceOverP", "PWonRaceOverN" or "PWonRaceOverP --> kEndBuild --> kToBeDeleted
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
 0:  477900207 [ 1004 CacheMissForP 0][ 1020 CacheMissForP 1][ 1032 BeginBuildP 0][ 1032 BeginBuildP 1][36380 CacheMissForN 2][36392 CacheMissForN 3][36396 BeginBuildN 2][36396 BeginBuildN 3][76368 AddedN 2][76384 NWonRaceOverN 2 3][76394 WaitBeginN 3][77595 WaitBeginN 2][255347 NWonRaceOverP 2 1][45069728 WaitEndN 2][45077308 WaitEndN 3][45086983 NWonRaceOverP 2 0]
 1: 2189112322 [77436 CacheMissForN 4][77443 BeginBuildN 4][115627 AddedN 4][45064868 CacheHitForP 4][45070601 WaitBeginN 4][45078344 CacheHitForN 4][45087044 CacheHitForP 4][45144862 WaitEndN 4]
 2: 1147256140 [116243 CacheMissForN 5][116249 BeginBuildN 5][154134 AddedN 5][45064919 CacheHitForP 5][45078928 CacheHitForN 5][45087059 CacheHitForP 5][45145500 WaitBeginN 5][45314804 WaitEndN 5]
 3:  883660955 [154611 CacheMissForN 6][154617 BeginBuildN 6][191593 AddedN 6][45065109 CacheHitForP 6][45079377 CacheHitForN 6][45087135 CacheHitForP 6][45315050 WaitBeginN 6][45315075 WaitEndN 6]
 4:  644542265 [192107 CacheMissForN 7][192114 BeginBuildN 7][237454 AddedN 7][45065133 CacheHitForP 7][45079694 CacheHitForN 7][45087150 CacheHitForP 7][45315197 WaitBeginN 7][45315215 WaitEndN 7]
 5:    2491771 [237923 CacheMissForN 8][237927 BeginBuildN 8][45065153 CacheMissForP 9][45065158 BeginBuildP 9][45079980 CacheMissForN 10][45079984 BeginBuildN 10][45087163 CacheMissForP 11][45087166 BeginBuildP 11][45096111 AddedN 8][45138143 NWonRaceOverN 8 10][45138155 WaitBeginN 10][45315855 WaitBeginN 8][45399255 WaitEndN 8][45399276 WaitEndN 10][45399358 NWonRaceOverP 8 9][45399717 NWonRaceOverP 8 11]
 6: 3234710487 [45096609 CacheMissForN 12][45096618 BeginBuildN 12][45136671 AddedN 12][45399442 CacheHitForP 12][45399537 WaitBeginN 12][45399572 WaitEndN 12][45399740 CacheHitForN 12][45399771 CacheHitForP 12]
 7: 1997496709 [45137099 CacheMissForN 13][45137103 BeginBuildN 13][45174789 AddedN 13][45399453 CacheHitForP 13][45399692 WaitBeginN 13][45399712 WaitEndN 13][45399782 CacheHitForP 13][45399948 CacheHitForN 13]
 8: 1301841557 [45175257 CacheMissForN 14][45175260 BeginBuildN 14][45212554 AddedN 14][45399462 CacheHitForP 14][45399792 CacheHitForP 14][45399827 WaitBeginN 14][45400150 CacheHitForN 14][45460947 WaitEndN 14]
 9: 2709710687 [45213032 CacheMissForN 15][45213038 BeginBuildN 15][45253408 AddedN 15][45399505 CacheHitForP 15][45399831 CacheHitForP 15][45400344 CacheHitForN 15][45461235 WaitBeginN 15][45461272 WaitEndN 15]
10:  138923328 [45254441 CacheMissForN 16][45254453 BeginBuildN 16][45313657 AddedN 16][45399517 CacheHitForP 16][45399898 CacheHitForP 16][45400525 CacheHitForN 16][45461389 WaitBeginN 16][45489215 WaitEndN 16]
11: 2121723309 [45314133 CacheMissForN 17][45314137 BeginBuildN 17][45353552 AddedN 17][45399526 CacheHitForP 17][45399912 CacheHitForP 17][45400698 CacheHitForN 17][45489435 WaitBeginN 17][45587086 WaitEndN 17]
#endif

#if 0
Reconstructed
 0:  477900207 [ 1004 CacheMissForP 0][ 1020 CacheMissForP 1][ 1032 BeginBuildP 0][ 1032 BeginBuildP 1][36380 CacheMissForN 2][36392 CacheMissForN 3][36396 BeginBuildN 2][36397 BeginBuildN 3][76368 AddedN 2][76384 NWonRaceOverN 2 3][76394 WaitBeginN 3][77595 WaitBeginN 2][255348 NWonRaceOverP 2 1][255380 DeletionP 1][45069728 WaitEndN 2][45077308 WaitEndN 3][45077333 DeletionN 3][45086983 NWonRaceOverP 2 0][45087004 DeletionP 0][45596392 DeletionN 2]
 1: 2189112322 [77436 CacheMissForN 4][77443 BeginBuildN 4][115628 AddedN 4][45064868 CacheHitForP 4][45070601 WaitBeginN 4][45078344 CacheHitForN 4][45087044 CacheHitForP 4][45144863 WaitEndN 4][45596431 DeletionN 4]
 2: 1147256140 [116243 CacheMissForN 5][116249 BeginBuildN 5][154134 AddedN 5][45064919 CacheHitForP 5][45078928 CacheHitForN 5][45087060 CacheHitForP 5][45145500 WaitBeginN 5][45314804 WaitEndN 5][45596461 DeletionN 5]
 3:  883660955 [154611 CacheMissForN 6][154617 BeginBuildN 6][191593 AddedN 6][45065109 CacheHitForP 6][45079377 CacheHitForN 6][45087135 CacheHitForP 6][45315050 WaitBeginN 6][45315075 WaitEndN 6][45596492 DeletionN 6]
 4:  644542265 [192107 CacheMissForN 7][192114 BeginBuildN 7][237454 AddedN 7][45065133 CacheHitForP 7][45079694 CacheHitForN 7][45087150 CacheHitForP 7][45315197 WaitBeginN 7][45315215 WaitEndN 7][45596524 DeletionN 7]
 5:    2491771 [237923 CacheMissForN 8][237927 BeginBuildN 8][45065153 CacheMissForP 9][45065158 BeginBuildP 9][45079980 CacheMissForN 10][45079984 BeginBuildN 10][45087163 CacheMissForP 11][45087166 BeginBuildP 11][45096111 AddedN 8][45138143 NWonRaceOverN 8 10][45138156 WaitBeginN 10][45315855 WaitBeginN 8][45399255 WaitEndN 8][45399276 WaitEndN 10][45399286 DeletionN 10][45399358 NWonRaceOverP 8 9][45399364 DeletionP 9][45399717 NWonRaceOverP 8 11][45399724 DeletionP 11][45596704 DeletionN 8]
 6: 3234710487 [45096609 CacheMissForN 12][45096618 BeginBuildN 12][45136671 AddedN 12][45399442 CacheHitForP 12][45399537 WaitBeginN 12][45399572 WaitEndN 12][45399740 CacheHitForN 12][45399771 CacheHitForP 12][45596733 DeletionN 12]
 7: 1997496709 [45137099 CacheMissForN 13][45137103 BeginBuildN 13][45174789 AddedN 13][45399453 CacheHitForP 13][45399692 WaitBeginN 13][45399712 WaitEndN 13][45399783 CacheHitForP 13][45399948 CacheHitForN 13][45596762 DeletionN 13]
 8: 1301841557 [45175257 CacheMissForN 14][45175260 BeginBuildN 14][45212555 AddedN 14][45399462 CacheHitForP 14][45399792 CacheHitForP 14][45399827 WaitBeginN 14][45400150 CacheHitForN 14][45460947 WaitEndN 14][45596800 DeletionN 14]
 9: 2709710687 [45213032 CacheMissForN 15][45213038 BeginBuildN 15][45253409 AddedN 15][45399505 CacheHitForP 15][45399832 CacheHitForP 15][45400344 CacheHitForN 15][45461235 WaitBeginN 15][45461272 WaitEndN 15][45596835 DeletionN 15]
10:  138923328 [45254441 CacheMissForN 16][45254453 BeginBuildN 16][45313657 AddedN 16][45399517 CacheHitForP 16][45399898 CacheHitForP 16][45400525 CacheHitForN 16][45461389 WaitBeginN 16][45489215 WaitEndN 16][45596868 DeletionN 16]
11: 2121723309 [45314133 CacheMissForN 17][45314137 BeginBuildN 17][45353552 AddedN 17][45399526 CacheHitForP 17][45399912 CacheHitForP 17][45400698 CacheHitForN 17][45489435 WaitBeginN 17][45587086 WaitEndN 17][45596922 DeletionN 17]
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
                    SkDebugf("[%5lld %s",
                             b.fTime,
                             b.fTag);

                    if (b.fPipelineID1 != UINT32_MAX) {
                        SkDebugf(" %u", b.fPipelineID1);
                    }
                    if (b.fPipelineID2 != UINT32_MAX) {
                        SkDebugf(" %u", b.fPipelineID2);
                    }

                    SkDebugf("]");
                    log.push_back(b);
                }
            }
            SkDebugf("\n");

            FSMNode fsm[Node::kLast+1];

            // Next steps:
            //   add time of addition to FSM - why?
            //   compute compile times for the N/P cases - create mean & std dev.
            //   resolve Bad Wait mystery - how can two Dawn pipelines be waiting!!
            //   maybe add a deleted state?
            //     - I think we can get this from State A below and the purge state
            //   Add purge state handling and testing!
            //   Compute the bad wait times - create mean and std dev
            int64_t waitStart = 0;
            for (auto l : log) {
                if (!strcmp(l.fTag, "CacheMissForP") ||
                    !strcmp(l.fTag, "CacheMissForN")) {
                    fsm[Node::kMiss].addToState(l.fPipelineID1, l.fTime);
                } else if (!strcmp(l.fTag, "BeginBuildP") ||
                           !strcmp(l.fTag, "BeginBuildN")) {
                    fsm[Node::kMiss].removeFromState(l.fPipelineID1);
                    fsm[Node::kBeginBuild].addToState(l.fPipelineID1, l.fTime);
                } else if (!strcmp(l.fTag, "AddedN") ||
                           !strcmp(l.fTag, "AddedP")) {
                    bool isP = IsP(l.fTag);

                    uint64_t beginTime = fsm[Node::kBeginBuild].removeFromState(l.fPipelineID1);
                    fsm[Node::kEndBuild].addToState(l.fPipelineID1, l.fTime);
                    // compute compile time here
                    int64_t diff = l.fTime - beginTime;
                    compileTimeTotals[isP] += diff;
                    compileCounts[isP]++;

                    fsm[Node::kEndBuild].removeFromState(l.fPipelineID1);
                    fsm[Node::kAddedToCache].addToState(l.fPipelineID1, l.fTime);
                } else if (!strcmp(l.fTag, "NWonRaceOverN") ||
                           !strcmp(l.fTag, "NWonRaceOverP") ||
                           !strcmp(l.fTag, "PWonRaceOverN") ||
                           !strcmp(l.fTag, "PWonRaceOverP")) {
                    bool isP = IsP(l.fTag);

                    SkASSERT(fsm[Node::kAddedToCache].isInState(l.fPipelineID1)); // fPipelineID1 won
                    uint64_t beginTime = fsm[Node::kBeginBuild].removeFromState(l.fPipelineID2);
                    fsm[Node::kEndBuild].addToState(l.fPipelineID2, l.fTime);

                    int64_t diff = l.fTime - beginTime;
                    compileTimeTotals[isP] += diff;
                    compileCounts[isP]++;

                    // compute compile time here
                    // State A
                    // fPipelineID2 lost and gets destroyed a total waste of time!!
                    fsm[Node::kEndBuild].removeFromState(l.fPipelineID2);
                    fsm[Node::kToBeDeleted].addToState(l.fPipelineID2, l.fTime);
                } else if (!strcmp(l.fTag, "CacheHitForP") ||
                           !strcmp(l.fTag, "CacheHitForN")) {
                    // saved work - esp for 'P'
                } else if (!strcmp(l.fTag, "WaitBeginN")) {
                    // There is a mystery going on here! How are two Dawn pipelines both
                    // getting waited on!!
                    // What! This assert should be true!!!
//                    SkASSERT(fsm[Node::kAddedToCache].isInState(l.fPipelineID1));
                    waitStart = l.fTime;
                } else if (!strcmp(l.fTag, "WaitEndN")) {
                    // As above, this assert should be true!!!
//                    SkASSERT(fsm[Node::kAddedToCache].isInState(l.fPipelineID1));
                    SkASSERT(waitStart != 0);
                    int64_t diff = l.fTime - waitStart;
                    SkDebugf("Bad wait %lld\n", diff);
                } else if (!strcmp(l.fTag, "DeletionP") ||
                           !strcmp(l.fTag, "DeletionN")) {
//                    bool isP = IsP(l.fTag);

                    if (fsm[Node::kAddedToCache].isInState(l.fPipelineID1)) {
                        fsm[Node::kAddedToCache].removeFromState(l.fPipelineID1);
                        fsm[Node::kDeleted].addToState(l.fPipelineID1, l.fTime);
                    } else {
                        SkASSERT(fsm[Node::kToBeDeleted].isInState(l.fPipelineID1));
                        fsm[Node::kToBeDeleted].removeFromState(l.fPipelineID1);
                        fsm[Node::kDeleted].addToState(l.fPipelineID1, l.fTime);
                    }
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
