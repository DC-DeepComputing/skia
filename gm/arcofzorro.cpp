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

static const struct {
    uint64_t fTime;
    const char* fTag;
    uint32_t fID;
    int fExtra;
} bar[] = {
        { 88414143642, "CacheMissForN", 2002950273, -1 },
        { 88414182555, "AddedN", 2002950273, -1 },
        { 456, "CacheMissForP", 1251730206, -1 },
        { 460, "CacheMissForP", 1251730206, -1 },
        { 5106, "AddedP", 1251730206, -1 },
        { 5114, "PWonRaceOverP", 1251730206, -1 },
        { 5127, "CacheMissForP", 3948671900, -1 },
        { 5132, "CacheMissForP", 3948671900, -1 },
        { 9298, "AddedP", 3948671900, -1 },
        { 9305, "PWonRaceOverP", 3948671900, -1 },
        { 9316, "CacheMissForP", 2742715100, -1 },
        { 9320, "CacheMissForP", 2742715100, -1 },
        { 13750, "AddedP", 2742715100, -1 },
        { 13754, "PWonRaceOverP", 2742715100, -1 },
        { 13838, "CacheMissForP", 2646413713, -1 },
        { 13843, "CacheMissForP", 2646413713, -1 },
        { 17990, "AddedP", 2646413713, -1 },
        { 17999, "PWonRaceOverP", 2646413713, -1 },
        { 18010, "CacheMissForP", 2326987662, -1 },
        { 18015, "CacheMissForP", 2326987662, -1 },
        { 22260, "AddedP", 2326987662, -1 },
        { 22267, "PWonRaceOverP", 2326987662, -1 },
        { 22278, "CacheMissForP", 339553235, -1 },
        { 22284, "CacheMissForP", 339553235, -1 },
        { 26428, "AddedP", 339553235, -1 },
        { 26432, "PWonRaceOverP", 339553235, -1 },
        { 26482, "CacheMissForP", 1250731221, -1 },
        { 26485, "CacheMissForP", 1250731221, -1 },
        { 30572, "AddedP", 1250731221, -1 },
        { 30581, "PWonRaceOverP", 1250731221, -1 },
        { 30592, "CacheMissForP", 1104035737, -1 },
        { 30595, "CacheMissForP", 1104035737, -1 },
        { 35163, "AddedP", 1104035737, -1 },
        { 35171, "PWonRaceOverP", 1104035737, -1 },
        { 35186, "CacheMissForP", 1361935096, -1 },
        { 35191, "CacheMissForP", 1361935096, -1 },
        { 35455, "CacheHitForN", 1251730206, -1 },
        { 35521, "CacheHitForN", 1251730206, -1 },
        { 36149, "CacheHitForN", 3948671900, -1 },
        { 36174, "CacheHitForN", 3948671900, -1 },
        { 36388, "CacheHitForN", 2742715100, -1 },
        { 36454, "CacheHitForN", 2742715100, -1 },
        { 36545, "CacheHitForN", 2646413713, -1 },
        { 36662, "CacheHitForN", 2646413713, -1 },
        { 36715, "CacheHitForN", 2326987662, -1 },
        { 36841, "CacheHitForN", 2326987662, -1 },
        { 36893, "CacheHitForN", 339553235, -1 },
        { 37039, "CacheHitForN", 339553235, -1 },
        { 37069, "CacheHitForN", 1250731221, -1 },
        { 37213, "CacheHitForN", 1250731221, -1 },
        { 37261, "CacheHitForN", 1104035737, -1 },
        { 37413, "CacheHitForN", 1104035737, -1 },
        { 37442, "CacheMissForN", 1361935096, -1 },
        { 37587, "CacheMissForN", 1361935096, -1 },
        { 39766, "AddedP", 1361935096, -1 },
        { 39770, "PWonRaceOverP", 1361935096, -1 },
        { 39823, "CacheMissForP", 3219415637, -1 },
        { 39825, "CacheMissForP", 3219415637, -1 },
        { 41934, "PWonRaceOverN", 1361935096, -1 },
        { 41943, "PWonRaceOverN", 1361935096, -1 },
        { 42188, "CacheMissForN", 3219415637, -1 },
        { 42195, "CacheMissForN", 3219415637, -1 },
        { 44126, "AddedP", 3219415637, -1 },
        { 44130, "PWonRaceOverP", 3219415637, -1 },
        { 44143, "CacheMissForP", 3296533686, -1 },
        { 44149, "CacheMissForP", 3296533686, -1 },
        { 46408, "PWonRaceOverN", 3219415637, -1 },
        { 46413, "PWonRaceOverN", 3219415637, -1 },
        { 46646, "CacheMissForN", 3296533686, -1 },
        { 46720, "CacheMissForN", 3296533686, -1 },
        { 48640, "AddedP", 3296533686, -1 },
        { 48646, "PWonRaceOverP", 3296533686, -1 },
        { 48659, "CacheMissForP", 2137548669, -1 },
        { 48661, "CacheMissForP", 2137548669, -1 },
        { 50939, "PWonRaceOverN", 3296533686, -1 },
        { 50944, "PWonRaceOverN", 3296533686, -1 },
        { 51190, "CacheMissForN", 2137548669, -1 },
        { 51200, "CacheMissForN", 2137548669, -1 },
        { 53253, "AddedP", 2137548669, -1 },
        { 53258, "PWonRaceOverP", 2137548669, -1 },
        { 55625, "PWonRaceOverN", 2137548669, -1 },
        { 55646, "PWonRaceOverN", 2137548669, -1 },
};
#if 0
        total races: 20 hits: 16 (NoN: 0 NoP: 0 PoN: 16 PoP: 0) misses: 32 (N: 8 P: 24)
        N-over-N 0
        N-over-P 0
        P-over-N 8
        P-over-P 12
        0: 2646413713 [13838 CacheMissForP][13843 CacheMissForP][17990 AddedP][17999 PWonRaceOverP][36545 CacheHitForN][36662 CacheHitForN]
        1:  339553235 [22278 CacheMissForP][22284 CacheMissForP][26428 AddedP][26432 PWonRaceOverP][36893 CacheHitForN][37039 CacheHitForN]
        2: 3219415637 [39823 CacheMissForP][39825 CacheMissForP][42188 CacheMissForN][42195 CacheMissForN][44126 AddedP][44130 PWonRaceOverP][46408 PWonRaceOverN][46413 PWonRaceOverN]
        3: 1250731221 [26482 CacheMissForP][26485 CacheMissForP][30572 AddedP][30581 PWonRaceOverP][37069 CacheHitForN][37213 CacheHitForN]
        4: 3296533686 [44143 CacheMissForP][44149 CacheMissForP][46646 CacheMissForN][46720 CacheMissForN][48640 AddedP][48646 PWonRaceOverP][50939 PWonRaceOverN][50944 PWonRaceOverN]
        5: 1361935096 [35186 CacheMissForP][35191 CacheMissForP][37442 CacheMissForN][37587 CacheMissForN][39766 AddedP][39770 PWonRaceOverP][41934 PWonRaceOverN][41943 PWonRaceOverN]
        6: 1104035737 [30592 CacheMissForP][30595 CacheMissForP][35163 AddedP][35171 PWonRaceOverP][37261 CacheHitForN][37412 CacheHitForN]
        7: 2137548669 [48659 CacheMissForP][48661 CacheMissForP][51190 CacheMissForN][51200 CacheMissForN][53253 AddedP][53257 PWonRaceOverP][55625 PWonRaceOverN][55646 PWonRaceOverN]
        8: 3948671900 [ 5126 CacheMissForP][ 5132 CacheMissForP][ 9298 AddedP][ 9305 PWonRaceOverP][36149 CacheHitForN][36174 CacheHitForN]
        9: 2742715100 [ 9316 CacheMissForP][ 9320 CacheMissForP][13750 AddedP][13754 PWonRaceOverP][36388 CacheHitForN][36454 CacheHitForN]
        10: 1251730206 [  456 CacheMissForP][  460 CacheMissForP][ 5106 AddedP][ 5114 PWonRaceOverP][35455 CacheHitForN][35521 CacheHitForN]
        11: 2326987662 [18010 CacheMissForP][18015 CacheMissForP][22260 AddedP][22267 PWonRaceOverP][36715 CacheHitForN][36841 CacheHitForN]
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
        for (auto& b : bar) {

        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
