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

static const struct foo {
    int fID;
    uint64_t fTime;
    const char* fTag;
    uint32_t fHash;
    int fExtra;
    uint32_t fPipelineID1;
    uint32_t fPipelineID2;
} bar[] = {
        { 0, 1110, "CacheMissForP", 477900207, -1, 0, UINT32_MAX },
        { 0, 1126, "CacheMissForP", 477900207, -1, 1, UINT32_MAX },
        { 0, 1140, "BeginBuildP", 477900207, -1, 0, UINT32_MAX },
        { 0, 1140, "BeginBuildP", 477900207, -1, 1, UINT32_MAX },
        { 0, 36162, "CacheMissForN", 477900207, -1, 2, UINT32_MAX },
        { 0, 36177, "CacheMissForN", 477900207, -1, 3, UINT32_MAX },
        { 0, 36181, "BeginBuildN", 477900207, -1, 2, UINT32_MAX },
        { 0, 36182, "BeginBuildN", 477900207, -1, 3, UINT32_MAX },
        { 0, 75213, "AddedN", 477900207, -1, 3, UINT32_MAX },
        { 0, 75320, "NWonRaceOverN", 477900207, -1, 3, 2 },
        { 0, 75332, "WaitBeginN", 477900207, -1, 2, UINT32_MAX },
        { 1, 76161, "CacheMissForN", 2189112322, -1, 4, UINT32_MAX },
        { 1, 76168, "BeginBuildN", 2189112322, -1, 4, UINT32_MAX },
        { 0, 76395, "WaitBeginN", 477900207, -1, 3, UINT32_MAX },
        { 1, 111400, "AddedN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 111786, "CacheMissForN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 111792, "BeginBuildN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 148901, "AddedN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 149369, "CacheMissForN", 883660955, -1, 6, UINT32_MAX },
        { 3, 149378, "BeginBuildN", 883660955, -1, 6, UINT32_MAX },
        { 3, 184464, "AddedN", 883660955, -1, 6, UINT32_MAX },
        { 4, 184851, "CacheMissForN", 644542265, -1, 7, UINT32_MAX },
        { 4, 184859, "BeginBuildN", 644542265, -1, 7, UINT32_MAX },
        { 4, 221640, "AddedN", 644542265, -1, 7, UINT32_MAX },
        { 5, 222035, "CacheMissForN", 2491771, -1, 8, UINT32_MAX },
        { 5, 222039, "BeginBuildN", 2491771, -1, 8, UINT32_MAX },
        { 0, 239705, "NWonRaceOverP", 477900207, -1, 3, 0 },
        { 0, 239714, "WaitEndN", 477900207, -1, 3, UINT32_MAX },
        { 1, 239755, "CacheHitForP", 2189112322, -1, 4, UINT32_MAX },
        { 0, 239758, "WaitEndN", 477900207, -1, 2, UINT32_MAX },
        { 2, 239767, "CacheHitForP", 1147256140, -1, 5, UINT32_MAX },
        { 3, 239824, "CacheHitForP", 883660955, -1, 6, UINT32_MAX },
        { 4, 239834, "CacheHitForP", 644542265, -1, 7, UINT32_MAX },
        { 5, 239842, "CacheMissForP", 2491771, -1, 9, UINT32_MAX },
        { 5, 239844, "BeginBuildP", 2491771, -1, 9, UINT32_MAX },
        { 0, 240082, "NWonRaceOverP", 477900207, -1, 3, 1 },
        { 1, 240104, "CacheHitForP", 2189112322, -1, 4, UINT32_MAX },
        { 2, 240114, "CacheHitForP", 1147256140, -1, 5, UINT32_MAX },
        { 3, 240155, "CacheHitForP", 883660955, -1, 6, UINT32_MAX },
        { 4, 240165, "CacheHitForP", 644542265, -1, 7, UINT32_MAX },
        { 5, 240173, "CacheMissForP", 2491771, -1, 10, UINT32_MAX },
        { 5, 240175, "BeginBuildP", 2491771, -1, 10, UINT32_MAX },
        { 1, 240639, "WaitBeginN", 2189112322, -1, 4, UINT32_MAX },
        { 1, 240654, "CacheHitForN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 241322, "CacheHitForN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 241942, "CacheHitForN", 883660955, -1, 6, UINT32_MAX },
        { 4, 242469, "CacheHitForN", 644542265, -1, 7, UINT32_MAX },
        { 5, 243035, "CacheMissForN", 2491771, -1, 11, UINT32_MAX },
        { 5, 243046, "BeginBuildN", 2491771, -1, 11, UINT32_MAX },
        { 5, 262639, "AddedN", 2491771, -1, 8, UINT32_MAX },
        { 6, 263016, "CacheMissForN", 3234710487, -1, 12, UINT32_MAX },
        { 6, 263020, "BeginBuildN", 3234710487, -1, 12, UINT32_MAX },
        { 5, 285629, "NWonRaceOverN", 2491771, -1, 8, 11 },
        { 5, 285641, "WaitBeginN", 2491771, -1, 11, UINT32_MAX },
        { 1, 289828, "WaitEndN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 290080, "WaitBeginN", 1147256140, -1, 5, UINT32_MAX },
        { 6, 300374, "AddedN", 3234710487, -1, 12, UINT32_MAX },
        { 7, 300819, "CacheMissForN", 1997496709, -1, 13, UINT32_MAX },
        { 7, 300825, "BeginBuildN", 1997496709, -1, 13, UINT32_MAX },
        { 2, 333063, "WaitEndN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 333969, "WaitBeginN", 883660955, -1, 6, UINT32_MAX },
        { 3, 348311, "WaitEndN", 883660955, -1, 6, UINT32_MAX },
        { 7, 348520, "AddedN", 1997496709, -1, 13, UINT32_MAX },
        { 4, 348562, "WaitBeginN", 644542265, -1, 7, UINT32_MAX },
        { 8, 348993, "CacheMissForN", 1301841557, -1, 14, UINT32_MAX },
        { 8, 348999, "BeginBuildN", 1301841557, -1, 14, UINT32_MAX },
        { 8, 389181, "AddedN", 1301841557, -1, 14, UINT32_MAX },
        { 9, 389657, "CacheMissForN", 2709710687, -1, 15, UINT32_MAX },
        { 9, 389663, "BeginBuildN", 2709710687, -1, 15, UINT32_MAX },
        { 4, 415779, "WaitEndN", 644542265, -1, 7, UINT32_MAX },
        { 5, 417054, "WaitBeginN", 2491771, -1, 8, UINT32_MAX },
        { 9, 429517, "AddedN", 2709710687, -1, 15, UINT32_MAX },
        { 10, 429994, "CacheMissForN", 138923328, -1, 16, UINT32_MAX },
        { 10, 429999, "BeginBuildN", 138923328, -1, 16, UINT32_MAX },
        { 5, 448638, "WaitEndN", 2491771, -1, 11, UINT32_MAX },
        { 6, 449098, "CacheHitForN", 3234710487, -1, 12, UINT32_MAX },
        { 7, 449352, "CacheHitForN", 1997496709, -1, 13, UINT32_MAX },
        { 8, 449559, "CacheHitForN", 1301841557, -1, 14, UINT32_MAX },
        { 9, 449755, "CacheHitForN", 2709710687, -1, 15, UINT32_MAX },
        { 10, 449932, "CacheMissForN", 138923328, -1, 17, UINT32_MAX },
        { 10, 449937, "BeginBuildN", 138923328, -1, 17, UINT32_MAX },
        { 5, 450977, "NWonRaceOverP", 2491771, -1, 8, 10 },
        { 6, 451053, "CacheHitForP", 3234710487, -1, 12, UINT32_MAX },
        { 7, 451066, "CacheHitForP", 1997496709, -1, 13, UINT32_MAX },
        { 8, 451075, "CacheHitForP", 1301841557, -1, 14, UINT32_MAX },
        { 9, 451117, "CacheHitForP", 2709710687, -1, 15, UINT32_MAX },
        { 10, 451127, "CacheMissForP", 138923328, -1, 18, UINT32_MAX },
        { 10, 451129, "BeginBuildP", 138923328, -1, 18, UINT32_MAX },
        { 5, 451892, "WaitEndN", 2491771, -1, 8, UINT32_MAX },
        { 6, 452162, "WaitBeginN", 3234710487, -1, 12, UINT32_MAX },
        { 5, 452655, "NWonRaceOverP", 2491771, -1, 8, 9 },
        { 6, 452740, "CacheHitForP", 3234710487, -1, 12, UINT32_MAX },
        { 7, 452751, "CacheHitForP", 1997496709, -1, 13, UINT32_MAX },
        { 8, 452760, "CacheHitForP", 1301841557, -1, 14, UINT32_MAX },
        { 9, 452803, "CacheHitForP", 2709710687, -1, 15, UINT32_MAX },
        { 10, 452813, "CacheMissForP", 138923328, -1, 19, UINT32_MAX },
        { 10, 452815, "BeginBuildP", 138923328, -1, 19, UINT32_MAX },
        { 6, 465285, "WaitEndN", 3234710487, -1, 12, UINT32_MAX },
        { 7, 465495, "WaitBeginN", 1997496709, -1, 13, UINT32_MAX },
        { 10, 469269, "AddedN", 138923328, -1, 16, UINT32_MAX },
        { 11, 469652, "CacheMissForN", 2121723309, -1, 20, UINT32_MAX },
        { 11, 469658, "BeginBuildN", 2121723309, -1, 20, UINT32_MAX },
        { 10, 488832, "NWonRaceOverN", 138923328, -1, 16, 17 },
        { 10, 488844, "WaitBeginN", 138923328, -1, 17, UINT32_MAX },
        { 11, 507966, "AddedN", 2121723309, -1, 20, UINT32_MAX },
        { 7, 526023, "WaitEndN", 1997496709, -1, 13, UINT32_MAX },
        { 8, 526281, "WaitBeginN", 1301841557, -1, 14, UINT32_MAX },
        { 8, 567968, "WaitEndN", 1301841557, -1, 14, UINT32_MAX },
        { 9, 568276, "WaitBeginN", 2709710687, -1, 15, UINT32_MAX },
        { 9, 599418, "WaitEndN", 2709710687, -1, 15, UINT32_MAX },
        { 10, 600255, "WaitBeginN", 138923328, -1, 16, UINT32_MAX },
        { 10, 644632, "WaitEndN", 138923328, -1, 16, UINT32_MAX },
        { 11, 644918, "WaitBeginN", 2121723309, -1, 20, UINT32_MAX },
        { 10, 664959, "WaitEndN", 138923328, -1, 17, UINT32_MAX },
        { 11, 665508, "CacheHitForN", 2121723309, -1, 20, UINT32_MAX },
        { 10, 665986, "NWonRaceOverP", 138923328, -1, 16, 18 },
        { 11, 666015, "CacheHitForP", 2121723309, -1, 20, UINT32_MAX },
        { 10, 667345, "NWonRaceOverP", 138923328, -1, 16, 19 },
        { 11, 667388, "CacheHitForP", 2121723309, -1, 20, UINT32_MAX },
        { 11, 682763, "WaitEndN", 2121723309, -1, 20, UINT32_MAX },
};

struct FSMNode {
    void addToState(uint32_t dude) {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < 4; ++i) {
            if (fInState[i] == UINT32_MAX) {
                fInState[i] = dude;
                return;
            }
        }

        SkASSERT(0);
    }

    void removeFromState(uint32_t dude) {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < 4; ++i) {
            if (fInState[i] == dude) {
                fInState[i] = UINT32_MAX;
                return;
            }
        }

        SkASSERT(0);
    }

    bool isInState(uint32_t dude) const {
        SkASSERT(dude != UINT32_MAX);
        for (int i = 0; i < 4; ++i) {
            if (fInState[i] == dude) {
                return true;
            }
        }

        return false;
    }

    uint32_t fInState[4] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
};

enum Node {
    kMiss = 0,
    kBeginBuild = 1,
    kEndBuild = 2,
    kAddedToCache = 3,

    kLast = kAddedToCache,
};

#if 0
Original
 0:  477900207 [ 1110 CacheMissForP 0][ 1126 CacheMissForP 1][ 1140 BeginBuildP 0][ 1140 BeginBuildP 1][36162 CacheMissForN 2][36177 CacheMissForN 3][36181 BeginBuildN 2][36182 BeginBuildN 3][75213 AddedN 3][75320 NWonRaceOverN 3 2][75332 WaitBeginN 2][76395 WaitBeginN 3][239705 NWonRaceOverP 3 0][239714 WaitEndN 3][239758 WaitEndN 2][240082 NWonRaceOverP 3 1]
 1: 2189112322 [76161 CacheMissForN 4][76168 BeginBuildN 4][111400 AddedN 4][239755 CacheHitForP 4][240104 CacheHitForP 4][240639 WaitBeginN 4][240653 CacheHitForN 4][289828 WaitEndN 4]
 2: 1147256140 [111786 CacheMissForN 5][111792 BeginBuildN 5][148901 AddedN 5][239767 CacheHitForP 5][240114 CacheHitForP 5][241322 CacheHitForN 5][290080 WaitBeginN 5][333062 WaitEndN 5]
 3:  883660955 [149369 CacheMissForN 6][149378 BeginBuildN 6][184463 AddedN 6][239824 CacheHitForP 6][240155 CacheHitForP 6][241942 CacheHitForN 6][333969 WaitBeginN 6][348310 WaitEndN 6]
 4:  644542265 [184850 CacheMissForN 7][184859 BeginBuildN 7][221640 AddedN 7][239834 CacheHitForP 7][240165 CacheHitForP 7][242469 CacheHitForN 7][348562 WaitBeginN 7][415779 WaitEndN 7]
 5:    2491771 [222034 CacheMissForN 8][222038 BeginBuildN 8][239842 CacheMissForP 9][239844 BeginBuildP 9][240173 CacheMissForP 10][240175 BeginBuildP 10][243035 CacheMissForN 11][243046 BeginBuildN 11][262639 AddedN 8][285629 NWonRaceOverN 8 11][285641 WaitBeginN 11][417054 WaitBeginN 8][448638 WaitEndN 11][450977 NWonRaceOverP 8 10][451892 WaitEndN 8][452654 NWonRaceOverP 8 9]
 6: 3234710487 [263016 CacheMissForN 12][263020 BeginBuildN 12][300374 AddedN 12][449098 CacheHitForN 12][451053 CacheHitForP 12][452162 WaitBeginN 12][452740 CacheHitForP 12][465285 WaitEndN 12]
 7: 1997496709 [300819 CacheMissForN 13][300825 BeginBuildN 13][348520 AddedN 13][449352 CacheHitForN 13][451066 CacheHitForP 13][452751 CacheHitForP 13][465495 WaitBeginN 13][526023 WaitEndN 13]
 8: 1301841557 [348992 CacheMissForN 14][348999 BeginBuildN 14][389181 AddedN 14][449559 CacheHitForN 14][451075 CacheHitForP 14][452760 CacheHitForP 14][526281 WaitBeginN 14][567968 WaitEndN 14]
 9: 2709710687 [389657 CacheMissForN 15][389663 BeginBuildN 15][429517 AddedN 15][449755 CacheHitForN 15][451117 CacheHitForP 15][452803 CacheHitForP 15][568276 WaitBeginN 15][599418 WaitEndN 15]
10:  138923328 [429994 CacheMissForN 16][429999 BeginBuildN 16][449932 CacheMissForN 17][449937 BeginBuildN 17][451127 CacheMissForP 18][451129 BeginBuildP 18][452813 CacheMissForP 19][452815 BeginBuildP 19][469269 AddedN 16][488832 NWonRaceOverN 16 17][488843 WaitBeginN 17][600255 WaitBeginN 16][644632 WaitEndN 16][664959 WaitEndN 17][665986 NWonRaceOverP 16 18][667345 NWonRaceOverP 16 19]
11: 2121723309 [469652 CacheMissForN 20][469658 BeginBuildN 20][507966 AddedN 20][644918 WaitBeginN 20][665508 CacheHitForN 20][666015 CacheHitForP 20][667388 CacheHitForP 20][682763 WaitEndN 20]
#endif

#if 0
Reconstructed
 0:  477900207 [ 1110 CacheMissForP 0][ 1126 CacheMissForP 1][ 1140 BeginBuildP 0][ 1140 BeginBuildP 1][36162 CacheMissForN 2][36177 CacheMissForN 3][36181 BeginBuildN 2][36182 BeginBuildN 3][75213 AddedN 3][75320 NWonRaceOverN 3 2][75332 WaitBeginN 2][76395 WaitBeginN 3][239705 NWonRaceOverP 3 0][239714 WaitEndN 3][239758 WaitEndN 2][240082 NWonRaceOverP 3 1]
 1: 2189112322 [76161 CacheMissForN 4][76168 BeginBuildN 4][111400 AddedN 4][239755 CacheHitForP 4][240104 CacheHitForP 4][240639 WaitBeginN 4][240654 CacheHitForN 4][289828 WaitEndN 4]
 2: 1147256140 [111786 CacheMissForN 5][111792 BeginBuildN 5][148901 AddedN 5][239767 CacheHitForP 5][240114 CacheHitForP 5][241322 CacheHitForN 5][290080 WaitBeginN 5][333063 WaitEndN 5]
 3:  883660955 [149369 CacheMissForN 6][149378 BeginBuildN 6][184464 AddedN 6][239824 CacheHitForP 6][240155 CacheHitForP 6][241942 CacheHitForN 6][333969 WaitBeginN 6][348311 WaitEndN 6]
 4:  644542265 [184851 CacheMissForN 7][184859 BeginBuildN 7][221640 AddedN 7][239834 CacheHitForP 7][240165 CacheHitForP 7][242469 CacheHitForN 7][348562 WaitBeginN 7][415779 WaitEndN 7]
 5:    2491771 [222035 CacheMissForN 8][222039 BeginBuildN 8][239842 CacheMissForP 9][239844 BeginBuildP 9][240173 CacheMissForP 10][240175 BeginBuildP 10][243035 CacheMissForN 11][243046 BeginBuildN 11][262639 AddedN 8][285629 NWonRaceOverN 8 11][285641 WaitBeginN 11][417054 WaitBeginN 8][448638 WaitEndN 11][450977 NWonRaceOverP 8 10][451892 WaitEndN 8][452655 NWonRaceOverP 8 9]
 6: 3234710487 [263016 CacheMissForN 12][263020 BeginBuildN 12][300374 AddedN 12][449098 CacheHitForN 12][451053 CacheHitForP 12][452162 WaitBeginN 12][452740 CacheHitForP 12][465285 WaitEndN 12]
 7: 1997496709 [300819 CacheMissForN 13][300825 BeginBuildN 13][348520 AddedN 13][449352 CacheHitForN 13][451066 CacheHitForP 13][452751 CacheHitForP 13][465495 WaitBeginN 13][526023 WaitEndN 13]
 8: 1301841557 [348993 CacheMissForN 14][348999 BeginBuildN 14][389181 AddedN 14][449559 CacheHitForN 14][451075 CacheHitForP 14][452760 CacheHitForP 14][526281 WaitBeginN 14][567968 WaitEndN 14]
 9: 2709710687 [389657 CacheMissForN 15][389663 BeginBuildN 15][429517 AddedN 15][449755 CacheHitForN 15][451117 CacheHitForP 15][452803 CacheHitForP 15][568276 WaitBeginN 15][599418 WaitEndN 15]
10:  138923328 [429994 CacheMissForN 16][429999 BeginBuildN 16][449932 CacheMissForN 17][449937 BeginBuildN 17][451127 CacheMissForP 18][451129 BeginBuildP 18][452813 CacheMissForP 19][452815 BeginBuildP 19][469269 AddedN 16][488832 NWonRaceOverN 16 17][488844 WaitBeginN 17][600255 WaitBeginN 16][644632 WaitEndN 16][664959 WaitEndN 17][665986 NWonRaceOverP 16 18][667345 NWonRaceOverP 16 19]
11: 2121723309 [469652 CacheMissForN 20][469658 BeginBuildN 20][507966 AddedN 20][644918 WaitBeginN 20][665508 CacheHitForN 20][666015 CacheHitForP 20][667388 CacheHitForP 20][682763 WaitEndN 20]
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

        for (int i = 0; i < 12; ++i) {
            std::vector<foo> log;

            bool first = true;
            SkDebugf("%2d: ", i);
            for (auto& b : bar) {
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
            //   add time of addition to FSM
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
