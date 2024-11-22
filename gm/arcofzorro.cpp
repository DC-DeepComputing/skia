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
        { 0, 1135, "CacheMissForP", 477900207, -1, UINT32_MAX, UINT32_MAX },
        { 0, 1152, "CacheMissForP", 477900207, -1, UINT32_MAX, UINT32_MAX },
        { 0, 1164, "BeginBuildP", 477900207, -1, UINT32_MAX, UINT32_MAX },
        { 0, 1164, "BeginBuildP", 477900207, -1, UINT32_MAX, UINT32_MAX },
        { 0, 36055, "CacheMissForN", 477900207, -1, UINT32_MAX, UINT32_MAX },
        { 0, 36066, "CacheMissForN", 477900207, -1, UINT32_MAX, UINT32_MAX },
        { 0, 36070, "BeginBuildN", 477900207, -1, UINT32_MAX, UINT32_MAX },
        { 0, 36070, "BeginBuildN", 477900207, -1, UINT32_MAX, UINT32_MAX },
        { 0, 75326, "AddedN", 477900207, -1, 0, UINT32_MAX },
        { 0, 75359, "NWonRaceOverN", 477900207, -1, 0, 1 },
        { 0, 75364, "WaitBeginN", 477900207, -1, 1, UINT32_MAX },
        { 1, 76189, "CacheMissForN", 2189112322, -1, UINT32_MAX, UINT32_MAX },
        { 1, 76194, "BeginBuildN", 2189112322, -1, UINT32_MAX, UINT32_MAX },
        { 0, 76450, "WaitBeginN", 477900207, -1, 0, UINT32_MAX },
        { 1, 111822, "AddedN", 2189112322, -1, 2, UINT32_MAX },
        { 2, 112267, "CacheMissForN", 1147256140, -1, UINT32_MAX, UINT32_MAX },
        { 2, 112272, "BeginBuildN", 1147256140, -1, UINT32_MAX, UINT32_MAX },
        { 2, 148481, "AddedN", 1147256140, -1, 3, UINT32_MAX },
        { 3, 149043, "CacheMissForN", 883660955, -1, UINT32_MAX, UINT32_MAX },
        { 3, 149048, "BeginBuildN", 883660955, -1, UINT32_MAX, UINT32_MAX },
        { 3, 184837, "AddedN", 883660955, -1, 4, UINT32_MAX },
        { 4, 187122, "CacheMissForN", 644542265, -1, UINT32_MAX, UINT32_MAX },
        { 4, 187142, "BeginBuildN", 644542265, -1, UINT32_MAX, UINT32_MAX },
        { 4, 227404, "AddedN", 644542265, -1, 5, UINT32_MAX },
        { 5, 227895, "CacheMissForN", 2491771, -1, UINT32_MAX, UINT32_MAX },
        { 5, 227899, "BeginBuildN", 2491771, -1, UINT32_MAX, UINT32_MAX },
        { 0, 239235, "NWonRaceOverP", 477900207, -1, 0, 6 },
        { 0, 239256, "WaitEndN", 477900207, -1, 1, UINT32_MAX },
        { 1, 239281, "CacheHitForP", 2189112322, -1, 2, UINT32_MAX },
        { 2, 239292, "CacheHitForP", 1147256140, -1, 3, UINT32_MAX },
        { 0, 239313, "WaitEndN", 477900207, -1, 0, UINT32_MAX },
        { 3, 239352, "CacheHitForP", 883660955, -1, 4, UINT32_MAX },
        { 4, 239363, "CacheHitForP", 644542265, -1, 5, UINT32_MAX },
        { 5, 239372, "CacheMissForP", 2491771, -1, UINT32_MAX, UINT32_MAX },
        { 5, 239374, "BeginBuildP", 2491771, -1, UINT32_MAX, UINT32_MAX },
        { 1, 239652, "CacheHitForN", 2189112322, -1, 2, UINT32_MAX },
        { 0, 239826, "NWonRaceOverP", 477900207, -1, 0, 7 },
        { 2, 239851, "CacheHitForN", 1147256140, -1, 3, UINT32_MAX },
        { 1, 239875, "CacheHitForP", 2189112322, -1, 2, UINT32_MAX },
        { 2, 239887, "CacheHitForP", 1147256140, -1, 3, UINT32_MAX },
        { 3, 239946, "CacheHitForP", 883660955, -1, 4, UINT32_MAX },
        { 4, 239957, "CacheHitForP", 644542265, -1, 5, UINT32_MAX },
        { 5, 239966, "CacheMissForP", 2491771, -1, UINT32_MAX, UINT32_MAX },
        { 1, 240419, "WaitBeginN", 2189112322, -1, 2, UINT32_MAX },
        { 5, 240419, "BeginBuildP", 2491771, -1, UINT32_MAX, UINT32_MAX },
        { 3, 240420, "CacheHitForN", 883660955, -1, 4, UINT32_MAX },
        { 4, 240643, "CacheHitForN", 644542265, -1, 5, UINT32_MAX },
        { 5, 240825, "CacheMissForN", 2491771, -1, UINT32_MAX, UINT32_MAX },
        { 5, 240831, "BeginBuildN", 2491771, -1, UINT32_MAX, UINT32_MAX },
        { 5, 266097, "AddedN", 2491771, -1, 8, UINT32_MAX },
        { 6, 266555, "CacheMissForN", 3234710487, -1, UINT32_MAX, UINT32_MAX },
        { 6, 266559, "BeginBuildN", 3234710487, -1, UINT32_MAX, UINT32_MAX },
        { 5, 286281, "NWonRaceOverN", 2491771, -1, 8, 9 },
        { 5, 286292, "WaitBeginN", 2491771, -1, 9, UINT32_MAX },
        { 1, 286788, "WaitEndN", 2189112322, -1, 2, UINT32_MAX },
        { 2, 287048, "WaitBeginN", 1147256140, -1, 3, UINT32_MAX },
        { 6, 307757, "AddedN", 3234710487, -1, 10, UINT32_MAX },
        { 7, 308221, "CacheMissForN", 1997496709, -1, UINT32_MAX, UINT32_MAX },
        { 7, 308227, "BeginBuildN", 1997496709, -1, UINT32_MAX, UINT32_MAX },
        { 2, 319846, "WaitEndN", 1147256140, -1, 3, UINT32_MAX },
        { 3, 320213, "WaitBeginN", 883660955, -1, 4, UINT32_MAX },
        { 7, 351483, "AddedN", 1997496709, -1, 11, UINT32_MAX },
        { 8, 351982, "CacheMissForN", 1301841557, -1, UINT32_MAX, UINT32_MAX },
        { 8, 351987, "BeginBuildN", 1301841557, -1, UINT32_MAX, UINT32_MAX },
        { 3, 366426, "WaitEndN", 883660955, -1, 4, UINT32_MAX },
        { 4, 366851, "WaitBeginN", 644542265, -1, 5, UINT32_MAX },
        { 8, 394513, "AddedN", 1301841557, -1, 12, UINT32_MAX },
        { 9, 395019, "CacheMissForN", 2709710687, -1, UINT32_MAX, UINT32_MAX },
        { 9, 395025, "BeginBuildN", 2709710687, -1, UINT32_MAX, UINT32_MAX },
        { 9, 433982, "AddedN", 2709710687, -1, 13, UINT32_MAX },
        { 10, 434482, "CacheMissForN", 138923328, -1, UINT32_MAX, UINT32_MAX },
        { 10, 434486, "BeginBuildN", 138923328, -1, UINT32_MAX, UINT32_MAX },
        { 5, 441398, "NWonRaceOverP", 2491771, -1, 8, 14 },
        { 6, 441508, "CacheHitForP", 3234710487, -1, 10, UINT32_MAX },
        { 7, 441519, "CacheHitForP", 1997496709, -1, 11, UINT32_MAX },
        { 8, 441528, "CacheHitForP", 1301841557, -1, 12, UINT32_MAX },
        { 9, 441571, "CacheHitForP", 2709710687, -1, 13, UINT32_MAX },
        { 10, 441583, "CacheMissForP", 138923328, -1, UINT32_MAX, UINT32_MAX },
        { 10, 441585, "BeginBuildP", 138923328, -1, UINT32_MAX, UINT32_MAX },
        { 4, 453286, "WaitEndN", 644542265, -1, 5, UINT32_MAX },
        { 5, 455410, "WaitBeginN", 2491771, -1, 8, UINT32_MAX },
        { 5, 455505, "WaitEndN", 2491771, -1, 8, UINT32_MAX },
        { 6, 455816, "WaitBeginN", 3234710487, -1, 10, UINT32_MAX },
        { 5, 458603, "WaitEndN", 2491771, -1, 9, UINT32_MAX },
        { 6, 459068, "CacheHitForN", 3234710487, -1, 10, UINT32_MAX },
        { 7, 459309, "CacheHitForN", 1997496709, -1, 11, UINT32_MAX },
        { 8, 459516, "CacheHitForN", 1301841557, -1, 12, UINT32_MAX },
        { 9, 459713, "CacheHitForN", 2709710687, -1, 13, UINT32_MAX },
        { 10, 459895, "CacheMissForN", 138923328, -1, UINT32_MAX, UINT32_MAX },
        { 10, 459900, "BeginBuildN", 138923328, -1, UINT32_MAX, UINT32_MAX },
        { 6, 471279, "WaitEndN", 3234710487, -1, 10, UINT32_MAX },
        { 7, 471502, "WaitBeginN", 1997496709, -1, 11, UINT32_MAX },
        { 10, 473565, "AddedN", 138923328, -1, 15, UINT32_MAX },
        { 11, 473929, "CacheMissForN", 2121723309, -1, UINT32_MAX, UINT32_MAX },
        { 11, 473935, "BeginBuildN", 2121723309, -1, UINT32_MAX, UINT32_MAX },
        { 5, 475234, "NWonRaceOverP", 2491771, -1, 8, 16 },
        { 6, 475339, "CacheHitForP", 3234710487, -1, 10, UINT32_MAX },
        { 7, 475353, "CacheHitForP", 1997496709, -1, 11, UINT32_MAX },
        { 8, 475362, "CacheHitForP", 1301841557, -1, 12, UINT32_MAX },
        { 9, 475409, "CacheHitForP", 2709710687, -1, 13, UINT32_MAX },
        { 10, 475420, "CacheHitForP", 138923328, -1, 15, UINT32_MAX },
        { 11, 475428, "CacheMissForP", 2121723309, -1, UINT32_MAX, UINT32_MAX },
        { 11, 475430, "BeginBuildP", 2121723309, -1, UINT32_MAX, UINT32_MAX },
        { 10, 498969, "NWonRaceOverN", 138923328, -1, 15, 17 },
        { 10, 498980, "WaitBeginN", 138923328, -1, 17, UINT32_MAX },
        { 11, 512037, "AddedN", 2121723309, -1, 18, UINT32_MAX },
        { 7, 538579, "WaitEndN", 1997496709, -1, 11, UINT32_MAX },
        { 8, 538985, "WaitBeginN", 1301841557, -1, 12, UINT32_MAX },
        { 8, 567807, "WaitEndN", 1301841557, -1, 12, UINT32_MAX },
        { 9, 568104, "WaitBeginN", 2709710687, -1, 13, UINT32_MAX },
        { 9, 607931, "WaitEndN", 2709710687, -1, 13, UINT32_MAX },
        { 10, 608754, "WaitBeginN", 138923328, -1, 15, UINT32_MAX },
        { 10, 652065, "WaitEndN", 138923328, -1, 15, UINT32_MAX },
        { 11, 652425, "WaitBeginN", 2121723309, -1, 18, UINT32_MAX },
        { 10, 664307, "NWonRaceOverP", 138923328, -1, 15, 19 },
        { 11, 664359, "CacheHitForP", 2121723309, -1, 18, UINT32_MAX },
        { 10, 675521, "WaitEndN", 138923328, -1, 17, UINT32_MAX },
        { 11, 676073, "CacheHitForN", 2121723309, -1, 18, UINT32_MAX },
        { 11, 686115, "WaitEndN", 2121723309, -1, 18, UINT32_MAX },
        { 11, 686792, "NWonRaceOverP", 2121723309, -1, 18, 20 },
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
};

#if 0
Original
 0:  477900207 [ 1135 CacheMissForP][ 1152 CacheMissForP][ 1163 BeginBuildP][ 1164 BeginBuildP][36055 CacheMissForN][36066 CacheMissForN][36070 BeginBuildN][36070 BeginBuildN][75326 AddedN 0][75359 NWonRaceOverN 0 1][75364 WaitBeginN 1][76450 WaitBeginN 0][239235 NWonRaceOverP 0 6][239256 WaitEndN 1][239312 WaitEndN 0][239826 NWonRaceOverP 0 7]
 1: 2189112322 [76189 CacheMissForN][76194 BeginBuildN][111822 AddedN 2][239281 CacheHitForP 2][239652 CacheHitForN 2][239875 CacheHitForP 2][240419 WaitBeginN 2][286788 WaitEndN 2]
 2: 1147256140 [112267 CacheMissForN][112272 BeginBuildN][148481 AddedN 3][239292 CacheHitForP 3][239851 CacheHitForN 3][239887 CacheHitForP 3][287048 WaitBeginN 3][319846 WaitEndN 3]
 3:  883660955 [149043 CacheMissForN][149048 BeginBuildN][184837 AddedN 4][239352 CacheHitForP 4][239946 CacheHitForP 4][240420 CacheHitForN 4][320212 WaitBeginN 4][366426 WaitEndN 4]
 4:  644542265 [187122 CacheMissForN][187142 BeginBuildN][227404 AddedN 5][239363 CacheHitForP 5][239956 CacheHitForP 5][240643 CacheHitForN 5][366851 WaitBeginN 5][453286 WaitEndN 5]
 5:    2491771 [227895 CacheMissForN][227899 BeginBuildN][239372 CacheMissForP][239374 BeginBuildP][239965 CacheMissForP][240419 BeginBuildP][240825 CacheMissForN][240831 BeginBuildN][266097 AddedN 8][286281 NWonRaceOverN 8 9][286292 WaitBeginN 9][441397 NWonRaceOverP 8 14][455410 WaitBeginN 8][455505 WaitEndN 8][458603 WaitEndN 9][475234 NWonRaceOverP 8 16]
 6: 3234710487 [266555 CacheMissForN][266559 BeginBuildN][307757 AddedN 10][441508 CacheHitForP 10][455815 WaitBeginN 10][459068 CacheHitForN 10][471279 WaitEndN 10][475339 CacheHitForP 10]
 7: 1997496709 [308221 CacheMissForN][308227 BeginBuildN][351483 AddedN 11][441519 CacheHitForP 11][459309 CacheHitForN 11][471502 WaitBeginN 11][475353 CacheHitForP 11][538578 WaitEndN 11]
 8: 1301841557 [351981 CacheMissForN][351987 BeginBuildN][394513 AddedN 12][441528 CacheHitForP 12][459516 CacheHitForN 12][475362 CacheHitForP 12][538985 WaitBeginN 12][567807 WaitEndN 12]
 9: 2709710687 [395019 CacheMissForN][395025 BeginBuildN][433982 AddedN 13][441571 CacheHitForP 13][459713 CacheHitForN 13][475409 CacheHitForP 13][568104 WaitBeginN 13][607931 WaitEndN 13]
10:  138923328 [434482 CacheMissForN][434486 BeginBuildN][441583 CacheMissForP][441585 BeginBuildP][459895 CacheMissForN][459900 BeginBuildN][473565 AddedN 15][475420 CacheHitForP 15][498969 NWonRaceOverN 15 17][498980 WaitBeginN 17][608754 WaitBeginN 15][652065 WaitEndN 15][664307 NWonRaceOverP 15 19][675521 WaitEndN 17]
11: 2121723309 [473929 CacheMissForN][473935 BeginBuildN][475428 CacheMissForP][475430 BeginBuildP][512037 AddedN 18][652425 WaitBeginN 18][664359 CacheHitForP 18][676073 CacheHitForN 18][686115 WaitEndN 18][686792 NWonRaceOverP 18 20]
#endif

#if 0
Reconstructed
 0:  477900207 [ 1135 CacheMissForP][ 1152 CacheMissForP][ 1164 BeginBuildP][ 1164 BeginBuildP][36055 CacheMissForN][36066 CacheMissForN][36070 BeginBuildN][36070 BeginBuildN][75326 AddedN 0][75359 NWonRaceOverN 0 1][75364 WaitBeginN 1][76450 WaitBeginN 0][239235 NWonRaceOverP 0 6][239256 WaitEndN 1][239313 WaitEndN 0][239826 NWonRaceOverP 0 7]
 1: 2189112322 [76189 CacheMissForN][76194 BeginBuildN][111822 AddedN 2][239281 CacheHitForP 2][239652 CacheHitForN 2][239875 CacheHitForP 2][240419 WaitBeginN 2][286788 WaitEndN 2]
 2: 1147256140 [112267 CacheMissForN][112272 BeginBuildN][148481 AddedN 3][239292 CacheHitForP 3][239851 CacheHitForN 3][239887 CacheHitForP 3][287048 WaitBeginN 3][319846 WaitEndN 3]
 3:  883660955 [149043 CacheMissForN][149048 BeginBuildN][184837 AddedN 4][239352 CacheHitForP 4][239946 CacheHitForP 4][240420 CacheHitForN 4][320213 WaitBeginN 4][366426 WaitEndN 4]
 4:  644542265 [187122 CacheMissForN][187142 BeginBuildN][227404 AddedN 5][239363 CacheHitForP 5][239957 CacheHitForP 5][240643 CacheHitForN 5][366851 WaitBeginN 5][453286 WaitEndN 5]
 5:    2491771 [227895 CacheMissForN][227899 BeginBuildN][239372 CacheMissForP][239374 BeginBuildP][239966 CacheMissForP][240419 BeginBuildP][240825 CacheMissForN][240831 BeginBuildN][266097 AddedN 8][286281 NWonRaceOverN 8 9][286292 WaitBeginN 9][441398 NWonRaceOverP 8 14][455410 WaitBeginN 8][455505 WaitEndN 8][458603 WaitEndN 9][475234 NWonRaceOverP 8 16]
 6: 3234710487 [266555 CacheMissForN][266559 BeginBuildN][307757 AddedN 10][441508 CacheHitForP 10][455816 WaitBeginN 10][459068 CacheHitForN 10][471279 WaitEndN 10][475339 CacheHitForP 10]
 7: 1997496709 [308221 CacheMissForN][308227 BeginBuildN][351483 AddedN 11][441519 CacheHitForP 11][459309 CacheHitForN 11][471502 WaitBeginN 11][475353 CacheHitForP 11][538579 WaitEndN 11]
 8: 1301841557 [351982 CacheMissForN][351987 BeginBuildN][394513 AddedN 12][441528 CacheHitForP 12][459516 CacheHitForN 12][475362 CacheHitForP 12][538985 WaitBeginN 12][567807 WaitEndN 12]
 9: 2709710687 [395019 CacheMissForN][395025 BeginBuildN][433982 AddedN 13][441571 CacheHitForP 13][459713 CacheHitForN 13][475409 CacheHitForP 13][568104 WaitBeginN 13][607931 WaitEndN 13]
10:  138923328 [434482 CacheMissForN][434486 BeginBuildN][441583 CacheMissForP][441585 BeginBuildP][459895 CacheMissForN][459900 BeginBuildN][473565 AddedN 15][475420 CacheHitForP 15][498969 NWonRaceOverN 15 17][498980 WaitBeginN 17][608754 WaitBeginN 15][652065 WaitEndN 15][664307 NWonRaceOverP 15 19][675521 WaitEndN 17]
11: 2121723309 [473929 CacheMissForN][473935 BeginBuildN][475428 CacheMissForP][475430 BeginBuildP][512037 AddedN 18][652425 WaitBeginN 18][664359 CacheHitForP 18][676073 CacheHitForN 18][686115 WaitEndN 18][686792 NWonRaceOverP 18 20]
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

            FSMNode fsm[3];

            int64_t waitStart = 0;
            for (auto l : log) {
                if (!strcmp(l.fTag, "CacheMissForP") ||
                    !strcmp(l.fTag, "CacheMissForN")) {
                    fsm[Node::kMiss].addToState(l.fPipelineID1);
                }
                if (!strcmp(l.fTag, "WaitBeginN")) {
                    waitStart = l.fTime;
                } else if (!strcmp(l.fTag, "WaitEndN")) {
                    SkASSERT(waitStart != 0);
                    int64_t diff = l.fTime - waitStart;
                    SkDebugf("Bad wait %lld\n", diff);
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
