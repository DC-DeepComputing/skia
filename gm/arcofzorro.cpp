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
//   Think more about the FSM - add more nodes
//   Shrink allowed # of pipelines and test out expanded FSM behavior
//   add time of addition to FSM - why?
//   compute compile times for the N/P cases - create mean & std dev.
//   resolve Bad Wait mystery - how can two Dawn pipelines be waiting!!
//   maybe add a deleted state?
//     - I think we can get this from State A below and the purge state
//   Add purge state handling and testing!
//   Compute the bad wait times - create mean and std dev
//   Add tracking of 'N' and 'P' to pipelines in FSM and add asserts
//   Replace XWonRaceOverY messages w/ just LostRaceY messages
//         - only need one xtra field then

static const struct LoggingOutput {
    int fID;
    uint64_t fTime;
    const char* fTag;
    uint32_t fHash;
    int fExtra;
    uint32_t fPipelineID1;
    uint32_t fPipelineID2;
} kOutputFromRealLogging[] = {
        { 0, 330, "CacheMissForP", 477900207, -1, 0, UINT32_MAX },
        { 0, 345, "CacheMissForP", 477900207, -1, 1, UINT32_MAX },
        { 0, 349, "BeginBuildP", 477900207, -1, 0, UINT32_MAX },
        { 0, 349, "BeginBuildP", 477900207, -1, 1, UINT32_MAX },
        { 0, 2738, "CacheMissForN", 477900207, -1, 2, UINT32_MAX },
        { 0, 2749, "CacheMissForN", 477900207, -1, 3, UINT32_MAX },
        { 0, 2752, "BeginBuildN", 477900207, -1, 2, UINT32_MAX },
        { 0, 2752, "BeginBuildN", 477900207, -1, 3, UINT32_MAX },
        { 0, 8665, "AddedN", 477900207, -1, 3, UINT32_MAX },
        { 0, 8729, "NWonRaceOverN", 477900207, -1, 3, 2 },
        { 0, 8736, "WaitBeginN", 477900207, -1, 2, UINT32_MAX },
        { 1, 9064, "CacheMissForN", 2189112322, -1, 4, UINT32_MAX },
        { 1, 9069, "BeginBuildN", 2189112322, -1, 4, UINT32_MAX },
        { 0, 9363, "WaitBeginN", 477900207, -1, 3, UINT32_MAX },
        { 1, 11989, "AddedN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 12115, "CacheMissForN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 12120, "BeginBuildN", 1147256140, -1, 5, UINT32_MAX },
        { 0, 15194, "BadPurgedN", 477900207, -1, 3, UINT32_MAX },
        { 2, 15204, "AddedN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 15355, "CacheMissForN", 883660955, -1, 6, UINT32_MAX },
        { 3, 15359, "BeginBuildN", 883660955, -1, 6, UINT32_MAX },
        { 1, 18300, "BadPurgedN", 2189112322, -1, 4, UINT32_MAX },
        { 3, 18308, "AddedN", 883660955, -1, 6, UINT32_MAX },
        { 4, 18422, "CacheMissForN", 644542265, -1, 7, UINT32_MAX },
        { 4, 18425, "BeginBuildN", 644542265, -1, 7, UINT32_MAX },
        { 2, 21331, "BadPurgedN", 1147256140, -1, 5, UINT32_MAX },
        { 4, 21339, "AddedN", 644542265, -1, 7, UINT32_MAX },
        { 5, 21467, "CacheMissForN", 2491771, -1, 8, UINT32_MAX },
        { 5, 21469, "BeginBuildN", 2491771, -1, 8, UINT32_MAX },
        { 3, 24416, "BadPurgedN", 883660955, -1, 6, UINT32_MAX },
        { 5, 24423, "AddedN", 2491771, -1, 8, UINT32_MAX },
        { 6, 24540, "CacheMissForN", 3234710487, -1, 9, UINT32_MAX },
        { 6, 24542, "BeginBuildN", 3234710487, -1, 9, UINT32_MAX },
        { 4, 27722, "BadPurgedN", 644542265, -1, 7, UINT32_MAX },
        { 6, 27733, "AddedN", 3234710487, -1, 9, UINT32_MAX },
        { 7, 27888, "CacheMissForN", 1997496709, -1, 10, UINT32_MAX },
        { 7, 27890, "BeginBuildN", 1997496709, -1, 10, UINT32_MAX },
        { 0, 28681, "WaitEndN", 477900207, -1, 2, UINT32_MAX },
        { 0, 28709, "DeletionN", 477900207, -1, 2, UINT32_MAX },
        { 1, 28822, "CacheMissForN", 2189112322, -1, 11, UINT32_MAX },
        { 1, 28824, "BeginBuildN", 2189112322, -1, 11, UINT32_MAX },
        { 0, 30491, "WaitEndN", 477900207, -1, 3, UINT32_MAX },
        { 1, 30603, "WaitBeginN", 2189112322, -1, 4, UINT32_MAX },
        { 5, 30859, "BadPurgedN", 2491771, -1, 8, UINT32_MAX },
        { 7, 30866, "AddedN", 1997496709, -1, 10, UINT32_MAX },
        { 6, 30869, "BadPurgedN", 3234710487, -1, 9, UINT32_MAX },
        { 0, 30873, "AddedP", 477900207, -1, 0, UINT32_MAX },
        { 1, 30882, "CacheMissForP", 2189112322, -1, 12, UINT32_MAX },
        { 1, 30883, "BeginBuildP", 2189112322, -1, 12, UINT32_MAX },
        { 8, 30978, "CacheMissForN", 1301841557, -1, 13, UINT32_MAX },
        { 8, 30980, "BeginBuildN", 1301841557, -1, 13, UINT32_MAX },
        { 0, 32246, "PWonRaceOverP", 477900207, -1, 0, 1 },
        { 0, 32256, "DeletionP", 477900207, -1, 1, UINT32_MAX },
        { 1, 32264, "CacheMissForP", 2189112322, -1, 14, UINT32_MAX },
        { 1, 32265, "BeginBuildP", 2189112322, -1, 14, UINT32_MAX },
        { 7, 33860, "BadPurgedN", 1997496709, -1, 10, UINT32_MAX },
        { 8, 33866, "AddedN", 1301841557, -1, 13, UINT32_MAX },
        { 9, 33984, "CacheMissForN", 2709710687, -1, 15, UINT32_MAX },
        { 9, 33986, "BeginBuildN", 2709710687, -1, 15, UINT32_MAX },
        { 1, 34809, "WaitEndN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 34860, "WaitBeginN", 1147256140, -1, 5, UINT32_MAX },
        { 0, 35590, "BadPurgedP", 477900207, -1, 0, UINT32_MAX },
        { 0, 35593, "DeletionP", 477900207, -1, 0, UINT32_MAX },
        { 1, 35595, "AddedN", 2189112322, -1, 11, UINT32_MAX },
        { 2, 35714, "CacheMissForN", 1147256140, -1, 16, UINT32_MAX },
        { 2, 35716, "BeginBuildN", 1147256140, -1, 16, UINT32_MAX },
        { 1, 36450, "NWonRaceOverP", 2189112322, -1, 11, 14 },
        { 1, 36456, "DeletionP", 2189112322, -1, 14, UINT32_MAX },
        { 2, 36461, "CacheMissForP", 1147256140, -1, 17, UINT32_MAX },
        { 2, 36462, "BeginBuildP", 1147256140, -1, 17, UINT32_MAX },
        { 8, 37078, "BadPurgedN", 1301841557, -1, 13, UINT32_MAX },
        { 9, 37086, "AddedN", 2709710687, -1, 15, UINT32_MAX },
        { 10, 37223, "CacheMissForN", 138923328, -1, 18, UINT32_MAX },
        { 10, 37226, "BeginBuildN", 138923328, -1, 18, UINT32_MAX },
        { 1, 38535, "NWonRaceOverP", 2189112322, -1, 11, 12 },
        { 1, 38556, "DeletionP", 2189112322, -1, 12, UINT32_MAX },
        { 2, 38571, "CacheMissForP", 1147256140, -1, 19, UINT32_MAX },
        { 2, 38575, "BeginBuildP", 1147256140, -1, 19, UINT32_MAX },
        { 9, 38662, "BadPurgedN", 2709710687, -1, 15, UINT32_MAX },
        { 2, 38668, "AddedN", 1147256140, -1, 16, UINT32_MAX },
        { 3, 38782, "CacheMissForN", 883660955, -1, 20, UINT32_MAX },
        { 3, 38784, "BeginBuildN", 883660955, -1, 20, UINT32_MAX },
        { 1, 43208, "BadPurgedN", 2189112322, -1, 11, UINT32_MAX },
        { 3, 43215, "AddedN", 883660955, -1, 20, UINT32_MAX },
        { 4, 43337, "CacheMissForN", 644542265, -1, 21, UINT32_MAX },
        { 4, 43339, "BeginBuildN", 644542265, -1, 21, UINT32_MAX },
        { 2, 44109, "WaitEndN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 44160, "WaitBeginN", 883660955, -1, 6, UINT32_MAX },
        { 3, 44170, "WaitEndN", 883660955, -1, 6, UINT32_MAX },
        { 4, 44181, "WaitBeginN", 644542265, -1, 7, UINT32_MAX },
        { 2, 44331, "BadPurgedN", 1147256140, -1, 16, UINT32_MAX },
        { 10, 44349, "AddedN", 138923328, -1, 18, UINT32_MAX },
        { 11, 44724, "CacheMissForN", 2121723309, -1, 22, UINT32_MAX },
        { 11, 44730, "BeginBuildN", 2121723309, -1, 22, UINT32_MAX },
        { 4, 46061, "WaitEndN", 644542265, -1, 7, UINT32_MAX },
        { 5, 46101, "WaitBeginN", 2491771, -1, 8, UINT32_MAX },
        { 5, 46112, "WaitEndN", 2491771, -1, 8, UINT32_MAX },
        { 6, 46121, "WaitBeginN", 3234710487, -1, 9, UINT32_MAX },
        { 3, 46272, "BadPurgedN", 883660955, -1, 20, UINT32_MAX },
        { 4, 46277, "AddedN", 644542265, -1, 21, UINT32_MAX },
        { 5, 46389, "CacheMissForN", 2491771, -1, 23, UINT32_MAX },
        { 5, 46391, "BeginBuildN", 2491771, -1, 23, UINT32_MAX },
        { 10, 49333, "BadPurgedN", 138923328, -1, 18, UINT32_MAX },
        { 5, 49342, "AddedN", 2491771, -1, 23, UINT32_MAX },
        { 6, 49476, "CacheMissForN", 3234710487, -1, 24, UINT32_MAX },
        { 6, 49478, "BeginBuildN", 3234710487, -1, 24, UINT32_MAX },
        { 4, 49753, "BadPurgedN", 644542265, -1, 21, UINT32_MAX },
        { 11, 49766, "AddedN", 2121723309, -1, 22, UINT32_MAX },
        { 5, 54834, "BadPurgedN", 2491771, -1, 23, UINT32_MAX },
        { 6, 54843, "AddedN", 3234710487, -1, 24, UINT32_MAX },
        { 7, 54965, "CacheMissForN", 1997496709, -1, 25, UINT32_MAX },
        { 7, 54967, "BeginBuildN", 1997496709, -1, 25, UINT32_MAX },
        { 6, 57698, "WaitEndN", 3234710487, -1, 9, UINT32_MAX },
        { 7, 57756, "WaitBeginN", 1997496709, -1, 10, UINT32_MAX },
        { 7, 57766, "WaitEndN", 1997496709, -1, 10, UINT32_MAX },
        { 8, 57776, "WaitBeginN", 1301841557, -1, 13, UINT32_MAX },
        { 8, 57782, "WaitEndN", 1301841557, -1, 13, UINT32_MAX },
        { 1, 57790, "WaitBeginN", 2189112322, -1, 11, UINT32_MAX },
        { 1, 57796, "WaitEndN", 2189112322, -1, 11, UINT32_MAX },
        { 9, 57802, "WaitBeginN", 2709710687, -1, 15, UINT32_MAX },
        { 9, 57807, "WaitEndN", 2709710687, -1, 15, UINT32_MAX },
        { 2, 57813, "WaitBeginN", 1147256140, -1, 16, UINT32_MAX },
        { 11, 58510, "BadPurgedN", 2121723309, -1, 22, UINT32_MAX },
        { 7, 58517, "AddedN", 1997496709, -1, 25, UINT32_MAX },
        { 8, 58722, "CacheMissForN", 1301841557, -1, 26, UINT32_MAX },
        { 8, 58727, "BeginBuildN", 1301841557, -1, 26, UINT32_MAX },
        { 6, 61739, "BadPurgedN", 3234710487, -1, 24, UINT32_MAX },
        { 2, 61749, "AddedP", 1147256140, -1, 17, UINT32_MAX },
        { 3, 61764, "CacheMissForP", 883660955, -1, 27, UINT32_MAX },
        { 3, 61768, "BeginBuildP", 883660955, -1, 27, UINT32_MAX },
        { 7, 62215, "BadPurgedN", 1997496709, -1, 25, UINT32_MAX },
        { 8, 62219, "AddedN", 1301841557, -1, 26, UINT32_MAX },
        { 9, 62347, "CacheMissForN", 2709710687, -1, 28, UINT32_MAX },
        { 9, 62350, "BeginBuildN", 2709710687, -1, 28, UINT32_MAX },
        { 2, 64343, "BadPurgedP", 1147256140, -1, 17, UINT32_MAX },
        { 2, 64346, "DeletionP", 1147256140, -1, 17, UINT32_MAX },
        { 3, 64348, "AddedP", 883660955, -1, 27, UINT32_MAX },
        { 4, 64353, "CacheMissForP", 644542265, -1, 29, UINT32_MAX },
        { 4, 64354, "BeginBuildP", 644542265, -1, 29, UINT32_MAX },
        { 8, 64793, "BadPurgedN", 1301841557, -1, 26, UINT32_MAX },
        { 2, 64808, "AddedP", 1147256140, -1, 19, UINT32_MAX },
        { 3, 64822, "CacheHitForP", 883660955, -1, 27, UINT32_MAX },
        { 4, 64825, "CacheMissForP", 644542265, -1, 30, UINT32_MAX },
        { 4, 64827, "BeginBuildP", 644542265, -1, 30, UINT32_MAX },
        { 2, 65076, "BadPurgedP", 1147256140, -1, 19, UINT32_MAX },
        { 2, 65078, "DeletionP", 1147256140, -1, 19, UINT32_MAX },
        { 9, 65081, "AddedN", 2709710687, -1, 28, UINT32_MAX },
        { 10, 65219, "CacheMissForN", 138923328, -1, 31, UINT32_MAX },
        { 10, 65223, "BeginBuildN", 138923328, -1, 31, UINT32_MAX },
        { 2, 66542, "WaitEndN", 1147256140, -1, 16, UINT32_MAX },
        { 3, 66589, "WaitBeginN", 883660955, -1, 20, UINT32_MAX },
        { 3, 66610, "WaitEndN", 883660955, -1, 20, UINT32_MAX },
        { 10, 66622, "WaitBeginN", 138923328, -1, 18, UINT32_MAX },
        { 3, 67067, "PurgedP", 883660955, -1, 27, UINT32_MAX },
        { 3, 67075, "DeletionP", 883660955, -1, 27, UINT32_MAX },
        { 4, 67076, "AddedP", 644542265, -1, 29, UINT32_MAX },
        { 5, 67081, "CacheMissForP", 2491771, -1, 32, UINT32_MAX },
        { 5, 67082, "BeginBuildP", 2491771, -1, 32, UINT32_MAX },
        { 4, 67740, "PWonRaceOverP", 644542265, -1, 29, 30 },
        { 4, 67748, "DeletionP", 644542265, -1, 30, UINT32_MAX },
        { 5, 67754, "CacheMissForP", 2491771, -1, 33, UINT32_MAX },
        { 5, 67755, "BeginBuildP", 2491771, -1, 33, UINT32_MAX },
        { 9, 68045, "BadPurgedN", 2709710687, -1, 28, UINT32_MAX },
        { 10, 68051, "AddedN", 138923328, -1, 31, UINT32_MAX },
        { 11, 68168, "CacheMissForN", 2121723309, -1, 34, UINT32_MAX },
        { 11, 68172, "BeginBuildN", 2121723309, -1, 34, UINT32_MAX },
        { 10, 68765, "WaitEndN", 138923328, -1, 18, UINT32_MAX },
        { 4, 68795, "WaitBeginN", 644542265, -1, 21, UINT32_MAX },
        { 4, 68804, "WaitEndN", 644542265, -1, 21, UINT32_MAX },
        { 5, 68813, "WaitBeginN", 2491771, -1, 23, UINT32_MAX },
        { 5, 68819, "WaitEndN", 2491771, -1, 23, UINT32_MAX },
        { 11, 68827, "WaitBeginN", 2121723309, -1, 22, UINT32_MAX },
        { 4, 69914, "BadPurgedP", 644542265, -1, 29, UINT32_MAX },
        { 4, 69917, "DeletionP", 644542265, -1, 29, UINT32_MAX },
        { 5, 69919, "AddedP", 2491771, -1, 32, UINT32_MAX },
        { 6, 69934, "CacheMissForP", 3234710487, -1, 35, UINT32_MAX },
        { 6, 69935, "BeginBuildP", 3234710487, -1, 35, UINT32_MAX },
        { 11, 70439, "WaitEndN", 2121723309, -1, 22, UINT32_MAX },
        { 6, 70467, "WaitBeginN", 3234710487, -1, 24, UINT32_MAX },
        { 6, 70487, "WaitEndN", 3234710487, -1, 24, UINT32_MAX },
        { 7, 70497, "WaitBeginN", 1997496709, -1, 25, UINT32_MAX },
        { 7, 70505, "WaitEndN", 1997496709, -1, 25, UINT32_MAX },
        { 8, 70513, "WaitBeginN", 1301841557, -1, 26, UINT32_MAX },
        { 8, 70518, "WaitEndN", 1301841557, -1, 26, UINT32_MAX },
        { 9, 70524, "WaitBeginN", 2709710687, -1, 28, UINT32_MAX },
        { 9, 70537, "WaitEndN", 2709710687, -1, 28, UINT32_MAX },
        { 10, 70547, "WaitBeginN", 138923328, -1, 31, UINT32_MAX },
        { 5, 70590, "PWonRaceOverP", 2491771, -1, 32, 33 },
        { 5, 70597, "DeletionP", 2491771, -1, 33, UINT32_MAX },
        { 6, 70605, "CacheMissForP", 3234710487, -1, 36, UINT32_MAX },
        { 6, 70608, "BeginBuildP", 3234710487, -1, 36, UINT32_MAX },
        { 10, 71148, "BadPurgedN", 138923328, -1, 31, UINT32_MAX },
        { 11, 71152, "AddedN", 2121723309, -1, 34, UINT32_MAX },
        { 5, 72631, "BadPurgedP", 2491771, -1, 32, UINT32_MAX },
        { 5, 72633, "DeletionP", 2491771, -1, 32, UINT32_MAX },
        { 6, 72634, "AddedP", 3234710487, -1, 35, UINT32_MAX },
        { 7, 72636, "CacheMissForP", 1997496709, -1, 37, UINT32_MAX },
        { 7, 72637, "BeginBuildP", 1997496709, -1, 37, UINT32_MAX },
        { 6, 73385, "PWonRaceOverP", 3234710487, -1, 35, 36 },
        { 6, 73391, "DeletionP", 3234710487, -1, 36, UINT32_MAX },
        { 7, 73398, "CacheMissForP", 1997496709, -1, 38, UINT32_MAX },
        { 7, 73399, "BeginBuildP", 1997496709, -1, 38, UINT32_MAX },
        { 11, 75252, "BadPurgedN", 2121723309, -1, 34, UINT32_MAX },
        { 7, 75256, "AddedP", 1997496709, -1, 37, UINT32_MAX },
        { 8, 75259, "CacheMissForP", 1301841557, -1, 39, UINT32_MAX },
        { 8, 75260, "BeginBuildP", 1301841557, -1, 39, UINT32_MAX },
        { 7, 76156, "PWonRaceOverP", 1997496709, -1, 37, 38 },
        { 7, 76162, "DeletionP", 1997496709, -1, 38, UINT32_MAX },
        { 8, 76166, "CacheMissForP", 1301841557, -1, 40, UINT32_MAX },
        { 8, 76167, "BeginBuildP", 1301841557, -1, 40, UINT32_MAX },
        { 6, 77955, "BadPurgedP", 3234710487, -1, 35, UINT32_MAX },
        { 6, 77957, "DeletionP", 3234710487, -1, 35, UINT32_MAX },
        { 8, 77959, "AddedP", 1301841557, -1, 39, UINT32_MAX },
        { 9, 77972, "CacheMissForP", 2709710687, -1, 41, UINT32_MAX },
        { 9, 77973, "BeginBuildP", 2709710687, -1, 41, UINT32_MAX },
        { 8, 78901, "PWonRaceOverP", 1301841557, -1, 39, 40 },
        { 8, 78906, "DeletionP", 1301841557, -1, 40, UINT32_MAX },
        { 9, 78915, "CacheMissForP", 2709710687, -1, 42, UINT32_MAX },
        { 9, 78916, "BeginBuildP", 2709710687, -1, 42, UINT32_MAX },
        { 7, 80540, "BadPurgedP", 1997496709, -1, 37, UINT32_MAX },
        { 7, 80541, "DeletionP", 1997496709, -1, 37, UINT32_MAX },
        { 9, 80543, "AddedP", 2709710687, -1, 41, UINT32_MAX },
        { 10, 80547, "CacheMissForP", 138923328, -1, 43, UINT32_MAX },
        { 10, 80548, "BeginBuildP", 138923328, -1, 43, UINT32_MAX },
        { 9, 81591, "PWonRaceOverP", 2709710687, -1, 41, 42 },
        { 9, 81596, "DeletionP", 2709710687, -1, 42, UINT32_MAX },
        { 10, 81600, "CacheMissForP", 138923328, -1, 44, UINT32_MAX },
        { 10, 81601, "BeginBuildP", 138923328, -1, 44, UINT32_MAX },
        { 8, 83165, "BadPurgedP", 1301841557, -1, 39, UINT32_MAX },
        { 8, 83167, "DeletionP", 1301841557, -1, 39, UINT32_MAX },
        { 10, 83169, "AddedP", 138923328, -1, 43, UINT32_MAX },
        { 11, 83171, "CacheMissForP", 2121723309, -1, 45, UINT32_MAX },
        { 11, 83172, "BeginBuildP", 2121723309, -1, 45, UINT32_MAX },
        { 10, 83616, "WaitEndN", 138923328, -1, 31, UINT32_MAX },
        { 11, 83648, "WaitBeginN", 2121723309, -1, 34, UINT32_MAX },
        { 11, 83656, "WaitEndN", 2121723309, -1, 34, UINT32_MAX },
        { 10, 84362, "PWonRaceOverP", 138923328, -1, 43, 44 },
        { 10, 84365, "DeletionP", 138923328, -1, 44, UINT32_MAX },
        { 11, 84370, "CacheMissForP", 2121723309, -1, 46, UINT32_MAX },
        { 11, 84371, "BeginBuildP", 2121723309, -1, 46, UINT32_MAX },
        { 9, 85755, "BadPurgedP", 2709710687, -1, 41, UINT32_MAX },
        { 9, 85756, "DeletionP", 2709710687, -1, 41, UINT32_MAX },
        { 11, 85758, "AddedP", 2121723309, -1, 45, UINT32_MAX },
        { 11, 87041, "PWonRaceOverP", 2121723309, -1, 45, 46 },
        { 11, 87045, "DeletionP", 2121723309, -1, 46, UINT32_MAX },
        { 1, 92772, "DeletionN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 92786, "DeletionN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 92799, "DeletionN", 883660955, -1, 6, UINT32_MAX },
        { 4, 92812, "DeletionN", 644542265, -1, 7, UINT32_MAX },
        { 5, 92825, "DeletionN", 2491771, -1, 8, UINT32_MAX },
        { 6, 92835, "DeletionN", 3234710487, -1, 9, UINT32_MAX },
        { 0, 92860, "DeletionN", 477900207, -1, 3, UINT32_MAX },
        { 7, 92874, "DeletionN", 1997496709, -1, 10, UINT32_MAX },
        { 8, 92887, "DeletionN", 1301841557, -1, 13, UINT32_MAX },
        { 1, 92906, "DeletionN", 2189112322, -1, 11, UINT32_MAX },
        { 9, 92917, "DeletionN", 2709710687, -1, 15, UINT32_MAX },
        { 2, 92948, "DeletionN", 1147256140, -1, 16, UINT32_MAX },
        { 3, 93004, "DeletionN", 883660955, -1, 20, UINT32_MAX },
        { 10, 93030, "DeletionN", 138923328, -1, 18, UINT32_MAX },
        { 4, 93060, "DeletionN", 644542265, -1, 21, UINT32_MAX },
        { 5, 93090, "DeletionN", 2491771, -1, 23, UINT32_MAX },
        { 11, 93149, "DeletionN", 2121723309, -1, 22, UINT32_MAX },
        { 6, 93199, "DeletionN", 3234710487, -1, 24, UINT32_MAX },
        { 7, 93225, "DeletionN", 1997496709, -1, 25, UINT32_MAX },
        { 8, 93254, "DeletionN", 1301841557, -1, 26, UINT32_MAX },
        { 9, 93278, "DeletionN", 2709710687, -1, 28, UINT32_MAX },
        { 10, 93298, "DeletionN", 138923328, -1, 31, UINT32_MAX },
        { 11, 93325, "DeletionN", 2121723309, -1, 34, UINT32_MAX },
        { 11, 93340, "DeletionP", 2121723309, -1, 45, UINT32_MAX },
        { 10, 93349, "DeletionP", 138923328, -1, 43, UINT32_MAX },
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
            SkASSERT(fInState1[i] != dude);  // had better not have already been in this state
        }

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
 0:  477900207 [  330 CacheMissForP 0][  345 CacheMissForP 1][  349 BeginBuildP 0][  349 BeginBuildP 1][ 2738 CacheMissForN 2][ 2749 CacheMissForN 3][ 2752 BeginBuildN 2][ 2752 BeginBuildN 3][ 8665 AddedN 3][ 8729 NWonRaceOverN 3 2][ 8736 WaitBeginN 2][ 9363 WaitBeginN 3][15194 BadPurgedN 3][28681 WaitEndN 2][28709 DeletionN 2][30491 WaitEndN 3][30873 AddedP 0][32246 PWonRaceOverP 0 1][32256 DeletionP 1][35584 BadPurgedP 0][35593 DeletionP 0][92860 DeletionN 3]
 1: 2189112322 [ 9064 CacheMissForN 4][ 9069 BeginBuildN 4][11989 AddedN 4][18300 BadPurgedN 4][28822 CacheMissForN 11][28824 BeginBuildN 11][30603 WaitBeginN 4][30882 CacheMissForP 12][30883 BeginBuildP 12][32264 CacheMissForP 14][32265 BeginBuildP 14][34809 WaitEndN 4][35595 AddedN 11][36450 NWonRaceOverP 11 14][36456 DeletionP 14][38535 NWonRaceOverP 11 12][38555 DeletionP 12][43208 BadPurgedN 11][57790 WaitBeginN 11][57796 WaitEndN 11][92772 DeletionN 4][92906 DeletionN 11]
 2: 1147256140 [12115 CacheMissForN 5][12120 BeginBuildN 5][15204 AddedN 5][21331 BadPurgedN 5][34859 WaitBeginN 5][35714 CacheMissForN 16][35716 BeginBuildN 16][36461 CacheMissForP 17][36462 BeginBuildP 17][38571 CacheMissForP 19][38575 BeginBuildP 19][38668 AddedN 16][44109 WaitEndN 5][44331 BadPurgedN 16][57813 WaitBeginN 16][61749 AddedP 17][64335 BadPurgedP 17][64346 DeletionP 17][64808 AddedP 19][65072 BadPurgedP 19][65078 DeletionP 19][66542 WaitEndN 16][92786 DeletionN 5][92948 DeletionN 16]
 3:  883660955 [15355 CacheMissForN 6][15359 BeginBuildN 6][18308 AddedN 6][24416 BadPurgedN 6][38782 CacheMissForN 20][38784 BeginBuildN 20][43215 AddedN 20][44160 WaitBeginN 6][44170 WaitEndN 6][46272 BadPurgedN 20][61764 CacheMissForP 27][61768 BeginBuildP 27][64348 AddedP 27][64822 CacheHitForP 27][66589 WaitBeginN 20][66610 WaitEndN 20][67067 PurgedP 27][67075 DeletionP 27][92799 DeletionN 6][93004 DeletionN 20]
 4:  644542265 [18422 CacheMissForN 7][18425 BeginBuildN 7][21339 AddedN 7][27722 BadPurgedN 7][43337 CacheMissForN 21][43339 BeginBuildN 21][44181 WaitBeginN 7][46061 WaitEndN 7][46277 AddedN 21][49753 BadPurgedN 21][64353 CacheMissForP 29][64354 BeginBuildP 29][64825 CacheMissForP 30][64827 BeginBuildP 30][67076 AddedP 29][67740 PWonRaceOverP 29 30][67748 DeletionP 30][68795 WaitBeginN 21][68804 WaitEndN 21][69903 BadPurgedP 29][69917 DeletionP 29][92812 DeletionN 7][93060 DeletionN 21]
 5:    2491771 [21467 CacheMissForN 8][21469 BeginBuildN 8][24423 AddedN 8][30859 BadPurgedN 8][46101 WaitBeginN 8][46112 WaitEndN 8][46389 CacheMissForN 23][46391 BeginBuildN 23][49342 AddedN 23][54834 BadPurgedN 23][67081 CacheMissForP 32][67082 BeginBuildP 32][67754 CacheMissForP 33][67755 BeginBuildP 33][68813 WaitBeginN 23][68819 WaitEndN 23][69919 AddedP 32][70590 PWonRaceOverP 32 33][70597 DeletionP 33][72626 BadPurgedP 32][72633 DeletionP 32][92824 DeletionN 8][93090 DeletionN 23]
 6: 3234710487 [24540 CacheMissForN 9][24542 BeginBuildN 9][27733 AddedN 9][30869 BadPurgedN 9][46121 WaitBeginN 9][49476 CacheMissForN 24][49478 BeginBuildN 24][54843 AddedN 24][57698 WaitEndN 9][61739 BadPurgedN 24][69934 CacheMissForP 35][69935 BeginBuildP 35][70467 WaitBeginN 24][70487 WaitEndN 24][70605 CacheMissForP 36][70608 BeginBuildP 36][72634 AddedP 35][73385 PWonRaceOverP 35 36][73391 DeletionP 36][77951 BadPurgedP 35][77957 DeletionP 35][92835 DeletionN 9][93199 DeletionN 24]
 7: 1997496709 [27888 CacheMissForN 10][27890 BeginBuildN 10][30866 AddedN 10][33860 BadPurgedN 10][54965 CacheMissForN 25][54967 BeginBuildN 25][57756 WaitBeginN 10][57766 WaitEndN 10][58517 AddedN 25][62215 BadPurgedN 25][70497 WaitBeginN 25][70505 WaitEndN 25][72636 CacheMissForP 37][72637 BeginBuildP 37][73398 CacheMissForP 38][73399 BeginBuildP 38][75256 AddedP 37][76156 PWonRaceOverP 37 38][76162 DeletionP 38][80535 BadPurgedP 37][80541 DeletionP 37][92874 DeletionN 10][93225 DeletionN 25]
 8: 1301841557 [30978 CacheMissForN 13][30980 BeginBuildN 13][33866 AddedN 13][37078 BadPurgedN 13][57776 WaitBeginN 13][57782 WaitEndN 13][58722 CacheMissForN 26][58727 BeginBuildN 26][62219 AddedN 26][64793 BadPurgedN 26][70512 WaitBeginN 26][70518 WaitEndN 26][75259 CacheMissForP 39][75260 BeginBuildP 39][76166 CacheMissForP 40][76167 BeginBuildP 40][77959 AddedP 39][78901 PWonRaceOverP 39 40][78906 DeletionP 40][83161 BadPurgedP 39][83167 DeletionP 39][92887 DeletionN 13][93254 DeletionN 26]
 9: 2709710687 [33984 CacheMissForN 15][33986 BeginBuildN 15][37086 AddedN 15][38662 BadPurgedN 15][57802 WaitBeginN 15][57807 WaitEndN 15][62347 CacheMissForN 28][62350 BeginBuildN 28][65081 AddedN 28][68045 BadPurgedN 28][70524 WaitBeginN 28][70537 WaitEndN 28][77972 CacheMissForP 41][77973 BeginBuildP 41][78915 CacheMissForP 42][78916 BeginBuildP 42][80543 AddedP 41][81591 PWonRaceOverP 41 42][81596 DeletionP 42][85751 BadPurgedP 41][85756 DeletionP 41][92917 DeletionN 15][93278 DeletionN 28]
10:  138923328 [37223 CacheMissForN 18][37226 BeginBuildN 18][44349 AddedN 18][49333 BadPurgedN 18][65219 CacheMissForN 31][65223 BeginBuildN 31][66622 WaitBeginN 18][68051 AddedN 31][68765 WaitEndN 18][70547 WaitBeginN 31][71148 BadPurgedN 31][80547 CacheMissForP 43][80548 BeginBuildP 43][81600 CacheMissForP 44][81601 BeginBuildP 44][83169 AddedP 43][83616 WaitEndN 31][84362 PWonRaceOverP 43 44][84365 DeletionP 44][93030 DeletionN 18][93298 DeletionN 31][93349 DeletionP 43]
11: 2121723309 [44724 CacheMissForN 22][44730 BeginBuildN 22][49766 AddedN 22][58510 BadPurgedN 22][68168 CacheMissForN 34][68172 BeginBuildN 34][68827 WaitBeginN 22][70439 WaitEndN 22][71152 AddedN 34][75251 BadPurgedN 34][83171 CacheMissForP 45][83172 BeginBuildP 45][83648 WaitBeginN 34][83656 WaitEndN 34][84370 CacheMissForP 46][84371 BeginBuildP 46][85758 AddedP 45][87041 PWonRaceOverP 45 46][87045 DeletionP 46][93149 DeletionN 22][93325 DeletionN 34][93340 DeletionP 45]
#endif

#if 0
Reconstructed
 0:  477900207 [  330 CacheMissForP 0][  345 CacheMissForP 1][  349 BeginBuildP 0][  349 BeginBuildP 1][ 2738 CacheMissForN 2][ 2749 CacheMissForN 3][ 2752 BeginBuildN 2][ 2752 BeginBuildN 3][ 8665 AddedN 3][ 8729 NWonRaceOverN 3 2][ 8736 WaitBeginN 2][ 9363 WaitBeginN 3][15194 BadPurgedN 3][28681 WaitEndN 2][28709 DeletionN 2][30491 WaitEndN 3][30873 AddedP 0][32246 PWonRaceOverP 0 1][32256 DeletionP 1][35590 BadPurgedP 0][35593 DeletionP 0][92860 DeletionN 3]
 1: 2189112322 [ 9064 CacheMissForN 4][ 9069 BeginBuildN 4][11989 AddedN 4][18300 BadPurgedN 4][28822 CacheMissForN 11][28824 BeginBuildN 11][30603 WaitBeginN 4][30882 CacheMissForP 12][30883 BeginBuildP 12][32264 CacheMissForP 14][32265 BeginBuildP 14][34809 WaitEndN 4][35595 AddedN 11][36450 NWonRaceOverP 11 14][36456 DeletionP 14][38535 NWonRaceOverP 11 12][38556 DeletionP 12][43208 BadPurgedN 11][57790 WaitBeginN 11][57796 WaitEndN 11][92772 DeletionN 4][92906 DeletionN 11]
 2: 1147256140 [12115 CacheMissForN 5][12120 BeginBuildN 5][15204 AddedN 5][21331 BadPurgedN 5][34860 WaitBeginN 5][35714 CacheMissForN 16][35716 BeginBuildN 16][36461 CacheMissForP 17][36462 BeginBuildP 17][38571 CacheMissForP 19][38575 BeginBuildP 19][38668 AddedN 16][44109 WaitEndN 5][44331 BadPurgedN 16][57813 WaitBeginN 16][61749 AddedP 17][64343 BadPurgedP 17][64346 DeletionP 17][64808 AddedP 19][65076 BadPurgedP 19][65078 DeletionP 19][66542 WaitEndN 16][92786 DeletionN 5][92948 DeletionN 16]
 3:  883660955 [15355 CacheMissForN 6][15359 BeginBuildN 6][18308 AddedN 6][24416 BadPurgedN 6][38782 CacheMissForN 20][38784 BeginBuildN 20][43215 AddedN 20][44160 WaitBeginN 6][44170 WaitEndN 6][46272 BadPurgedN 20][61764 CacheMissForP 27][61768 BeginBuildP 27][64348 AddedP 27][64822 CacheHitForP 27][66589 WaitBeginN 20][66610 WaitEndN 20][67067 PurgedP 27][67075 DeletionP 27][92799 DeletionN 6][93004 DeletionN 20]
 4:  644542265 [18422 CacheMissForN 7][18425 BeginBuildN 7][21339 AddedN 7][27722 BadPurgedN 7][43337 CacheMissForN 21][43339 BeginBuildN 21][44181 WaitBeginN 7][46061 WaitEndN 7][46277 AddedN 21][49753 BadPurgedN 21][64353 CacheMissForP 29][64354 BeginBuildP 29][64825 CacheMissForP 30][64827 BeginBuildP 30][67076 AddedP 29][67740 PWonRaceOverP 29 30][67748 DeletionP 30][68795 WaitBeginN 21][68804 WaitEndN 21][69914 BadPurgedP 29][69917 DeletionP 29][92812 DeletionN 7][93060 DeletionN 21]
 5:    2491771 [21467 CacheMissForN 8][21469 BeginBuildN 8][24423 AddedN 8][30859 BadPurgedN 8][46101 WaitBeginN 8][46112 WaitEndN 8][46389 CacheMissForN 23][46391 BeginBuildN 23][49342 AddedN 23][54834 BadPurgedN 23][67081 CacheMissForP 32][67082 BeginBuildP 32][67754 CacheMissForP 33][67755 BeginBuildP 33][68813 WaitBeginN 23][68819 WaitEndN 23][69919 AddedP 32][70590 PWonRaceOverP 32 33][70597 DeletionP 33][72631 BadPurgedP 32][72633 DeletionP 32][92825 DeletionN 8][93090 DeletionN 23]
 6: 3234710487 [24540 CacheMissForN 9][24542 BeginBuildN 9][27733 AddedN 9][30869 BadPurgedN 9][46121 WaitBeginN 9][49476 CacheMissForN 24][49478 BeginBuildN 24][54843 AddedN 24][57698 WaitEndN 9][61739 BadPurgedN 24][69934 CacheMissForP 35][69935 BeginBuildP 35][70467 WaitBeginN 24][70487 WaitEndN 24][70605 CacheMissForP 36][70608 BeginBuildP 36][72634 AddedP 35][73385 PWonRaceOverP 35 36][73391 DeletionP 36][77955 BadPurgedP 35][77957 DeletionP 35][92835 DeletionN 9][93199 DeletionN 24]
 7: 1997496709 [27888 CacheMissForN 10][27890 BeginBuildN 10][30866 AddedN 10][33860 BadPurgedN 10][54965 CacheMissForN 25][54967 BeginBuildN 25][57756 WaitBeginN 10][57766 WaitEndN 10][58517 AddedN 25][62215 BadPurgedN 25][70497 WaitBeginN 25][70505 WaitEndN 25][72636 CacheMissForP 37][72637 BeginBuildP 37][73398 CacheMissForP 38][73399 BeginBuildP 38][75256 AddedP 37][76156 PWonRaceOverP 37 38][76162 DeletionP 38][80540 BadPurgedP 37][80541 DeletionP 37][92874 DeletionN 10][93225 DeletionN 25]
 8: 1301841557 [30978 CacheMissForN 13][30980 BeginBuildN 13][33866 AddedN 13][37078 BadPurgedN 13][57776 WaitBeginN 13][57782 WaitEndN 13][58722 CacheMissForN 26][58727 BeginBuildN 26][62219 AddedN 26][64793 BadPurgedN 26][70513 WaitBeginN 26][70518 WaitEndN 26][75259 CacheMissForP 39][75260 BeginBuildP 39][76166 CacheMissForP 40][76167 BeginBuildP 40][77959 AddedP 39][78901 PWonRaceOverP 39 40][78906 DeletionP 40][83165 BadPurgedP 39][83167 DeletionP 39][92887 DeletionN 13][93254 DeletionN 26]
 9: 2709710687 [33984 CacheMissForN 15][33986 BeginBuildN 15][37086 AddedN 15][38662 BadPurgedN 15][57802 WaitBeginN 15][57807 WaitEndN 15][62347 CacheMissForN 28][62350 BeginBuildN 28][65081 AddedN 28][68045 BadPurgedN 28][70524 WaitBeginN 28][70537 WaitEndN 28][77972 CacheMissForP 41][77973 BeginBuildP 41][78915 CacheMissForP 42][78916 BeginBuildP 42][80543 AddedP 41][81591 PWonRaceOverP 41 42][81596 DeletionP 42][85755 BadPurgedP 41][85756 DeletionP 41][92917 DeletionN 15][93278 DeletionN 28]
10:  138923328 [37223 CacheMissForN 18][37226 BeginBuildN 18][44349 AddedN 18][49333 BadPurgedN 18][65219 CacheMissForN 31][65223 BeginBuildN 31][66622 WaitBeginN 18][68051 AddedN 31][68765 WaitEndN 18][70547 WaitBeginN 31][71148 BadPurgedN 31][80547 CacheMissForP 43][80548 BeginBuildP 43][81600 CacheMissForP 44][81601 BeginBuildP 44][83169 AddedP 43][83616 WaitEndN 31][84362 PWonRaceOverP 43 44][84365 DeletionP 44][93030 DeletionN 18][93298 DeletionN 31][93349 DeletionP 43]
11: 2121723309 [44724 CacheMissForN 22][44730 BeginBuildN 22][49766 AddedN 22][58510 BadPurgedN 22][68168 CacheMissForN 34][68172 BeginBuildN 34][68827 WaitBeginN 22][70439 WaitEndN 22][71152 AddedN 34][75252 BadPurgedN 34][83171 CacheMissForP 45][83172 BeginBuildP 45][83648 WaitBeginN 34][83656 WaitEndN 34][84370 CacheMissForP 46][84371 BeginBuildP 46][85758 AddedP 45][87041 PWonRaceOverP 45 46][87045 DeletionP 46][93149 DeletionN 22][93325 DeletionN 34][93340 DeletionP 45]
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

            int64_t waitStart = 0;
            for (auto l : log) {
                if (!strcmp(l.fTag, "CacheMissForP") ||
                    !strcmp(l.fTag, "CacheMissForN")) {

                    if (fsm[Node::kDeleted].isInState(l.fPipelineID1)) {
                        // This Pipeline was purged and is being re-created
                        fsm[Node::kDeleted].removeFromState(l.fPipelineID1);
                    }

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
                } else if (!strcmp(l.fTag, "PurgedP") || !strcmp(l.fTag, "BadPurgedP") ||
                           !strcmp(l.fTag, "PurgedN") || !strcmp(l.fTag, "BadPurgedN")) {
                    fsm[Node::kAddedToCache].removeFromState(l.fPipelineID1);
                    fsm[Node::kToBeDeleted].addToState(l.fPipelineID1, l.fTime);
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
