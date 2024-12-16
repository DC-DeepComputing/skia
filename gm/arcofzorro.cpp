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
        { 0, 293, "CacheMissForP", 477900207, -1, 0, UINT32_MAX },
        { 0, 302, "BeginBuildP", 477900207, -1, 0, UINT32_MAX },
        { 0, 368, "CacheMissForP", 477900207, -1, 1, UINT32_MAX },
        { 0, 382, "BeginBuildP", 477900207, -1, 1, UINT32_MAX },
        { 0, 2523, "CacheMissForN", 477900207, -1, 2, UINT32_MAX },
        { 0, 2530, "BeginBuildN", 477900207, -1, 2, UINT32_MAX },
        { 0, 2623, "CacheMissForN", 477900207, -1, 3, UINT32_MAX },
        { 0, 2625, "BeginBuildN", 477900207, -1, 3, UINT32_MAX },
        { 0, 8840, "AddedN", 477900207, -1, 2, UINT32_MAX },
        { 0, 8881, "NWonRaceOverN", 477900207, -1, 2, 3 },
        { 0, 8889, "WaitBeginN", 477900207, -1, 3, UINT32_MAX },
        { 1, 9067, "CacheMissForN", 2189112322, -1, 4, UINT32_MAX },
        { 1, 9070, "BeginBuildN", 2189112322, -1, 4, UINT32_MAX },
        { 0, 11329, "WaitBeginN", 477900207, -1, 2, UINT32_MAX },
        { 1, 12036, "AddedN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 12112, "CacheMissForN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 12114, "BeginBuildN", 1147256140, -1, 5, UINT32_MAX },
        { 0, 15040, "PurgedN", 477900207, -1, 2, UINT32_MAX },
        { 2, 15045, "AddedN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 15124, "CacheMissForN", 883660955, -1, 6, UINT32_MAX },
        { 3, 15126, "BeginBuildN", 883660955, -1, 6, UINT32_MAX },
        { 1, 19064, "PurgedN", 2189112322, -1, 4, UINT32_MAX },
        { 3, 19073, "AddedN", 883660955, -1, 6, UINT32_MAX },
        { 4, 19201, "CacheMissForN", 644542265, -1, 7, UINT32_MAX },
        { 4, 19203, "BeginBuildN", 644542265, -1, 7, UINT32_MAX },
        { 2, 26421, "PurgedN", 1147256140, -1, 5, UINT32_MAX },
        { 4, 26431, "AddedN", 644542265, -1, 7, UINT32_MAX },
        { 5, 26584, "CacheMissForN", 2491771, -1, 8, UINT32_MAX },
        { 5, 26586, "BeginBuildN", 2491771, -1, 8, UINT32_MAX },
        { 3, 29459, "PurgedN", 883660955, -1, 6, UINT32_MAX },
        { 5, 29466, "AddedN", 2491771, -1, 8, UINT32_MAX },
        { 6, 29584, "CacheMissForN", 3234710487, -1, 9, UINT32_MAX },
        { 6, 29587, "BeginBuildN", 3234710487, -1, 9, UINT32_MAX },
        { 4, 29757, "PurgedN", 644542265, -1, 7, UINT32_MAX },
        { 0, 29764, "AddedP", 477900207, -1, 1, UINT32_MAX },
        { 1, 29772, "CacheMissForP", 2189112322, -1, 10, UINT32_MAX },
        { 1, 29774, "BeginBuildP", 2189112322, -1, 10, UINT32_MAX },
        { 0, 30250, "WaitEndN", 477900207, -1, 3, UINT32_MAX },
        { 0, 30255, "DeletionN", 477900207, -1, 3, UINT32_MAX },
        { 1, 30386, "CacheMissForN", 2189112322, -1, 11, UINT32_MAX },
        { 1, 30388, "BeginBuildN", 2189112322, -1, 11, UINT32_MAX },
        { 0, 31149, "WaitEndN", 477900207, -1, 2, UINT32_MAX },
        { 1, 31205, "WaitBeginN", 2189112322, -1, 4, UINT32_MAX },
        { 0, 31288, "PWonRaceOverP", 477900207, -1, 1, 0 },
        { 0, 31292, "DeletionP", 477900207, -1, 0, UINT32_MAX },
        { 1, 31298, "CacheMissForP", 2189112322, -1, 12, UINT32_MAX },
        { 1, 31300, "BeginBuildP", 2189112322, -1, 12, UINT32_MAX },
        { 5, 32357, "PurgedN", 2491771, -1, 8, UINT32_MAX },
        { 6, 32364, "AddedN", 3234710487, -1, 9, UINT32_MAX },
        { 7, 32474, "CacheMissForN", 1997496709, -1, 13, UINT32_MAX },
        { 7, 32476, "BeginBuildN", 1997496709, -1, 13, UINT32_MAX },
        { 0, 33796, "BadPurgedP", 477900207, -1, 1, UINT32_MAX },
        { 0, 33800, "DeletionP", 477900207, -1, 1, UINT32_MAX },
        { 1, 33802, "AddedN", 2189112322, -1, 11, UINT32_MAX },
        { 2, 33963, "CacheMissForN", 1147256140, -1, 14, UINT32_MAX },
        { 2, 33967, "BeginBuildN", 1147256140, -1, 14, UINT32_MAX },
        { 6, 35351, "PurgedN", 3234710487, -1, 9, UINT32_MAX },
        { 7, 35359, "AddedN", 1997496709, -1, 13, UINT32_MAX },
        { 8, 35460, "CacheMissForN", 1301841557, -1, 15, UINT32_MAX },
        { 8, 35463, "BeginBuildN", 1301841557, -1, 15, UINT32_MAX },
        { 1, 38349, "PurgedN", 2189112322, -1, 11, UINT32_MAX },
        { 8, 38356, "AddedN", 1301841557, -1, 15, UINT32_MAX },
        { 9, 38486, "CacheMissForN", 2709710687, -1, 16, UINT32_MAX },
        { 9, 38489, "BeginBuildN", 2709710687, -1, 16, UINT32_MAX },
        { 1, 39614, "WaitEndN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 39770, "WaitBeginN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 39813, "WaitEndN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 39838, "WaitBeginN", 883660955, -1, 6, UINT32_MAX },
        { 3, 40175, "WaitEndN", 883660955, -1, 6, UINT32_MAX },
        { 4, 40216, "WaitBeginN", 644542265, -1, 7, UINT32_MAX },
        { 7, 41362, "PurgedN", 1997496709, -1, 13, UINT32_MAX },
        { 9, 41367, "AddedN", 2709710687, -1, 16, UINT32_MAX },
        { 10, 41481, "CacheMissForN", 138923328, -1, 17, UINT32_MAX },
        { 10, 41483, "BeginBuildN", 138923328, -1, 17, UINT32_MAX },
        { 8, 41679, "PurgedN", 1301841557, -1, 15, UINT32_MAX },
        { 2, 41684, "AddedN", 1147256140, -1, 14, UINT32_MAX },
        { 3, 41806, "CacheMissForN", 883660955, -1, 18, UINT32_MAX },
        { 3, 41808, "BeginBuildN", 883660955, -1, 18, UINT32_MAX },
        { 9, 44511, "PurgedN", 2709710687, -1, 16, UINT32_MAX },
        { 10, 44518, "AddedN", 138923328, -1, 17, UINT32_MAX },
        { 2, 44538, "PurgedN", 1147256140, -1, 14, UINT32_MAX },
        { 3, 44544, "AddedN", 883660955, -1, 18, UINT32_MAX },
        { 11, 44633, "CacheMissForN", 2121723309, -1, 19, UINT32_MAX },
        { 11, 44636, "BeginBuildN", 2121723309, -1, 19, UINT32_MAX },
        { 4, 44671, "CacheMissForN", 644542265, -1, 20, UINT32_MAX },
        { 4, 44674, "BeginBuildN", 644542265, -1, 20, UINT32_MAX },
        { 10, 47663, "PurgedN", 138923328, -1, 17, UINT32_MAX },
        { 11, 47669, "AddedN", 2121723309, -1, 19, UINT32_MAX },
        { 3, 48472, "PurgedN", 883660955, -1, 18, UINT32_MAX },
        { 1, 48481, "AddedP", 2189112322, -1, 10, UINT32_MAX },
        { 2, 48489, "CacheMissForP", 1147256140, -1, 21, UINT32_MAX },
        { 2, 48490, "BeginBuildP", 1147256140, -1, 21, UINT32_MAX },
        { 11, 49904, "PurgedN", 2121723309, -1, 19, UINT32_MAX },
        { 4, 49911, "AddedN", 644542265, -1, 20, UINT32_MAX },
        { 5, 50043, "CacheMissForN", 2491771, -1, 22, UINT32_MAX },
        { 5, 50046, "BeginBuildN", 2491771, -1, 22, UINT32_MAX },
        { 1, 51454, "BadPurgedP", 2189112322, -1, 10, UINT32_MAX },
        { 1, 51457, "DeletionP", 2189112322, -1, 10, UINT32_MAX },
        { 2, 51458, "AddedP", 1147256140, -1, 21, UINT32_MAX },
        { 3, 51472, "CacheMissForP", 883660955, -1, 23, UINT32_MAX },
        { 3, 51473, "BeginBuildP", 883660955, -1, 23, UINT32_MAX },
        { 4, 54204, "PurgedN", 644542265, -1, 20, UINT32_MAX },
        { 3, 54215, "AddedP", 883660955, -1, 23, UINT32_MAX },
        { 4, 54220, "CacheMissForP", 644542265, -1, 24, UINT32_MAX },
        { 4, 54222, "BeginBuildP", 644542265, -1, 24, UINT32_MAX },
        { 2, 54673, "BadPurgedP", 1147256140, -1, 21, UINT32_MAX },
        { 2, 54676, "DeletionP", 1147256140, -1, 21, UINT32_MAX },
        { 5, 54679, "AddedN", 2491771, -1, 22, UINT32_MAX },
        { 6, 54818, "CacheMissForN", 3234710487, -1, 25, UINT32_MAX },
        { 6, 54820, "BeginBuildN", 3234710487, -1, 25, UINT32_MAX },
        { 3, 57592, "BadPurgedP", 883660955, -1, 23, UINT32_MAX },
        { 3, 57595, "DeletionP", 883660955, -1, 23, UINT32_MAX },
        { 1, 57596, "AddedP", 2189112322, -1, 12, UINT32_MAX },
        { 2, 57603, "CacheMissForP", 1147256140, -1, 26, UINT32_MAX },
        { 2, 57604, "BeginBuildP", 1147256140, -1, 26, UINT32_MAX },
        { 5, 57969, "PurgedN", 2491771, -1, 22, UINT32_MAX },
        { 6, 57972, "AddedN", 3234710487, -1, 25, UINT32_MAX },
        { 7, 58093, "CacheMissForN", 1997496709, -1, 27, UINT32_MAX },
        { 7, 58095, "BeginBuildN", 1997496709, -1, 27, UINT32_MAX },
        { 1, 60357, "BadPurgedP", 2189112322, -1, 12, UINT32_MAX },
        { 1, 60359, "DeletionP", 2189112322, -1, 12, UINT32_MAX },
        { 2, 60361, "AddedP", 1147256140, -1, 26, UINT32_MAX },
        { 3, 60373, "CacheMissForP", 883660955, -1, 28, UINT32_MAX },
        { 3, 60374, "BeginBuildP", 883660955, -1, 28, UINT32_MAX },
        { 6, 61079, "PurgedN", 3234710487, -1, 25, UINT32_MAX },
        { 7, 61090, "AddedN", 1997496709, -1, 27, UINT32_MAX },
        { 8, 61234, "CacheMissForN", 1301841557, -1, 29, UINT32_MAX },
        { 4, 61971, "WaitEndN", 644542265, -1, 7, UINT32_MAX },
        { 8, 63241, "BeginBuildN", 1301841557, -1, 29, UINT32_MAX },
        { 2, 63251, "BadPurgedP", 1147256140, -1, 26, UINT32_MAX },
        { 2, 64503, "DeletionP", 1147256140, -1, 26, UINT32_MAX },
        { 3, 64576, "AddedP", 883660955, -1, 28, UINT32_MAX },
        { 4, 64589, "CacheMissForP", 644542265, -1, 30, UINT32_MAX },
        { 4, 64590, "BeginBuildP", 644542265, -1, 30, UINT32_MAX },
        { 5, 64600, "WaitBeginN", 2491771, -1, 8, UINT32_MAX },
        { 5, 64619, "WaitEndN", 2491771, -1, 8, UINT32_MAX },
        { 6, 64661, "WaitBeginN", 3234710487, -1, 9, UINT32_MAX },
        { 6, 64668, "WaitEndN", 3234710487, -1, 9, UINT32_MAX },
        { 1, 64677, "WaitBeginN", 2189112322, -1, 11, UINT32_MAX },
        { 1, 64697, "WaitEndN", 2189112322, -1, 11, UINT32_MAX },
        { 7, 64706, "WaitBeginN", 1997496709, -1, 13, UINT32_MAX },
        { 7, 66064, "PurgedN", 1997496709, -1, 27, UINT32_MAX },
        { 8, 66069, "AddedN", 1301841557, -1, 29, UINT32_MAX },
        { 3, 71100, "BadPurgedP", 883660955, -1, 28, UINT32_MAX },
        { 3, 71104, "DeletionP", 883660955, -1, 28, UINT32_MAX },
        { 4, 71106, "AddedP", 644542265, -1, 30, UINT32_MAX },
        { 5, 71113, "CacheMissForP", 2491771, -1, 31, UINT32_MAX },
        { 5, 71114, "BeginBuildP", 2491771, -1, 31, UINT32_MAX },
        { 9, 71257, "CacheMissForN", 2709710687, -1, 32, UINT32_MAX },
        { 9, 71260, "BeginBuildN", 2709710687, -1, 32, UINT32_MAX },
        { 4, 71734, "PWonRaceOverP", 644542265, -1, 30, 24 },
        { 4, 71743, "DeletionP", 644542265, -1, 24, UINT32_MAX },
        { 5, 71750, "CacheMissForP", 2491771, -1, 33, UINT32_MAX },
        { 5, 71752, "BeginBuildP", 2491771, -1, 33, UINT32_MAX },
        { 8, 74663, "PurgedN", 1301841557, -1, 29, UINT32_MAX },
        { 5, 74671, "AddedP", 2491771, -1, 33, UINT32_MAX },
        { 6, 74684, "CacheMissForP", 3234710487, -1, 34, UINT32_MAX },
        { 6, 74686, "BeginBuildP", 3234710487, -1, 34, UINT32_MAX },
        { 5, 75621, "PWonRaceOverP", 2491771, -1, 33, 31 },
        { 5, 75628, "DeletionP", 2491771, -1, 31, UINT32_MAX },
        { 6, 75641, "CacheMissForP", 3234710487, -1, 35, UINT32_MAX },
        { 6, 75642, "BeginBuildP", 3234710487, -1, 35, UINT32_MAX },
        { 7, 75700, "WaitEndN", 1997496709, -1, 13, UINT32_MAX },
        { 8, 75783, "WaitBeginN", 1301841557, -1, 15, UINT32_MAX },
        { 8, 75801, "WaitEndN", 1301841557, -1, 15, UINT32_MAX },
        { 9, 75823, "WaitBeginN", 2709710687, -1, 16, UINT32_MAX },
        { 9, 75838, "WaitEndN", 2709710687, -1, 16, UINT32_MAX },
        { 2, 75860, "WaitBeginN", 1147256140, -1, 14, UINT32_MAX },
        { 2, 75876, "WaitEndN", 1147256140, -1, 14, UINT32_MAX },
        { 10, 75896, "WaitBeginN", 138923328, -1, 17, UINT32_MAX },
        { 4, 77467, "BadPurgedP", 644542265, -1, 30, UINT32_MAX },
        { 4, 77469, "DeletionP", 644542265, -1, 30, UINT32_MAX },
        { 6, 77471, "AddedP", 3234710487, -1, 34, UINT32_MAX },
        { 7, 77476, "CacheMissForP", 1997496709, -1, 36, UINT32_MAX },
        { 7, 77477, "BeginBuildP", 1997496709, -1, 36, UINT32_MAX },
        { 6, 78452, "PWonRaceOverP", 3234710487, -1, 34, 35 },
        { 6, 78458, "DeletionP", 3234710487, -1, 35, UINT32_MAX },
        { 7, 78463, "CacheMissForP", 1997496709, -1, 37, UINT32_MAX },
        { 7, 78464, "BeginBuildP", 1997496709, -1, 37, UINT32_MAX },
        { 5, 80317, "BadPurgedP", 2491771, -1, 33, UINT32_MAX },
        { 5, 80319, "DeletionP", 2491771, -1, 33, UINT32_MAX },
        { 7, 80320, "AddedP", 1997496709, -1, 36, UINT32_MAX },
        { 8, 80325, "CacheMissForP", 1301841557, -1, 38, UINT32_MAX },
        { 8, 80326, "BeginBuildP", 1301841557, -1, 38, UINT32_MAX },
        { 6, 80465, "BadPurgedP", 3234710487, -1, 34, UINT32_MAX },
        { 6, 80470, "DeletionP", 3234710487, -1, 34, UINT32_MAX },
        { 9, 83470, "AddedN", 2709710687, -1, 32, UINT32_MAX },
        { 7, 83476, "PWonRaceOverP", 1997496709, -1, 36, 37 },
        { 7, 83482, "DeletionP", 1997496709, -1, 37, UINT32_MAX },
        { 9, 83483, "PurgedN", 2709710687, -1, 32, UINT32_MAX },
        { 8, 83489, "AddedP", 1301841557, -1, 38, UINT32_MAX },
        { 8, 83491, "CacheHitForP", 1301841557, -1, 38, UINT32_MAX },
        { 9, 83500, "CacheMissForP", 2709710687, -1, 39, UINT32_MAX },
        { 9, 83501, "BeginBuildP", 2709710687, -1, 39, UINT32_MAX },
        { 9, 83501, "CacheMissForP", 2709710687, -1, 40, UINT32_MAX },
        { 9, 83506, "BeginBuildP", 2709710687, -1, 40, UINT32_MAX },
        { 10, 84152, "CacheMissForN", 138923328, -1, 41, UINT32_MAX },
        { 10, 84156, "BeginBuildN", 138923328, -1, 41, UINT32_MAX },
        { 10, 82239, "WaitEndN", 138923328, -1, 17, UINT32_MAX },
        { 3, 85056, "WaitBeginN", 883660955, -1, 18, UINT32_MAX },
        { 3, 85092, "WaitEndN", 883660955, -1, 18, UINT32_MAX },
        { 11, 85117, "WaitBeginN", 2121723309, -1, 19, UINT32_MAX },
        { 11, 85133, "WaitEndN", 2121723309, -1, 19, UINT32_MAX },
        { 4, 85155, "WaitBeginN", 644542265, -1, 20, UINT32_MAX },
        { 7, 86294, "BadPurgedP", 1997496709, -1, 36, UINT32_MAX },
        { 7, 86296, "DeletionP", 1997496709, -1, 36, UINT32_MAX },
        { 9, 86297, "AddedP", 2709710687, -1, 40, UINT32_MAX },
        { 9, 86299, "PWonRaceOverP", 2709710687, -1, 40, 39 },
        { 9, 86302, "DeletionP", 2709710687, -1, 39, UINT32_MAX },
        { 10, 86302, "CacheMissForP", 138923328, -1, 42, UINT32_MAX },
        { 10, 86307, "BeginBuildP", 138923328, -1, 42, UINT32_MAX },
        { 10, 86307, "CacheMissForP", 138923328, -1, 43, UINT32_MAX },
        { 10, 86312, "BeginBuildP", 138923328, -1, 43, UINT32_MAX },
        { 4, 87848, "WaitEndN", 644542265, -1, 20, UINT32_MAX },
        { 5, 87885, "WaitBeginN", 2491771, -1, 22, UINT32_MAX },
        { 5, 87902, "WaitEndN", 2491771, -1, 22, UINT32_MAX },
        { 6, 87911, "WaitBeginN", 3234710487, -1, 25, UINT32_MAX },
        { 6, 87926, "WaitEndN", 3234710487, -1, 25, UINT32_MAX },
        { 7, 87934, "WaitBeginN", 1997496709, -1, 27, UINT32_MAX },
        { 7, 87948, "WaitEndN", 1997496709, -1, 27, UINT32_MAX },
        { 8, 87955, "WaitBeginN", 1301841557, -1, 29, UINT32_MAX },
        { 8, 89208, "PurgedP", 1301841557, -1, 38, UINT32_MAX },
        { 8, 89218, "DeletionP", 1301841557, -1, 38, UINT32_MAX },
        { 10, 89219, "AddedP", 138923328, -1, 43, UINT32_MAX },
        { 10, 89221, "PWonRaceOverP", 138923328, -1, 43, 42 },
        { 10, 89223, "DeletionP", 138923328, -1, 42, UINT32_MAX },
        { 11, 89224, "CacheMissForP", 2121723309, -1, 44, UINT32_MAX },
        { 11, 89226, "CacheMissForP", 2121723309, -1, 45, UINT32_MAX },
        { 11, 89228, "BeginBuildP", 2121723309, -1, 45, UINT32_MAX },
        { 11, 89228, "BeginBuildP", 2121723309, -1, 44, UINT32_MAX },
        { 10, 89724, "PWonRaceOverN", 138923328, -1, 43, 41 },
        { 10, 89741, "WaitBeginN", 138923328, -1, 41, UINT32_MAX },
        { 10, 89758, "WaitEndN", 138923328, -1, 41, UINT32_MAX },
        { 10, 89761, "DeletionN", 138923328, -1, 41, UINT32_MAX },
        { 11, 90191, "CacheMissForN", 2121723309, -1, 46, UINT32_MAX },
        { 11, 90197, "BeginBuildN", 2121723309, -1, 46, UINT32_MAX },
        { 9, 92145, "BadPurgedP", 2709710687, -1, 40, UINT32_MAX },
        { 9, 92147, "DeletionP", 2709710687, -1, 40, UINT32_MAX },
        { 11, 92148, "AddedP", 2121723309, -1, 45, UINT32_MAX },
        { 11, 94593, "PWonRaceOverP", 2121723309, -1, 45, 44 },
        { 11, 94598, "DeletionP", 2121723309, -1, 44, UINT32_MAX },
        { 11, 95758, "PWonRaceOverN", 2121723309, -1, 45, 46 },
        { 11, 95796, "WaitBeginN", 2121723309, -1, 46, UINT32_MAX },
        { 11, 95948, "WaitEndN", 2121723309, -1, 46, UINT32_MAX },
        { 11, 95965, "DeletionN", 2121723309, -1, 46, UINT32_MAX },
        { 8, 99431, "WaitEndN", 1301841557, -1, 29, UINT32_MAX },
        { 9, 99467, "WaitBeginN", 2709710687, -1, 32, UINT32_MAX },
        { 9, 101677, "WaitEndN", 2709710687, -1, 32, UINT32_MAX },
        { 1, 107862, "DeletionN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 107891, "DeletionN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 107910, "DeletionN", 883660955, -1, 6, UINT32_MAX },
        { 4, 107927, "DeletionN", 644542265, -1, 7, UINT32_MAX },
        { 5, 107945, "DeletionN", 2491771, -1, 8, UINT32_MAX },
        { 0, 107973, "DeletionN", 477900207, -1, 2, UINT32_MAX },
        { 6, 107989, "DeletionN", 3234710487, -1, 9, UINT32_MAX },
        { 1, 108016, "DeletionN", 2189112322, -1, 11, UINT32_MAX },
        { 7, 108034, "DeletionN", 1997496709, -1, 13, UINT32_MAX },
        { 8, 108052, "DeletionN", 1301841557, -1, 15, UINT32_MAX },
        { 9, 108067, "DeletionN", 2709710687, -1, 16, UINT32_MAX },
        { 2, 108091, "DeletionN", 1147256140, -1, 14, UINT32_MAX },
        { 10, 108109, "DeletionN", 138923328, -1, 17, UINT32_MAX },
        { 3, 108132, "DeletionN", 883660955, -1, 18, UINT32_MAX },
        { 11, 108180, "DeletionN", 2121723309, -1, 19, UINT32_MAX },
        { 4, 108235, "DeletionN", 644542265, -1, 20, UINT32_MAX },
        { 5, 108259, "DeletionN", 2491771, -1, 22, UINT32_MAX },
        { 6, 108279, "DeletionN", 3234710487, -1, 25, UINT32_MAX },
        { 7, 108297, "DeletionN", 1997496709, -1, 27, UINT32_MAX },
        { 8, 108319, "DeletionN", 1301841557, -1, 29, UINT32_MAX },
        { 9, 108345, "DeletionN", 2709710687, -1, 32, UINT32_MAX },
        { 11, 108409, "DeletionP", 2121723309, -1, 45, UINT32_MAX },
        { 10, 108426, "DeletionP", 138923328, -1, 43, UINT32_MAX },
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
 0:  477900207 [  293 CacheMissForP 0][  302 BeginBuildP 0][  368 CacheMissForP 1][  381 BeginBuildP 1][ 2523 CacheMissForN 2][ 2530 BeginBuildN 2][ 2623 CacheMissForN 3][ 2625 BeginBuildN 3][ 8840 AddedN 2][ 8881 NWonRaceOverN 2 3][ 8889 WaitBeginN 3][11329 WaitBeginN 2][15040 PurgedN 2][29764 AddedP 1][30250 WaitEndN 3][30255 DeletionN 3][31148 WaitEndN 2][31288 PWonRaceOverP 1 0][31292 DeletionP 0][33788 BadPurgedP 1][33800 DeletionP 1][107973 DeletionN 2]
 1: 2189112322 [ 9066 CacheMissForN 4][ 9070 BeginBuildN 4][12036 AddedN 4][19064 PurgedN 4][29772 CacheMissForP 10][29774 BeginBuildP 10][30386 CacheMissForN 11][30388 BeginBuildN 11][31205 WaitBeginN 4][31298 CacheMissForP 12][31300 BeginBuildP 12][33802 AddedN 11][38349 PurgedN 11][39614 WaitEndN 4][48481 AddedP 10][51448 BadPurgedP 10][51457 DeletionP 10][57596 AddedP 12][60353 BadPurgedP 12][60359 DeletionP 12][64677 WaitBeginN 11][64697 WaitEndN 11][107862 DeletionN 4][108016 DeletionN 11]
 2: 1147256140 [12112 CacheMissForN 5][12114 BeginBuildN 5][15045 AddedN 5][26421 PurgedN 5][33963 CacheMissForN 14][33967 BeginBuildN 14][39770 WaitBeginN 5][39812 WaitEndN 5][41683 AddedN 14][44538 PurgedN 14][48489 CacheMissForP 21][48490 BeginBuildP 21][51458 AddedP 21][54665 BadPurgedP 21][54676 DeletionP 21][57603 CacheMissForP 26][57604 BeginBuildP 26][60361 AddedP 26][63241 BadPurgedP 26][64503 DeletionP 26][75860 WaitBeginN 14][75875 WaitEndN 14][107891 DeletionN 5][108091 DeletionN 14]
 3:  883660955 [15124 CacheMissForN 6][15126 BeginBuildN 6][19073 AddedN 6][29459 PurgedN 6][39838 WaitBeginN 6][40175 WaitEndN 6][41806 CacheMissForN 18][41808 BeginBuildN 18][44544 AddedN 18][48472 PurgedN 18][51472 CacheMissForP 23][51473 BeginBuildP 23][54215 AddedP 23][57582 BadPurgedP 23][57594 DeletionP 23][60373 CacheMissForP 28][60374 BeginBuildP 28][64575 AddedP 28][71096 BadPurgedP 28][71104 DeletionP 28][85056 WaitBeginN 18][85092 WaitEndN 18][107910 DeletionN 6][108132 DeletionN 18]
 4:  644542265 [19201 CacheMissForN 7][19203 BeginBuildN 7][26431 AddedN 7][29757 PurgedN 7][40216 WaitBeginN 7][44671 CacheMissForN 20][44674 BeginBuildN 20][49911 AddedN 20][54204 PurgedN 20][54220 CacheMissForP 24][54222 BeginBuildP 24][61971 WaitEndN 7][64589 CacheMissForP 30][64590 BeginBuildP 30][71106 AddedP 30][71734 PWonRaceOverP 30 24][71743 DeletionP 24][77456 BadPurgedP 30][77469 DeletionP 30][85155 WaitBeginN 20][87848 WaitEndN 20][107927 DeletionN 7][108235 DeletionN 20]
 5:    2491771 [26584 CacheMissForN 8][26586 BeginBuildN 8][29466 AddedN 8][32357 PurgedN 8][50043 CacheMissForN 22][50046 BeginBuildN 22][54679 AddedN 22][57969 PurgedN 22][64600 WaitBeginN 8][64619 WaitEndN 8][71113 CacheMissForP 31][71114 BeginBuildP 31][71750 CacheMissForP 33][71752 BeginBuildP 33][74671 AddedP 33][75621 PWonRaceOverP 33 31][75628 DeletionP 31][80312 BadPurgedP 33][80319 DeletionP 33][87885 WaitBeginN 22][87902 WaitEndN 22][107945 DeletionN 8][108259 DeletionN 22]
 6: 3234710487 [29584 CacheMissForN 9][29587 BeginBuildN 9][32364 AddedN 9][35351 PurgedN 9][54818 CacheMissForN 25][54820 BeginBuildN 25][57972 AddedN 25][61079 PurgedN 25][64661 WaitBeginN 9][64668 WaitEndN 9][74684 CacheMissForP 34][74686 BeginBuildP 34][75641 CacheMissForP 35][75642 BeginBuildP 35][77471 AddedP 34][78452 PWonRaceOverP 34 35][78458 DeletionP 35][80459 BadPurgedP 34][80470 DeletionP 34][87911 WaitBeginN 25][87926 WaitEndN 25][107989 DeletionN 9][108279 DeletionN 25]
 7: 1997496709 [32474 CacheMissForN 13][32476 BeginBuildN 13][35359 AddedN 13][41362 PurgedN 13][58093 CacheMissForN 27][58095 BeginBuildN 27][61090 AddedN 27][64706 WaitBeginN 13][66064 PurgedN 27][75700 WaitEndN 13][77476 CacheMissForP 36][77477 BeginBuildP 36][78463 CacheMissForP 37][78464 BeginBuildP 37][80320 AddedP 36][83476 PWonRaceOverP 36 37][83482 DeletionP 37][86287 BadPurgedP 36][86296 DeletionP 36][87934 WaitBeginN 27][87948 WaitEndN 27][108034 DeletionN 13][108297 DeletionN 27]
 8: 1301841557 [35460 CacheMissForN 15][35463 BeginBuildN 15][38356 AddedN 15][41679 PurgedN 15][61234 CacheMissForN 29][63241 BeginBuildN 29][66069 AddedN 29][74663 PurgedN 29][75783 WaitBeginN 15][75801 WaitEndN 15][80325 CacheMissForP 38][80326 BeginBuildP 38][83489 AddedP 38][83491 CacheHitForP 38][87955 WaitBeginN 29][89208 PurgedP 38][89218 DeletionP 38][99431 WaitEndN 29][108052 DeletionN 15][108319 DeletionN 29]
 9: 2709710687 [38486 CacheMissForN 16][38489 BeginBuildN 16][41367 AddedN 16][44511 PurgedN 16][71257 CacheMissForN 32][71260 BeginBuildN 32][75823 WaitBeginN 16][75838 WaitEndN 16][83469 AddedN 32][83483 PurgedN 32][83500 CacheMissForP 39][83501 BeginBuildP 39][83501 CacheMissForP 40][83506 BeginBuildP 40][86297 AddedP 40][86299 PWonRaceOverP 40 39][86302 DeletionP 39][92139 BadPurgedP 40][92147 DeletionP 40][99467 WaitBeginN 32][101677 WaitEndN 32][108067 DeletionN 16][108345 DeletionN 32]
10:  138923328 [41481 CacheMissForN 17][41483 BeginBuildN 17][44518 AddedN 17][47663 PurgedN 17][75896 WaitBeginN 17][82239 WaitEndN 17][84152 CacheMissForN 41][84156 BeginBuildN 41][86302 CacheMissForP 42][86307 BeginBuildP 42][86307 CacheMissForP 43][86312 BeginBuildP 43][89219 AddedP 43][89221 PWonRaceOverP 43 42][89223 DeletionP 42][89724 PWonRaceOverN 43 41][89741 WaitBeginN 41][89758 WaitEndN 41][89761 DeletionN 41][108109 DeletionN 17][108426 DeletionP 43]
11: 2121723309 [44633 CacheMissForN 19][44636 BeginBuildN 19][47669 AddedN 19][49904 PurgedN 19][85116 WaitBeginN 19][85133 WaitEndN 19][89224 CacheMissForP 44][89226 CacheMissForP 45][89228 BeginBuildP 45][89228 BeginBuildP 44][90191 CacheMissForN 46][90196 BeginBuildN 46][92148 AddedP 45][94593 PWonRaceOverP 45 44][94598 DeletionP 44][95758 PWonRaceOverN 45 46][95796 WaitBeginN 46][95948 WaitEndN 46][95965 DeletionN 46][108180 DeletionN 19][108409 DeletionP 45]
#endif

#if 0
Reconstructed
 0:  477900207 [  293 CacheMissForP 0][  302 BeginBuildP 0][  368 CacheMissForP 1][  382 BeginBuildP 1][ 2523 CacheMissForN 2][ 2530 BeginBuildN 2][ 2623 CacheMissForN 3][ 2625 BeginBuildN 3][ 8840 AddedN 2][ 8881 NWonRaceOverN 2 3][ 8889 WaitBeginN 3][11329 WaitBeginN 2][15040 PurgedN 2][29764 AddedP 1][30250 WaitEndN 3][30255 DeletionN 3][31149 WaitEndN 2][31288 PWonRaceOverP 1 0][31292 DeletionP 0][33796 BadPurgedP 1][33800 DeletionP 1][107973 DeletionN 2]
 1: 2189112322 [ 9067 CacheMissForN 4][ 9070 BeginBuildN 4][12036 AddedN 4][19064 PurgedN 4][29772 CacheMissForP 10][29774 BeginBuildP 10][30386 CacheMissForN 11][30388 BeginBuildN 11][31205 WaitBeginN 4][31298 CacheMissForP 12][31300 BeginBuildP 12][33802 AddedN 11][38349 PurgedN 11][39614 WaitEndN 4][48481 AddedP 10][51454 BadPurgedP 10][51457 DeletionP 10][57596 AddedP 12][60357 BadPurgedP 12][60359 DeletionP 12][64677 WaitBeginN 11][64697 WaitEndN 11][107862 DeletionN 4][108016 DeletionN 11]
 2: 1147256140 [12112 CacheMissForN 5][12114 BeginBuildN 5][15045 AddedN 5][26421 PurgedN 5][33963 CacheMissForN 14][33967 BeginBuildN 14][39770 WaitBeginN 5][39813 WaitEndN 5][41684 AddedN 14][44538 PurgedN 14][48489 CacheMissForP 21][48490 BeginBuildP 21][51458 AddedP 21][54673 BadPurgedP 21][54676 DeletionP 21][57603 CacheMissForP 26][57604 BeginBuildP 26][60361 AddedP 26][63251 BadPurgedP 26][64503 DeletionP 26][75860 WaitBeginN 14][75876 WaitEndN 14][107891 DeletionN 5][108091 DeletionN 14]
 3:  883660955 [15124 CacheMissForN 6][15126 BeginBuildN 6][19073 AddedN 6][29459 PurgedN 6][39838 WaitBeginN 6][40175 WaitEndN 6][41806 CacheMissForN 18][41808 BeginBuildN 18][44544 AddedN 18][48472 PurgedN 18][51472 CacheMissForP 23][51473 BeginBuildP 23][54215 AddedP 23][57592 BadPurgedP 23][57595 DeletionP 23][60373 CacheMissForP 28][60374 BeginBuildP 28][64576 AddedP 28][71100 BadPurgedP 28][71104 DeletionP 28][85056 WaitBeginN 18][85092 WaitEndN 18][107910 DeletionN 6][108132 DeletionN 18]
 4:  644542265 [19201 CacheMissForN 7][19203 BeginBuildN 7][26431 AddedN 7][29757 PurgedN 7][40216 WaitBeginN 7][44671 CacheMissForN 20][44674 BeginBuildN 20][49911 AddedN 20][54204 PurgedN 20][54220 CacheMissForP 24][54222 BeginBuildP 24][61971 WaitEndN 7][64589 CacheMissForP 30][64590 BeginBuildP 30][71106 AddedP 30][71734 PWonRaceOverP 30 24][71743 DeletionP 24][77467 BadPurgedP 30][77469 DeletionP 30][85155 WaitBeginN 20][87848 WaitEndN 20][107927 DeletionN 7][108235 DeletionN 20]
 5:    2491771 [26584 CacheMissForN 8][26586 BeginBuildN 8][29466 AddedN 8][32357 PurgedN 8][50043 CacheMissForN 22][50046 BeginBuildN 22][54679 AddedN 22][57969 PurgedN 22][64600 WaitBeginN 8][64619 WaitEndN 8][71113 CacheMissForP 31][71114 BeginBuildP 31][71750 CacheMissForP 33][71752 BeginBuildP 33][74671 AddedP 33][75621 PWonRaceOverP 33 31][75628 DeletionP 31][80317 BadPurgedP 33][80319 DeletionP 33][87885 WaitBeginN 22][87902 WaitEndN 22][107945 DeletionN 8][108259 DeletionN 22]
 6: 3234710487 [29584 CacheMissForN 9][29587 BeginBuildN 9][32364 AddedN 9][35351 PurgedN 9][54818 CacheMissForN 25][54820 BeginBuildN 25][57972 AddedN 25][61079 PurgedN 25][64661 WaitBeginN 9][64668 WaitEndN 9][74684 CacheMissForP 34][74686 BeginBuildP 34][75641 CacheMissForP 35][75642 BeginBuildP 35][77471 AddedP 34][78452 PWonRaceOverP 34 35][78458 DeletionP 35][80465 BadPurgedP 34][80470 DeletionP 34][87911 WaitBeginN 25][87926 WaitEndN 25][107989 DeletionN 9][108279 DeletionN 25]
 7: 1997496709 [32474 CacheMissForN 13][32476 BeginBuildN 13][35359 AddedN 13][41362 PurgedN 13][58093 CacheMissForN 27][58095 BeginBuildN 27][61090 AddedN 27][64706 WaitBeginN 13][66064 PurgedN 27][75700 WaitEndN 13][77476 CacheMissForP 36][77477 BeginBuildP 36][78463 CacheMissForP 37][78464 BeginBuildP 37][80320 AddedP 36][83476 PWonRaceOverP 36 37][83482 DeletionP 37][86294 BadPurgedP 36][86296 DeletionP 36][87934 WaitBeginN 27][87948 WaitEndN 27][108034 DeletionN 13][108297 DeletionN 27]
 8: 1301841557 [35460 CacheMissForN 15][35463 BeginBuildN 15][38356 AddedN 15][41679 PurgedN 15][61234 CacheMissForN 29][63241 BeginBuildN 29][66069 AddedN 29][74663 PurgedN 29][75783 WaitBeginN 15][75801 WaitEndN 15][80325 CacheMissForP 38][80326 BeginBuildP 38][83489 AddedP 38][83491 CacheHitForP 38][87955 WaitBeginN 29][89208 PurgedP 38][89218 DeletionP 38][99431 WaitEndN 29][108052 DeletionN 15][108319 DeletionN 29]
 9: 2709710687 [38486 CacheMissForN 16][38489 BeginBuildN 16][41367 AddedN 16][44511 PurgedN 16][71257 CacheMissForN 32][71260 BeginBuildN 32][75823 WaitBeginN 16][75838 WaitEndN 16][83470 AddedN 32][83483 PurgedN 32][83500 CacheMissForP 39][83501 BeginBuildP 39][83501 CacheMissForP 40][83506 BeginBuildP 40][86297 AddedP 40][86299 PWonRaceOverP 40 39][86302 DeletionP 39][92145 BadPurgedP 40][92147 DeletionP 40][99467 WaitBeginN 32][101677 WaitEndN 32][108067 DeletionN 16][108345 DeletionN 32]
10:  138923328 [41481 CacheMissForN 17][41483 BeginBuildN 17][44518 AddedN 17][47663 PurgedN 17][75896 WaitBeginN 17][84152 CacheMissForN 41][84156 BeginBuildN 41][82239 WaitEndN 17][86302 CacheMissForP 42][86307 BeginBuildP 42][86307 CacheMissForP 43][86312 BeginBuildP 43][89219 AddedP 43][89221 PWonRaceOverP 43 42][89223 DeletionP 42][89724 PWonRaceOverN 43 41][89741 WaitBeginN 41][89758 WaitEndN 41][89761 DeletionN 41][108109 DeletionN 17][108426 DeletionP 43]
11: 2121723309 [44633 CacheMissForN 19][44636 BeginBuildN 19][47669 AddedN 19][49904 PurgedN 19][85117 WaitBeginN 19][85133 WaitEndN 19][89224 CacheMissForP 44][89226 CacheMissForP 45][89228 BeginBuildP 45][89228 BeginBuildP 44][90191 CacheMissForN 46][90197 BeginBuildN 46][92148 AddedP 45][94593 PWonRaceOverP 45 44][94598 DeletionP 44][95758 PWonRaceOverN 45 46][95796 WaitBeginN 46][95948 WaitEndN 46][95965 DeletionN 46][108180 DeletionN 19][108409 DeletionP 45]
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
