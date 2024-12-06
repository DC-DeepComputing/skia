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
        { 0, 935, "CacheMissForP", 477900207, -1, 0, UINT32_MAX },
        { 0, 950, "CacheMissForP", 477900207, -1, 1, UINT32_MAX },
        { 0, 954, "BeginBuildP", 477900207, -1, 0, UINT32_MAX },
        { 0, 955, "BeginBuildP", 477900207, -1, 1, UINT32_MAX },
        { 0, 35863, "CacheMissForN", 477900207, -1, 2, UINT32_MAX },
        { 0, 35874, "BeginBuildN", 477900207, -1, 2, UINT32_MAX },
        { 0, 35888, "CacheMissForN", 477900207, -1, 3, UINT32_MAX },
        { 0, 35895, "BeginBuildN", 477900207, -1, 3, UINT32_MAX },
        { 0, 75148, "AddedN", 477900207, -1, 3, UINT32_MAX },
        { 0, 75190, "NWonRaceOverN", 477900207, -1, 3, 2 },
        { 0, 75195, "WaitBeginN", 477900207, -1, 2, UINT32_MAX },
        { 1, 76048, "CacheMissForN", 2189112322, -1, 4, UINT32_MAX },
        { 1, 76057, "BeginBuildN", 2189112322, -1, 4, UINT32_MAX },
        { 0, 76198, "WaitBeginN", 477900207, -1, 3, UINT32_MAX },
        { 1, 111379, "AddedN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 111814, "CacheMissForN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 111826, "BeginBuildN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 147796, "AddedN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 148252, "CacheMissForN", 883660955, -1, 6, UINT32_MAX },
        { 3, 148259, "BeginBuildN", 883660955, -1, 6, UINT32_MAX },
        { 3, 183004, "AddedN", 883660955, -1, 6, UINT32_MAX },
        { 4, 183462, "CacheMissForN", 644542265, -1, 7, UINT32_MAX },
        { 4, 183470, "BeginBuildN", 644542265, -1, 7, UINT32_MAX },
        { 4, 219833, "AddedN", 644542265, -1, 7, UINT32_MAX },
        { 5, 220289, "CacheMissForN", 2491771, -1, 8, UINT32_MAX },
        { 5, 220295, "BeginBuildN", 2491771, -1, 8, UINT32_MAX },
        { 5, 263739, "AddedN", 2491771, -1, 8, UINT32_MAX },
        { 6, 264208, "CacheMissForN", 3234710487, -1, 9, UINT32_MAX },
        { 6, 264211, "BeginBuildN", 3234710487, -1, 9, UINT32_MAX },
        { 6, 306583, "AddedN", 3234710487, -1, 9, UINT32_MAX },
        { 7, 307026, "CacheMissForN", 1997496709, -1, 10, UINT32_MAX },
        { 7, 307029, "BeginBuildN", 1997496709, -1, 10, UINT32_MAX },
        { 7, 350048, "AddedN", 1997496709, -1, 10, UINT32_MAX },
        { 8, 350573, "CacheMissForN", 1301841557, -1, 11, UINT32_MAX },
        { 8, 350581, "BeginBuildN", 1301841557, -1, 11, UINT32_MAX },
        { 8, 388483, "AddedN", 1301841557, -1, 11, UINT32_MAX },
        { 9, 389127, "CacheMissForN", 2709710687, -1, 12, UINT32_MAX },
        { 9, 389137, "BeginBuildN", 2709710687, -1, 12, UINT32_MAX },
        { 0, 420484, "WaitEndN", 477900207, -1, 3, UINT32_MAX },
        { 0, 420570, "NWonRaceOverP", 477900207, -1, 3, 0 },
        { 1, 420626, "CacheHitForP", 2189112322, -1, 4, UINT32_MAX },
        { 2, 420638, "CacheHitForP", 1147256140, -1, 5, UINT32_MAX },
        { 3, 420707, "CacheHitForP", 883660955, -1, 6, UINT32_MAX },
        { 4, 420717, "CacheHitForP", 644542265, -1, 7, UINT32_MAX },
        { 5, 420726, "CacheHitForP", 2491771, -1, 8, UINT32_MAX },
        { 6, 420763, "CacheHitForP", 3234710487, -1, 9, UINT32_MAX },
        { 7, 420773, "CacheHitForP", 1997496709, -1, 10, UINT32_MAX },
        { 8, 420781, "CacheHitForP", 1301841557, -1, 11, UINT32_MAX },
        { 9, 420826, "CacheMissForP", 2709710687, -1, 13, UINT32_MAX },
        { 9, 420828, "BeginBuildP", 2709710687, -1, 13, UINT32_MAX },
        { 1, 421640, "WaitBeginN", 2189112322, -1, 4, UINT32_MAX },
        { 0, 420542, "WaitEndN", 477900207, -1, 2, UINT32_MAX },
        { 0, 422022, "NWonRaceOverP", 477900207, -1, 3, 1 },
        { 1, 422087, "CacheHitForP", 2189112322, -1, 4, UINT32_MAX },
        { 2, 422114, "CacheHitForP", 1147256140, -1, 5, UINT32_MAX },
        { 3, 422229, "CacheHitForP", 883660955, -1, 6, UINT32_MAX },
        { 4, 422256, "CacheHitForP", 644542265, -1, 7, UINT32_MAX },
        { 5, 422282, "CacheHitForP", 2491771, -1, 8, UINT32_MAX },
        { 6, 422388, "CacheHitForP", 3234710487, -1, 9, UINT32_MAX },
        { 7, 422449, "CacheHitForP", 1997496709, -1, 10, UINT32_MAX },
        { 8, 422475, "CacheHitForP", 1301841557, -1, 11, UINT32_MAX },
        { 9, 422589, "CacheMissForP", 2709710687, -1, 14, UINT32_MAX },
        { 9, 422595, "BeginBuildP", 2709710687, -1, 14, UINT32_MAX },
        { 1, 422610, "CacheHitForN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 423085, "CacheHitForN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 423534, "CacheHitForN", 883660955, -1, 6, UINT32_MAX },
        { 4, 424004, "CacheHitForN", 644542265, -1, 7, UINT32_MAX },
        { 5, 424492, "CacheHitForN", 2491771, -1, 8, UINT32_MAX },
        { 6, 424985, "CacheHitForN", 3234710487, -1, 9, UINT32_MAX },
        { 7, 425487, "CacheHitForN", 1997496709, -1, 10, UINT32_MAX },
        { 8, 426453, "CacheHitForN", 1301841557, -1, 11, UINT32_MAX },
        { 9, 427270, "CacheMissForN", 2709710687, -1, 15, UINT32_MAX },
        { 9, 427283, "AddedN", 2709710687, -1, 12, UINT32_MAX },
        { 9, 427291, "BeginBuildN", 2709710687, -1, 15, UINT32_MAX },
        { 10, 427679, "CacheMissForN", 138923328, -1, 16, UINT32_MAX },
        { 10, 427683, "BeginBuildN", 138923328, -1, 16, UINT32_MAX },
        { 9, 475413, "NWonRaceOverN", 2709710687, -1, 12, 15 },
        { 9, 475427, "WaitBeginN", 2709710687, -1, 15, UINT32_MAX },
        { 10, 481025, "AddedN", 138923328, -1, 16, UINT32_MAX },
        { 11, 481498, "CacheMissForN", 2121723309, -1, 17, UINT32_MAX },
        { 11, 481502, "BeginBuildN", 2121723309, -1, 17, UINT32_MAX },
        { 11, 520144, "AddedN", 2121723309, -1, 17, UINT32_MAX },
        { 1, 545021, "WaitEndN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 545480, "WaitBeginN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 681628, "WaitEndN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 681954, "WaitBeginN", 883660955, -1, 6, UINT32_MAX },
        { 3, 681996, "WaitEndN", 883660955, -1, 6, UINT32_MAX },
        { 4, 682141, "WaitBeginN", 644542265, -1, 7, UINT32_MAX },
        { 4, 682165, "WaitEndN", 644542265, -1, 7, UINT32_MAX },
        { 5, 682289, "WaitBeginN", 2491771, -1, 8, UINT32_MAX },
        { 9, 746446, "WaitEndN", 2709710687, -1, 15, UINT32_MAX },
        { 9, 746524, "NWonRaceOverP", 2709710687, -1, 12, 13 },
        { 10, 746583, "CacheHitForP", 138923328, -1, 16, UINT32_MAX },
        { 11, 746595, "CacheHitForP", 2121723309, -1, 17, UINT32_MAX },
        { 10, 746937, "CacheHitForN", 138923328, -1, 16, UINT32_MAX },
        { 9, 746949, "NWonRaceOverP", 2709710687, -1, 12, 14 },
        { 10, 746982, "CacheHitForP", 138923328, -1, 16, UINT32_MAX },
        { 11, 746994, "CacheHitForP", 2121723309, -1, 17, UINT32_MAX },
        { 11, 747131, "CacheHitForN", 2121723309, -1, 17, UINT32_MAX },
        { 5, 773062, "WaitEndN", 2491771, -1, 8, UINT32_MAX },
        { 6, 773360, "WaitBeginN", 3234710487, -1, 9, UINT32_MAX },
        { 6, 773394, "WaitEndN", 3234710487, -1, 9, UINT32_MAX },
        { 7, 773515, "WaitBeginN", 1997496709, -1, 10, UINT32_MAX },
        { 7, 773536, "WaitEndN", 1997496709, -1, 10, UINT32_MAX },
        { 8, 773651, "WaitBeginN", 1301841557, -1, 11, UINT32_MAX },
        { 8, 849272, "WaitEndN", 1301841557, -1, 11, UINT32_MAX },
        { 9, 850500, "WaitBeginN", 2709710687, -1, 12, UINT32_MAX },
        { 9, 850529, "WaitEndN", 2709710687, -1, 12, UINT32_MAX },
        { 10, 850634, "WaitBeginN", 138923328, -1, 16, UINT32_MAX },
        { 10, 850656, "WaitEndN", 138923328, -1, 16, UINT32_MAX },
        { 11, 850762, "WaitBeginN", 2121723309, -1, 17, UINT32_MAX },
        { 11, 866345, "WaitEndN", 2121723309, -1, 17, UINT32_MAX },
};

struct FSMNode {
    // The ThreadedCompilePrecompileTest uses two Precompile threads and two Normal threads so,
    // at worst, we can get 4 Pipelines for the same uniqueID.
    static const int kMaxPipelinesInAGivenState = 4;

    void addToState(uint32_t dude) {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState[i] == UINT32_MAX) {
                fInState[i] = dude;
                return;
            }
        }

        SkASSERT(0);
    }

    void removeFromState(uint32_t dude) {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState[i] == dude) {
                fInState[i] = UINT32_MAX;
                return;
            }
        }

        SkASSERT(0);
    }

    bool isInState(uint32_t dude) const {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < kMaxPipelinesInAGivenState; ++i) {
            if (fInState[i] == dude) {
                return true;
            }
        }

        return false;
    }

    uint32_t fInState[kMaxPipelinesInAGivenState] = {
            UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX
    };
};

enum Node {
    // The usual first node in the FSM. This is where a given pipeline is first detected as a
    // cache miss. It can be either 'N' or 'P'.
    kMiss = 0,

    kBeginBuild = 1,
    kEndBuild = 2,
    kAddedToCache = 3,

    kLast = kAddedToCache,
};

#if 0
Original
 0:  477900207 [  935 CacheMissForP 0][  950 CacheMissForP 1][  954 BeginBuildP 0][  955 BeginBuildP 1][35863 CacheMissForN 2][35874 BeginBuildN 2][35887 CacheMissForN 3][35895 BeginBuildN 3][75148 AddedN 3][75190 NWonRaceOverN 3 2][75195 WaitBeginN 2][76198 WaitBeginN 3][420484 WaitEndN 3][420542 WaitEndN 2][420569 NWonRaceOverP 3 0][422022 NWonRaceOverP 3 1]
 1: 2189112322 [76048 CacheMissForN 4][76057 BeginBuildN 4][111379 AddedN 4][420626 CacheHitForP 4][421639 WaitBeginN 4][422086 CacheHitForP 4][422610 CacheHitForN 4][545021 WaitEndN 4]
 2: 1147256140 [111814 CacheMissForN 5][111826 BeginBuildN 5][147796 AddedN 5][420638 CacheHitForP 5][422114 CacheHitForP 5][423085 CacheHitForN 5][545479 WaitBeginN 5][681628 WaitEndN 5]
 3:  883660955 [148252 CacheMissForN 6][148259 BeginBuildN 6][183004 AddedN 6][420707 CacheHitForP 6][422229 CacheHitForP 6][423534 CacheHitForN 6][681953 WaitBeginN 6][681996 WaitEndN 6]
 4:  644542265 [183462 CacheMissForN 7][183470 BeginBuildN 7][219833 AddedN 7][420717 CacheHitForP 7][422256 CacheHitForP 7][424004 CacheHitForN 7][682140 WaitBeginN 7][682165 WaitEndN 7]
 5:    2491771 [220289 CacheMissForN 8][220295 BeginBuildN 8][263739 AddedN 8][420726 CacheHitForP 8][422282 CacheHitForP 8][424491 CacheHitForN 8][682289 WaitBeginN 8][773062 WaitEndN 8]
 6: 3234710487 [264207 CacheMissForN 9][264211 BeginBuildN 9][306583 AddedN 9][420763 CacheHitForP 9][422388 CacheHitForP 9][424984 CacheHitForN 9][773360 WaitBeginN 9][773394 WaitEndN 9]
 7: 1997496709 [307025 CacheMissForN 10][307029 BeginBuildN 10][350047 AddedN 10][420773 CacheHitForP 10][422448 CacheHitForP 10][425486 CacheHitForN 10][773515 WaitBeginN 10][773536 WaitEndN 10]
 8: 1301841557 [350573 CacheMissForN 11][350580 BeginBuildN 11][388483 AddedN 11][420781 CacheHitForP 11][422475 CacheHitForP 11][426453 CacheHitForN 11][773651 WaitBeginN 11][849272 WaitEndN 11]
 9: 2709710687 [389127 CacheMissForN 12][389136 BeginBuildN 12][420826 CacheMissForP 13][420828 BeginBuildP 13][422589 CacheMissForP 14][422595 BeginBuildP 14][427270 CacheMissForN 15][427283 AddedN 12][427291 BeginBuildN 15][475413 NWonRaceOverN 12 15][475427 WaitBeginN 15][746446 WaitEndN 15][746524 NWonRaceOverP 12 13][746949 NWonRaceOverP 12 14][850500 WaitBeginN 12][850529 WaitEndN 12]
10:  138923328 [427679 CacheMissForN 16][427683 BeginBuildN 16][481025 AddedN 16][746583 CacheHitForP 16][746937 CacheHitForN 16][746982 CacheHitForP 16][850634 WaitBeginN 16][850656 WaitEndN 16]
11: 2121723309 [481498 CacheMissForN 17][481502 BeginBuildN 17][520144 AddedN 17][746595 CacheHitForP 17][746994 CacheHitForP 17][747131 CacheHitForN 17][850762 WaitBeginN 17][866345 WaitEndN 17]
#endif

#if 0
Reconstructed
 0:  477900207 [  935 CacheMissForP 0][  950 CacheMissForP 1][  954 BeginBuildP 0][  955 BeginBuildP 1][35863 CacheMissForN 2][35874 BeginBuildN 2][35888 CacheMissForN 3][35895 BeginBuildN 3][75148 AddedN 3][75190 NWonRaceOverN 3 2][75195 WaitBeginN 2][76198 WaitBeginN 3][420484 WaitEndN 3][420570 NWonRaceOverP 3 0][420542 WaitEndN 2][422022 NWonRaceOverP 3 1]
 1: 2189112322 [76048 CacheMissForN 4][76057 BeginBuildN 4][111379 AddedN 4][420626 CacheHitForP 4][421640 WaitBeginN 4][422087 CacheHitForP 4][422610 CacheHitForN 4][545021 WaitEndN 4]
 2: 1147256140 [111814 CacheMissForN 5][111826 BeginBuildN 5][147796 AddedN 5][420638 CacheHitForP 5][422114 CacheHitForP 5][423085 CacheHitForN 5][545480 WaitBeginN 5][681628 WaitEndN 5]
 3:  883660955 [148252 CacheMissForN 6][148259 BeginBuildN 6][183004 AddedN 6][420707 CacheHitForP 6][422229 CacheHitForP 6][423534 CacheHitForN 6][681954 WaitBeginN 6][681996 WaitEndN 6]
 4:  644542265 [183462 CacheMissForN 7][183470 BeginBuildN 7][219833 AddedN 7][420717 CacheHitForP 7][422256 CacheHitForP 7][424004 CacheHitForN 7][682141 WaitBeginN 7][682165 WaitEndN 7]
 5:    2491771 [220289 CacheMissForN 8][220295 BeginBuildN 8][263739 AddedN 8][420726 CacheHitForP 8][422282 CacheHitForP 8][424492 CacheHitForN 8][682289 WaitBeginN 8][773062 WaitEndN 8]
 6: 3234710487 [264208 CacheMissForN 9][264211 BeginBuildN 9][306583 AddedN 9][420763 CacheHitForP 9][422388 CacheHitForP 9][424985 CacheHitForN 9][773360 WaitBeginN 9][773394 WaitEndN 9]
 7: 1997496709 [307026 CacheMissForN 10][307029 BeginBuildN 10][350048 AddedN 10][420773 CacheHitForP 10][422449 CacheHitForP 10][425487 CacheHitForN 10][773515 WaitBeginN 10][773536 WaitEndN 10]
 8: 1301841557 [350573 CacheMissForN 11][350581 BeginBuildN 11][388483 AddedN 11][420781 CacheHitForP 11][422475 CacheHitForP 11][426453 CacheHitForN 11][773651 WaitBeginN 11][849272 WaitEndN 11]
 9: 2709710687 [389127 CacheMissForN 12][389137 BeginBuildN 12][420826 CacheMissForP 13][420828 BeginBuildP 13][422589 CacheMissForP 14][422595 BeginBuildP 14][427270 CacheMissForN 15][427283 AddedN 12][427291 BeginBuildN 15][475413 NWonRaceOverN 12 15][475427 WaitBeginN 15][746446 WaitEndN 15][746524 NWonRaceOverP 12 13][746949 NWonRaceOverP 12 14][850500 WaitBeginN 12][850529 WaitEndN 12]
10:  138923328 [427679 CacheMissForN 16][427683 BeginBuildN 16][481025 AddedN 16][746583 CacheHitForP 16][746937 CacheHitForN 16][746982 CacheHitForP 16][850634 WaitBeginN 16][850656 WaitEndN 16]
11: 2121723309 [481498 CacheMissForN 17][481502 BeginBuildN 17][520144 AddedN 17][746595 CacheHitForP 17][746994 CacheHitForP 17][747131 CacheHitForN 17][850762 WaitBeginN 17][866345 WaitEndN 17]
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
                    fsm[Node::kMiss].addToState(l.fPipelineID1);
                } else if (!strcmp(l.fTag, "BeginBuildP") ||
                           !strcmp(l.fTag, "BeginBuildN")) {
                    fsm[Node::kMiss].removeFromState(l.fPipelineID1);
                    fsm[Node::kBeginBuild].addToState(l.fPipelineID1);
                } else if (!strcmp(l.fTag, "AddedN") ||
                           !strcmp(l.fTag, "AddedP")) {
                    fsm[Node::kBeginBuild].removeFromState(l.fPipelineID1);
                    fsm[Node::kEndBuild].addToState(l.fPipelineID1);
                    // compute compile time here
                    fsm[Node::kEndBuild].removeFromState(l.fPipelineID1);
                    fsm[Node::kAddedToCache].addToState(l.fPipelineID1);
                } else if (!strcmp(l.fTag, "NWonRaceOverN") ||
                           !strcmp(l.fTag, "NWonRaceOverP") ||
                           !strcmp(l.fTag, "PWonRaceOverN") ||
                           !strcmp(l.fTag, "PWonRaceOverP")) {
                    fsm[Node::kAddedToCache].isInState(l.fPipelineID1); // fPipelineID1 won
                    fsm[Node::kBeginBuild].removeFromState(l.fPipelineID2);
                    fsm[Node::kEndBuild].addToState(l.fPipelineID2);
                    // compute compile time here
                    // State A
                    // fPipelineID2 lost and gets destroyed a total waste of time!!
                } else if (!strcmp(l.fTag, "CacheHitForP") ||
                           !strcmp(l.fTag, "CacheHitForN")) {
                    // saved work - esp for 'P'
                } else if (!strcmp(l.fTag, "WaitBeginN")) {
                    // There is a mystery going on here! How are two Dawn pipelines both
                    // getting waited on!!
                    // What! This should be failing!!!
                    fsm[Node::kAddedToCache].isInState(l.fPipelineID1);
                    waitStart = l.fTime;
                } else if (!strcmp(l.fTag, "WaitEndN")) {
                    fsm[Node::kAddedToCache].isInState(l.fPipelineID1);
                    SkASSERT(waitStart != 0);
                    int64_t diff = l.fTime - waitStart;
                    SkDebugf("Bad wait %lld\n", diff);
                } else {
                    SkDebugf("--------------- Unhandled %s\n", l.fTag);
                }
            }
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
