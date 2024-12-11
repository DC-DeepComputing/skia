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
        { 0, 933, "CacheMissForP", 477900207, -1, 0, UINT32_MAX },
        { 0, 949, "CacheMissForP", 477900207, -1, 1, UINT32_MAX },
        { 0, 952, "BeginBuildP", 477900207, -1, 0, UINT32_MAX },
        { 0, 953, "BeginBuildP", 477900207, -1, 1, UINT32_MAX },
        { 0, 35823, "CacheMissForN", 477900207, -1, 2, UINT32_MAX },
        { 0, 35834, "CacheMissForN", 477900207, -1, 3, UINT32_MAX },
        { 0, 35838, "BeginBuildN", 477900207, -1, 2, UINT32_MAX },
        { 0, 35839, "BeginBuildN", 477900207, -1, 3, UINT32_MAX },
        { 0, 73753, "AddedN", 477900207, -1, 3, UINT32_MAX },
        { 0, 73772, "NWonRaceOverN", 477900207, -1, 3, 2 },
        { 0, 73783, "WaitBeginN", 477900207, -1, 2, UINT32_MAX },
        { 1, 74697, "CacheMissForN", 2189112322, -1, 4, UINT32_MAX },
        { 1, 74707, "BeginBuildN", 2189112322, -1, 4, UINT32_MAX },
        { 0, 74939, "WaitBeginN", 477900207, -1, 3, UINT32_MAX },
        { 1, 109441, "AddedN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 109801, "CacheMissForN", 1147256140, -1, 5, UINT32_MAX },
        { 2, 109806, "BeginBuildN", 1147256140, -1, 5, UINT32_MAX },
        { 0, 145480, "PurgedN", 477900207, 0, 3, UINT32_MAX },
        { 2, 145492, "AddedN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 145937, "CacheMissForN", 883660955, -1, 6, UINT32_MAX },
        { 3, 145943, "BeginBuildN", 883660955, -1, 6, UINT32_MAX },
        { 1, 180177, "PurgedN", 2189112322, 0, 4, UINT32_MAX },
        { 3, 180189, "AddedN", 883660955, -1, 6, UINT32_MAX },
        { 4, 180580, "CacheMissForN", 644542265, -1, 7, UINT32_MAX },
        { 4, 180583, "BeginBuildN", 644542265, -1, 7, UINT32_MAX },
        { 2, 217535, "PurgedN", 1147256140, 0, 5, UINT32_MAX },
        { 4, 217547, "AddedN", 644542265, -1, 7, UINT32_MAX },
        { 5, 218031, "CacheMissForN", 2491771, -1, 8, UINT32_MAX },
        { 5, 218038, "BeginBuildN", 2491771, -1, 8, UINT32_MAX },
        { 3, 236810, "PurgedN", 883660955, 0, 6, UINT32_MAX },
        { 0, 236823, "AddedP", 477900207, -1, 0, UINT32_MAX },
        { 1, 236861, "CacheMissForP", 2189112322, -1, 9, UINT32_MAX },
        { 1, 236864, "BeginBuildP", 2189112322, -1, 9, UINT32_MAX },
        { 0, 237554, "PWonRaceOverP", 477900207, -1, 0, 1 },
        { 0, 237576, "DeletionP", 477900207, -1, 1, UINT32_MAX },
        { 1, 237645, "CacheMissForP", 2189112322, -1, 10, UINT32_MAX },
        { 1, 237654, "BeginBuildP", 2189112322, -1, 10, UINT32_MAX },
        { 0, 239685, "WaitEndN", 477900207, -1, 3, UINT32_MAX },
        { 1, 240799, "WaitBeginN", 2189112322, -1, 4, UINT32_MAX },
        { 0, 245155, "WaitEndN", 477900207, -1, 2, UINT32_MAX },
        { 0, 245164, "DeletionN", 477900207, -1, 2, UINT32_MAX },
        { 1, 245652, "CacheMissForN", 2189112322, -1, 11, UINT32_MAX },
        { 1, 245657, "BeginBuildN", 2189112322, -1, 11, UINT32_MAX },
        { 4, 254332, "PurgedN", 644542265, 0, 7, UINT32_MAX },
        { 5, 254341, "AddedN", 2491771, -1, 8, UINT32_MAX },
        { 6, 254686, "CacheMissForN", 3234710487, -1, 12, UINT32_MAX },
        { 6, 254689, "BeginBuildN", 3234710487, -1, 12, UINT32_MAX },
        { 1, 275621, "WaitEndN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 275878, "WaitBeginN", 1147256140, -1, 5, UINT32_MAX },
        { 0, 291007, "PurgedP", 477900207, 0, 0, UINT32_MAX },
        { 0, 291012, "DeletionP", 477900207, -1, 0, UINT32_MAX },
        { 1, 291015, "AddedN", 2189112322, -1, 11, UINT32_MAX },
        { 2, 291556, "CacheMissForN", 1147256140, -1, 13, UINT32_MAX },
        { 2, 291567, "BeginBuildN", 1147256140, -1, 13, UINT32_MAX },
        { 5, 296278, "PurgedN", 2491771, 0, 8, UINT32_MAX },
        { 6, 296286, "AddedN", 3234710487, -1, 12, UINT32_MAX },
        { 7, 296610, "CacheMissForN", 1997496709, -1, 14, UINT32_MAX },
        { 7, 296615, "BeginBuildN", 1997496709, -1, 14, UINT32_MAX },
        { 2, 311231, "WaitEndN", 1147256140, -1, 5, UINT32_MAX },
        { 3, 312436, "WaitBeginN", 883660955, -1, 6, UINT32_MAX },
        { 1, 329024, "PurgedN", 2189112322, 0, 11, UINT32_MAX },
        { 2, 329038, "AddedN", 1147256140, -1, 13, UINT32_MAX },
        { 3, 329523, "CacheMissForN", 883660955, -1, 15, UINT32_MAX },
        { 3, 329530, "BeginBuildN", 883660955, -1, 15, UINT32_MAX },
        { 6, 333680, "PurgedN", 3234710487, 0, 12, UINT32_MAX },
        { 7, 333690, "AddedN", 1997496709, -1, 14, UINT32_MAX },
        { 8, 334056, "CacheMissForN", 1301841557, -1, 16, UINT32_MAX },
        { 8, 334061, "BeginBuildN", 1301841557, -1, 16, UINT32_MAX },
        { 3, 347452, "WaitEndN", 883660955, -1, 6, UINT32_MAX },
        { 4, 347709, "WaitBeginN", 644542265, -1, 7, UINT32_MAX },
        { 2, 363689, "PurgedN", 1147256140, 0, 13, UINT32_MAX },
        { 3, 363701, "AddedN", 883660955, -1, 15, UINT32_MAX },
        { 4, 364072, "CacheMissForN", 644542265, -1, 17, UINT32_MAX },
        { 4, 364077, "BeginBuildN", 644542265, -1, 17, UINT32_MAX },
        { 7, 370777, "PurgedN", 1997496709, 0, 14, UINT32_MAX },
        { 8, 370788, "AddedN", 1301841557, -1, 16, UINT32_MAX },
        { 9, 371207, "CacheMissForN", 2709710687, -1, 18, UINT32_MAX },
        { 9, 371213, "BeginBuildN", 2709710687, -1, 18, UINT32_MAX },
        { 4, 405803, "WaitEndN", 644542265, -1, 7, UINT32_MAX },
        { 3, 407383, "PurgedN", 883660955, 0, 15, UINT32_MAX },
        { 9, 407395, "AddedN", 2709710687, -1, 18, UINT32_MAX },
        { 5, 407414, "WaitBeginN", 2491771, -1, 8, UINT32_MAX },
        { 10, 407833, "CacheMissForN", 138923328, -1, 19, UINT32_MAX },
        { 10, 407839, "BeginBuildN", 138923328, -1, 19, UINT32_MAX },
        { 8, 413687, "PurgedN", 1301841557, 0, 16, UINT32_MAX },
        { 4, 413700, "AddedN", 644542265, -1, 17, UINT32_MAX },
        { 5, 414051, "CacheMissForN", 2491771, -1, 20, UINT32_MAX },
        { 5, 414056, "BeginBuildN", 2491771, -1, 20, UINT32_MAX },
        { 5, 419953, "WaitEndN", 2491771, -1, 8, UINT32_MAX },
        { 1, 420171, "WaitBeginN", 2189112322, -1, 11, UINT32_MAX },
        { 1, 420201, "WaitEndN", 2189112322, -1, 11, UINT32_MAX },
        { 6, 420324, "WaitBeginN", 3234710487, -1, 12, UINT32_MAX },
        { 9, 439919, "PurgedN", 2709710687, 0, 18, UINT32_MAX },
        { 1, 439936, "AddedP", 2189112322, -1, 10, UINT32_MAX },
        { 2, 439969, "CacheMissForP", 1147256140, -1, 21, UINT32_MAX },
        { 2, 439972, "BeginBuildP", 1147256140, -1, 21, UINT32_MAX },
        { 1, 444754, "PWonRaceOverP", 2189112322, -1, 10, 9 },
        { 1, 444772, "DeletionP", 2189112322, -1, 9, UINT32_MAX },
        { 2, 444801, "CacheMissForP", 1147256140, -1, 22, UINT32_MAX },
        { 2, 444805, "BeginBuildP", 1147256140, -1, 22, UINT32_MAX },
        { 4, 449500, "PurgedN", 644542265, 0, 17, UINT32_MAX },
        { 10, 449507, "AddedN", 138923328, -1, 19, UINT32_MAX },
        { 11, 449928, "CacheMissForN", 2121723309, -1, 23, UINT32_MAX },
        { 11, 449933, "BeginBuildN", 2121723309, -1, 23, UINT32_MAX },
        { 1, 450013, "PurgedP", 2189112322, 0, 10, UINT32_MAX },
        { 1, 450018, "DeletionP", 2189112322, -1, 10, UINT32_MAX },
        { 5, 450021, "AddedN", 2491771, -1, 20, UINT32_MAX },
        { 6, 450409, "CacheMissForN", 3234710487, -1, 24, UINT32_MAX },
        { 6, 450415, "BeginBuildN", 3234710487, -1, 24, UINT32_MAX },
        { 10, 475758, "PurgedN", 138923328, 0, 19, UINT32_MAX },
        { 2, 475771, "AddedP", 1147256140, -1, 21, UINT32_MAX },
        { 3, 475847, "CacheMissForP", 883660955, -1, 25, UINT32_MAX },
        { 3, 475850, "BeginBuildP", 883660955, -1, 25, UINT32_MAX },
        { 6, 477939, "WaitEndN", 3234710487, -1, 12, UINT32_MAX },
        { 2, 478340, "WaitBeginN", 1147256140, -1, 13, UINT32_MAX },
        { 2, 478383, "WaitEndN", 1147256140, -1, 13, UINT32_MAX },
        { 7, 478505, "WaitBeginN", 1997496709, -1, 14, UINT32_MAX },
        { 2, 480403, "PWonRaceOverP", 1147256140, -1, 21, 22 },
        { 2, 480415, "DeletionP", 1147256140, -1, 22, UINT32_MAX },
        { 3, 480476, "CacheMissForP", 883660955, -1, 26, UINT32_MAX },
        { 3, 480481, "BeginBuildP", 883660955, -1, 26, UINT32_MAX },
        { 5, 487285, "PurgedN", 2491771, 0, 20, UINT32_MAX },
        { 11, 487293, "AddedN", 2121723309, -1, 23, UINT32_MAX },
        { 2, 492911, "PurgedP", 1147256140, 0, 21, UINT32_MAX },
        { 2, 492915, "DeletionP", 1147256140, -1, 21, UINT32_MAX },
        { 6, 492917, "AddedN", 3234710487, -1, 24, UINT32_MAX },
        { 7, 493430, "CacheMissForN", 1997496709, -1, 27, UINT32_MAX },
        { 7, 493441, "BeginBuildN", 1997496709, -1, 27, UINT32_MAX },
        { 11, 510196, "PurgedN", 2121723309, 0, 23, UINT32_MAX },
        { 3, 510208, "AddedP", 883660955, -1, 25, UINT32_MAX },
        { 4, 510234, "CacheMissForP", 644542265, -1, 28, UINT32_MAX },
        { 4, 510237, "BeginBuildP", 644542265, -1, 28, UINT32_MAX },
        { 3, 514604, "PWonRaceOverP", 883660955, -1, 25, 26 },
        { 3, 514612, "DeletionP", 883660955, -1, 26, UINT32_MAX },
        { 4, 514630, "CacheMissForP", 644542265, -1, 29, UINT32_MAX },
        { 4, 514633, "BeginBuildP", 644542265, -1, 29, UINT32_MAX },
        { 7, 520579, "WaitEndN", 1997496709, -1, 14, UINT32_MAX },
        { 3, 520937, "WaitBeginN", 883660955, -1, 15, UINT32_MAX },
        { 3, 520968, "WaitEndN", 883660955, -1, 15, UINT32_MAX },
        { 8, 521089, "WaitBeginN", 1301841557, -1, 16, UINT32_MAX },
        { 6, 531590, "PurgedN", 3234710487, 0, 24, UINT32_MAX },
        { 7, 531600, "AddedN", 1997496709, -1, 27, UINT32_MAX },
        { 8, 532048, "CacheMissForN", 1301841557, -1, 30, UINT32_MAX },
        { 8, 532054, "BeginBuildN", 1301841557, -1, 30, UINT32_MAX },
        { 8, 541736, "WaitEndN", 1301841557, -1, 16, UINT32_MAX },
        { 9, 541951, "WaitBeginN", 2709710687, -1, 18, UINT32_MAX },
        { 3, 546399, "PurgedP", 883660955, 0, 25, UINT32_MAX },
        { 3, 546403, "DeletionP", 883660955, -1, 25, UINT32_MAX },
        { 4, 546405, "AddedP", 644542265, -1, 28, UINT32_MAX },
        { 5, 546437, "CacheMissForP", 2491771, -1, 31, UINT32_MAX },
        { 5, 546441, "BeginBuildP", 2491771, -1, 31, UINT32_MAX },
        { 4, 550741, "PWonRaceOverP", 644542265, -1, 28, 29 },
        { 4, 550751, "DeletionP", 644542265, -1, 29, UINT32_MAX },
        { 5, 550777, "CacheMissForP", 2491771, -1, 32, UINT32_MAX },
        { 5, 550780, "BeginBuildP", 2491771, -1, 32, UINT32_MAX },
        { 7, 569059, "PurgedN", 1997496709, 0, 27, UINT32_MAX },
        { 8, 569071, "AddedN", 1301841557, -1, 30, UINT32_MAX },
        { 9, 569528, "CacheMissForN", 2709710687, -1, 33, UINT32_MAX },
        { 9, 569533, "BeginBuildN", 2709710687, -1, 33, UINT32_MAX },
        { 9, 574156, "WaitEndN", 2709710687, -1, 18, UINT32_MAX },
        { 4, 574391, "WaitBeginN", 644542265, -1, 17, UINT32_MAX },
        { 4, 574417, "WaitEndN", 644542265, -1, 17, UINT32_MAX },
        { 10, 574535, "WaitBeginN", 138923328, -1, 19, UINT32_MAX },
        { 4, 582280, "PurgedP", 644542265, 0, 28, UINT32_MAX },
        { 4, 582284, "DeletionP", 644542265, -1, 28, UINT32_MAX },
        { 5, 582286, "AddedP", 2491771, -1, 31, UINT32_MAX },
        { 6, 582361, "CacheMissForP", 3234710487, -1, 34, UINT32_MAX },
        { 6, 582364, "BeginBuildP", 3234710487, -1, 34, UINT32_MAX },
        { 5, 585954, "PWonRaceOverP", 2491771, -1, 31, 32 },
        { 5, 585963, "DeletionP", 2491771, -1, 32, UINT32_MAX },
        { 6, 586033, "CacheMissForP", 3234710487, -1, 35, UINT32_MAX },
        { 6, 586038, "BeginBuildP", 3234710487, -1, 35, UINT32_MAX },
        { 8, 605065, "PurgedN", 1301841557, 0, 30, UINT32_MAX },
        { 9, 605075, "AddedN", 2709710687, -1, 33, UINT32_MAX },
        { 10, 605571, "CacheMissForN", 138923328, -1, 36, UINT32_MAX },
        { 10, 605583, "BeginBuildN", 138923328, -1, 36, UINT32_MAX },
        { 5, 617690, "PurgedP", 2491771, 0, 31, UINT32_MAX },
        { 5, 617695, "DeletionP", 2491771, -1, 31, UINT32_MAX },
        { 6, 617697, "AddedP", 3234710487, -1, 34, UINT32_MAX },
        { 7, 617725, "CacheMissForP", 1997496709, -1, 37, UINT32_MAX },
        { 7, 617728, "BeginBuildP", 1997496709, -1, 37, UINT32_MAX },
        { 6, 620913, "PWonRaceOverP", 3234710487, -1, 34, 35 },
        { 6, 620922, "DeletionP", 3234710487, -1, 35, UINT32_MAX },
        { 7, 620949, "CacheMissForP", 1997496709, -1, 38, UINT32_MAX },
        { 7, 620951, "BeginBuildP", 1997496709, -1, 38, UINT32_MAX },
        { 10, 628169, "WaitEndN", 138923328, -1, 19, UINT32_MAX },
        { 5, 628389, "WaitBeginN", 2491771, -1, 20, UINT32_MAX },
        { 5, 628411, "WaitEndN", 2491771, -1, 20, UINT32_MAX },
        { 11, 628536, "WaitBeginN", 2121723309, -1, 23, UINT32_MAX },
        { 9, 643293, "PurgedN", 2709710687, 0, 33, UINT32_MAX },
        { 10, 643304, "AddedN", 138923328, -1, 36, UINT32_MAX },
        { 11, 643697, "CacheMissForN", 2121723309, -1, 39, UINT32_MAX },
        { 11, 643704, "BeginBuildN", 2121723309, -1, 39, UINT32_MAX },
        { 6, 653983, "PurgedP", 3234710487, 0, 34, UINT32_MAX },
        { 6, 653987, "DeletionP", 3234710487, -1, 34, UINT32_MAX },
        { 7, 653989, "AddedP", 1997496709, -1, 37, UINT32_MAX },
        { 8, 654014, "CacheMissForP", 1301841557, -1, 40, UINT32_MAX },
        { 8, 654018, "BeginBuildP", 1301841557, -1, 40, UINT32_MAX },
        { 7, 657951, "PWonRaceOverP", 1997496709, -1, 37, 38 },
        { 7, 657960, "DeletionP", 1997496709, -1, 38, UINT32_MAX },
        { 8, 657987, "CacheMissForP", 1301841557, -1, 41, UINT32_MAX },
        { 8, 657990, "BeginBuildP", 1301841557, -1, 41, UINT32_MAX },
        { 11, 660238, "WaitEndN", 2121723309, -1, 23, UINT32_MAX },
        { 6, 660447, "WaitBeginN", 3234710487, -1, 24, UINT32_MAX },
        { 6, 660471, "WaitEndN", 3234710487, -1, 24, UINT32_MAX },
        { 7, 660589, "WaitBeginN", 1997496709, -1, 27, UINT32_MAX },
        { 7, 660608, "WaitEndN", 1997496709, -1, 27, UINT32_MAX },
        { 8, 660756, "WaitBeginN", 1301841557, -1, 30, UINT32_MAX },
        { 8, 660779, "WaitEndN", 1301841557, -1, 30, UINT32_MAX },
        { 9, 660893, "WaitBeginN", 2709710687, -1, 33, UINT32_MAX },
        { 9, 660911, "WaitEndN", 2709710687, -1, 33, UINT32_MAX },
        { 10, 661058, "WaitBeginN", 138923328, -1, 36, UINT32_MAX },
        { 10, 661080, "WaitEndN", 138923328, -1, 36, UINT32_MAX },
        { 10, 681780, "PurgedN", 138923328, 0, 36, UINT32_MAX },
        { 11, 681790, "AddedN", 2121723309, -1, 39, UINT32_MAX },
        { 11, 681994, "WaitBeginN", 2121723309, -1, 39, UINT32_MAX },
        { 11, 682021, "WaitEndN", 2121723309, -1, 39, UINT32_MAX },
        { 7, 690590, "PurgedP", 1997496709, 0, 37, UINT32_MAX },
        { 7, 690598, "DeletionP", 1997496709, -1, 37, UINT32_MAX },
        { 8, 690600, "AddedP", 1301841557, -1, 40, UINT32_MAX },
        { 9, 690695, "CacheMissForP", 2709710687, -1, 42, UINT32_MAX },
        { 9, 690698, "BeginBuildP", 2709710687, -1, 42, UINT32_MAX },
        { 8, 694166, "PWonRaceOverP", 1301841557, -1, 40, 41 },
        { 8, 694176, "DeletionP", 1301841557, -1, 41, UINT32_MAX },
        { 9, 694249, "CacheMissForP", 2709710687, -1, 43, UINT32_MAX },
        { 9, 694252, "BeginBuildP", 2709710687, -1, 43, UINT32_MAX },
        { 11, 726081, "PurgedN", 2121723309, 0, 39, UINT32_MAX },
        { 9, 726094, "AddedP", 2709710687, -1, 42, UINT32_MAX },
        { 10, 726120, "CacheMissForP", 138923328, -1, 44, UINT32_MAX },
        { 10, 726122, "BeginBuildP", 138923328, -1, 44, UINT32_MAX },
        { 9, 729086, "PWonRaceOverP", 2709710687, -1, 42, 43 },
        { 9, 729095, "DeletionP", 2709710687, -1, 43, UINT32_MAX },
        { 10, 729115, "CacheMissForP", 138923328, -1, 45, UINT32_MAX },
        { 10, 729117, "BeginBuildP", 138923328, -1, 45, UINT32_MAX },
        { 8, 763086, "PurgedP", 1301841557, 0, 40, UINT32_MAX },
        { 8, 763090, "DeletionP", 1301841557, -1, 40, UINT32_MAX },
        { 10, 763092, "AddedP", 138923328, -1, 44, UINT32_MAX },
        { 11, 763117, "CacheMissForP", 2121723309, -1, 46, UINT32_MAX },
        { 11, 763119, "BeginBuildP", 2121723309, -1, 46, UINT32_MAX },
        { 10, 765739, "PWonRaceOverP", 138923328, -1, 44, 45 },
        { 10, 765748, "DeletionP", 138923328, -1, 45, UINT32_MAX },
        { 11, 765768, "CacheMissForP", 2121723309, -1, 47, UINT32_MAX },
        { 11, 765770, "BeginBuildP", 2121723309, -1, 47, UINT32_MAX },
        { 9, 799683, "PurgedP", 2709710687, 0, 42, UINT32_MAX },
        { 9, 799687, "DeletionP", 2709710687, -1, 42, UINT32_MAX },
        { 11, 799689, "AddedP", 2121723309, -1, 46, UINT32_MAX },
        { 11, 802116, "PWonRaceOverP", 2121723309, -1, 46, 47 },
        { 11, 802124, "DeletionP", 2121723309, -1, 47, UINT32_MAX },
        { 11, 809889, "DeletionP", 2121723309, -1, 46, UINT32_MAX },
        { 10, 809892, "DeletionP", 138923328, -1, 44, UINT32_MAX },
        { 1, 810393, "DeletionN", 2189112322, -1, 4, UINT32_MAX },
        { 2, 810421, "DeletionN", 1147256140, -1, 5, UINT32_MAX },
        {  3, 810445, "DeletionN", 883660955, -1, 6, UINT32_MAX },
        {  4, 810466, "DeletionN", 644542265, -1, 7, UINT32_MAX },
        {  0, 810535, "DeletionN", 477900207, -1, 3, UINT32_MAX },
        {  5, 810570, "DeletionN", 2491771, -1, 8, UINT32_MAX },
        {  1, 810611, "DeletionN", 2189112322, -1, 11, UINT32_MAX },
        {  6, 810631, "DeletionN", 3234710487, -1, 12, UINT32_MAX },
        {  2, 810666, "DeletionN", 1147256140, -1, 13, UINT32_MAX },
        {  7, 810686, "DeletionN", 1997496709, -1, 14, UINT32_MAX },
        {  3, 810715, "DeletionN", 883660955, -1, 15, UINT32_MAX },
        {  8, 810735, "DeletionN", 1301841557, -1, 16, UINT32_MAX },
        {  9, 810760, "DeletionN", 2709710687, -1, 18, UINT32_MAX },
        {  4, 810791, "DeletionN", 644542265, -1, 17, UINT32_MAX },
        { 10, 810815, "DeletionN", 138923328, -1, 19, UINT32_MAX },
        {  5, 810849, "DeletionN", 2491771, -1, 20, UINT32_MAX },
        { 11, 810914, "DeletionN", 2121723309, -1, 23, UINT32_MAX },
        {  6, 810963, "DeletionN", 3234710487, -1, 24, UINT32_MAX },
        {  7, 810994, "DeletionN", 1997496709, -1, 27, UINT32_MAX },
        {  8, 811024, "DeletionN", 1301841557, -1, 30, UINT32_MAX },
        {  9, 811051, "DeletionN", 2709710687, -1, 33, UINT32_MAX },
        { 10, 811080, "DeletionN", 138923328, -1, 36, UINT32_MAX },
        { 11, 811141, "DeletionN", 2121723309, -1, 39, UINT32_MAX },
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
 0:  477900207 [  933 CacheMissForP 0][  949 CacheMissForP 1][  952 BeginBuildP 0][  952 BeginBuildP 1][35823 CacheMissForN 2][35834 CacheMissForN 3][35838 BeginBuildN 2][35838 BeginBuildN 3][73753 AddedN 3][73772 NWonRaceOverN 3 2][73782 WaitBeginN 2][74939 WaitBeginN 3][145480 PurgedN-0 3][236823 AddedP 0][237554 PWonRaceOverP 0 1][239685 WaitEndN 3][245155 WaitEndN 2][290995 PurgedP-0 0]
 1: 2189112322 [74697 CacheMissForN 4][74707 BeginBuildN 4][109441 AddedN 4][180177 PurgedN-0 4][236860 CacheMissForP 9][236864 BeginBuildP 9][237645 CacheMissForP 10][237654 BeginBuildP 10][240799 WaitBeginN 4][245652 CacheMissForN 11][245657 BeginBuildN 11][275621 WaitEndN 4][291015 AddedN 11][329024 PurgedN-0 11][420171 WaitBeginN 11][420200 WaitEndN 11][439936 AddedP 10][444754 PWonRaceOverP 10 9][450005 PurgedP-0 10]
 2: 1147256140 [109801 CacheMissForN 5][109806 BeginBuildN 5][145492 AddedN 5][217534 PurgedN-0 5][275878 WaitBeginN 5][291556 CacheMissForN 13][291567 BeginBuildN 13][311231 WaitEndN 5][329038 AddedN 13][363689 PurgedN-0 13][439969 CacheMissForP 21][439972 BeginBuildP 21][444801 CacheMissForP 22][444804 BeginBuildP 22][475771 AddedP 21][478340 WaitBeginN 13][478383 WaitEndN 13][480403 PWonRaceOverP 21 22][492904 PurgedP-0 21]
 3:  883660955 [145937 CacheMissForN 6][145943 BeginBuildN 6][180188 AddedN 6][236810 PurgedN-0 6][312436 WaitBeginN 6][329523 CacheMissForN 15][329530 BeginBuildN 15][347452 WaitEndN 6][363701 AddedN 15][407383 PurgedN-0 15][475847 CacheMissForP 25][475850 BeginBuildP 25][480476 CacheMissForP 26][480481 BeginBuildP 26][510208 AddedP 25][514604 PWonRaceOverP 25 26][520937 WaitBeginN 15][520968 WaitEndN 15][546393 PurgedP-0 25]
 4:  644542265 [180580 CacheMissForN 7][180583 BeginBuildN 7][217547 AddedN 7][254332 PurgedN-0 7][347709 WaitBeginN 7][364072 CacheMissForN 17][364077 BeginBuildN 17][405803 WaitEndN 7][413700 AddedN 17][449500 PurgedN-0 17][510234 CacheMissForP 28][510237 BeginBuildP 28][514630 CacheMissForP 29][514633 BeginBuildP 29][546405 AddedP 28][550741 PWonRaceOverP 28 29][574391 WaitBeginN 17][574417 WaitEndN 17][582270 PurgedP-0 28]
 5:    2491771 [218031 CacheMissForN 8][218038 BeginBuildN 8][254341 AddedN 8][296278 PurgedN-0 8][407414 WaitBeginN 8][414051 CacheMissForN 20][414056 BeginBuildN 20][419953 WaitEndN 8][450021 AddedN 20][487285 PurgedN-0 20][546437 CacheMissForP 31][546441 BeginBuildP 31][550777 CacheMissForP 32][550780 BeginBuildP 32][582286 AddedP 31][585954 PWonRaceOverP 31 32][617681 PurgedP-0 31][628389 WaitBeginN 20][628411 WaitEndN 20]
 6: 3234710487 [254686 CacheMissForN 12][254689 BeginBuildN 12][296286 AddedN 12][333680 PurgedN-0 12][420324 WaitBeginN 12][450409 CacheMissForN 24][450414 BeginBuildN 24][477938 WaitEndN 12][492917 AddedN 24][531590 PurgedN-0 24][582360 CacheMissForP 34][582364 BeginBuildP 34][586033 CacheMissForP 35][586038 BeginBuildP 35][617697 AddedP 34][620913 PWonRaceOverP 34 35][653969 PurgedP-0 34][660447 WaitBeginN 24][660471 WaitEndN 24]
 7: 1997496709 [296610 CacheMissForN 14][296615 BeginBuildN 14][333690 AddedN 14][370777 PurgedN-0 14][478505 WaitBeginN 14][493429 CacheMissForN 27][493441 BeginBuildN 27][520579 WaitEndN 14][531599 AddedN 27][569059 PurgedN-0 27][617725 CacheMissForP 37][617728 BeginBuildP 37][620949 CacheMissForP 38][620951 BeginBuildP 38][653989 AddedP 37][657951 PWonRaceOverP 37 38][660589 WaitBeginN 27][660608 WaitEndN 27][690582 PurgedP-0 37]
 8: 1301841557 [334056 CacheMissForN 16][334061 BeginBuildN 16][370788 AddedN 16][413687 PurgedN-0 16][521089 WaitBeginN 16][532048 CacheMissForN 30][532054 BeginBuildN 30][541736 WaitEndN 16][569070 AddedN 30][605065 PurgedN-0 30][654014 CacheMissForP 40][654018 BeginBuildP 40][657987 CacheMissForP 41][657990 BeginBuildP 41][660756 WaitBeginN 30][660779 WaitEndN 30][690600 AddedP 40][694166 PWonRaceOverP 40 41][763077 PurgedP-0 40]
 9: 2709710687 [371207 CacheMissForN 18][371213 BeginBuildN 18][407394 AddedN 18][439919 PurgedN-0 18][541951 WaitBeginN 18][569528 CacheMissForN 33][569533 BeginBuildN 33][574156 WaitEndN 18][605075 AddedN 33][643293 PurgedN-0 33][660893 WaitBeginN 33][660911 WaitEndN 33][690695 CacheMissForP 42][690698 BeginBuildP 42][694249 CacheMissForP 43][694252 BeginBuildP 43][726094 AddedP 42][729086 PWonRaceOverP 42 43][799675 PurgedP-0 42]
10:  138923328 [407833 CacheMissForN 19][407839 BeginBuildN 19][449507 AddedN 19][475758 PurgedN-0 19][574535 WaitBeginN 19][605571 CacheMissForN 36][605583 BeginBuildN 36][628169 WaitEndN 19][643304 AddedN 36][661058 WaitBeginN 36][661080 WaitEndN 36][681780 PurgedN-0 36][726120 CacheMissForP 44][726122 BeginBuildP 44][729115 CacheMissForP 45][729117 BeginBuildP 45][763092 AddedP 44][765739 PWonRaceOverP 44 45]
11: 2121723309 [449928 CacheMissForN 23][449933 BeginBuildN 23][487293 AddedN 23][510196 PurgedN-0 23][628536 WaitBeginN 23][643697 CacheMissForN 39][643703 BeginBuildN 39][660238 WaitEndN 23][681790 AddedN 39][681994 WaitBeginN 39][682021 WaitEndN 39][726081 PurgedN-0 39][763117 CacheMissForP 46][763119 BeginBuildP 46][765768 CacheMissForP 47][765770 BeginBuildP 47][799689 AddedP 46][802116 PWonRaceOverP 46 47]
#endif

#if 0
Reconstructed
 0:  477900207 [  933 CacheMissForP 0][  949 CacheMissForP 1][  952 BeginBuildP 0][  953 BeginBuildP 1][35823 CacheMissForN 2][35834 CacheMissForN 3][35838 BeginBuildN 2][35839 BeginBuildN 3][73753 AddedN 3][73772 NWonRaceOverN 3 2][73783 WaitBeginN 2][74939 WaitBeginN 3][145480 PurgedN 3][236823 AddedP 0][237554 PWonRaceOverP 0 1][237576 DeletionP 1][239685 WaitEndN 3][245155 WaitEndN 2][245164 DeletionN 2][291007 PurgedP 0][291012 DeletionP 0][810535 DeletionN 3]
 1: 2189112322 [74697 CacheMissForN 4][74707 BeginBuildN 4][109441 AddedN 4][180177 PurgedN 4][236861 CacheMissForP 9][236864 BeginBuildP 9][237645 CacheMissForP 10][237654 BeginBuildP 10][240799 WaitBeginN 4][245652 CacheMissForN 11][245657 BeginBuildN 11][275621 WaitEndN 4][291015 AddedN 11][329024 PurgedN 11][420171 WaitBeginN 11][420201 WaitEndN 11][439936 AddedP 10][444754 PWonRaceOverP 10 9][444772 DeletionP 9][450013 PurgedP 10][450018 DeletionP 10][810393 DeletionN 4][810611 DeletionN 11]
 2: 1147256140 [109801 CacheMissForN 5][109806 BeginBuildN 5][145492 AddedN 5][217535 PurgedN 5][275878 WaitBeginN 5][291556 CacheMissForN 13][291567 BeginBuildN 13][311231 WaitEndN 5][329038 AddedN 13][363689 PurgedN 13][439969 CacheMissForP 21][439972 BeginBuildP 21][444801 CacheMissForP 22][444805 BeginBuildP 22][475771 AddedP 21][478340 WaitBeginN 13][478383 WaitEndN 13][480403 PWonRaceOverP 21 22][480415 DeletionP 22][492911 PurgedP 21][492915 DeletionP 21][810421 DeletionN 5][810666 DeletionN 13]
 3:  883660955 [145937 CacheMissForN 6][145943 BeginBuildN 6][180189 AddedN 6][236810 PurgedN 6][312436 WaitBeginN 6][329523 CacheMissForN 15][329530 BeginBuildN 15][347452 WaitEndN 6][363701 AddedN 15][407383 PurgedN 15][475847 CacheMissForP 25][475850 BeginBuildP 25][480476 CacheMissForP 26][480481 BeginBuildP 26][510208 AddedP 25][514604 PWonRaceOverP 25 26][514612 DeletionP 26][520937 WaitBeginN 15][520968 WaitEndN 15][546399 PurgedP 25][546403 DeletionP 25][810445 DeletionN 6][810715 DeletionN 15]
 4:  644542265 [180580 CacheMissForN 7][180583 BeginBuildN 7][217547 AddedN 7][254332 PurgedN 7][347709 WaitBeginN 7][364072 CacheMissForN 17][364077 BeginBuildN 17][405803 WaitEndN 7][413700 AddedN 17][449500 PurgedN 17][510234 CacheMissForP 28][510237 BeginBuildP 28][514630 CacheMissForP 29][514633 BeginBuildP 29][546405 AddedP 28][550741 PWonRaceOverP 28 29][550751 DeletionP 29][574391 WaitBeginN 17][574417 WaitEndN 17][582280 PurgedP 28][582284 DeletionP 28][810466 DeletionN 7][810791 DeletionN 17]
 5:    2491771 [218031 CacheMissForN 8][218038 BeginBuildN 8][254341 AddedN 8][296278 PurgedN 8][407414 WaitBeginN 8][414051 CacheMissForN 20][414056 BeginBuildN 20][419953 WaitEndN 8][450021 AddedN 20][487285 PurgedN 20][546437 CacheMissForP 31][546441 BeginBuildP 31][550777 CacheMissForP 32][550780 BeginBuildP 32][582286 AddedP 31][585954 PWonRaceOverP 31 32][585963 DeletionP 32][617690 PurgedP 31][617695 DeletionP 31][628389 WaitBeginN 20][628411 WaitEndN 20][810570 DeletionN 8][810849 DeletionN 20]
 6: 3234710487 [254686 CacheMissForN 12][254689 BeginBuildN 12][296286 AddedN 12][333680 PurgedN 12][420324 WaitBeginN 12][450409 CacheMissForN 24][450415 BeginBuildN 24][477939 WaitEndN 12][492917 AddedN 24][531590 PurgedN 24][582361 CacheMissForP 34][582364 BeginBuildP 34][586033 CacheMissForP 35][586038 BeginBuildP 35][617697 AddedP 34][620913 PWonRaceOverP 34 35][620922 DeletionP 35][653983 PurgedP 34][653987 DeletionP 34][660447 WaitBeginN 24][660471 WaitEndN 24][810631 DeletionN 12][810963 DeletionN 24]
 7: 1997496709 [296610 CacheMissForN 14][296615 BeginBuildN 14][333690 AddedN 14][370777 PurgedN 14][478505 WaitBeginN 14][493430 CacheMissForN 27][493441 BeginBuildN 27][520579 WaitEndN 14][531600 AddedN 27][569059 PurgedN 27][617725 CacheMissForP 37][617728 BeginBuildP 37][620949 CacheMissForP 38][620951 BeginBuildP 38][653989 AddedP 37][657951 PWonRaceOverP 37 38][657960 DeletionP 38][660589 WaitBeginN 27][660608 WaitEndN 27][690590 PurgedP 37][690598 DeletionP 37][810686 DeletionN 14][810994 DeletionN 27]
 8: 1301841557 [334056 CacheMissForN 16][334061 BeginBuildN 16][370788 AddedN 16][413687 PurgedN 16][521089 WaitBeginN 16][532048 CacheMissForN 30][532054 BeginBuildN 30][541736 WaitEndN 16][569071 AddedN 30][605065 PurgedN 30][654014 CacheMissForP 40][654018 BeginBuildP 40][657987 CacheMissForP 41][657990 BeginBuildP 41][660756 WaitBeginN 30][660779 WaitEndN 30][690600 AddedP 40][694166 PWonRaceOverP 40 41][694176 DeletionP 41][763086 PurgedP 40][763090 DeletionP 40][810735 DeletionN 16][811024 DeletionN 30]
 9: 2709710687 [371207 CacheMissForN 18][371213 BeginBuildN 18][407395 AddedN 18][439919 PurgedN 18][541951 WaitBeginN 18][569528 CacheMissForN 33][569533 BeginBuildN 33][574156 WaitEndN 18][605075 AddedN 33][643293 PurgedN 33][660893 WaitBeginN 33][660911 WaitEndN 33][690695 CacheMissForP 42][690698 BeginBuildP 42][694249 CacheMissForP 43][694252 BeginBuildP 43][726094 AddedP 42][729086 PWonRaceOverP 42 43][729095 DeletionP 43][799683 PurgedP 42][799687 DeletionP 42][810760 DeletionN 18][811051 DeletionN 33]
10:  138923328 [407833 CacheMissForN 19][407839 BeginBuildN 19][449507 AddedN 19][475758 PurgedN 19][574535 WaitBeginN 19][605571 CacheMissForN 36][605583 BeginBuildN 36][628169 WaitEndN 19][643304 AddedN 36][661058 WaitBeginN 36][661080 WaitEndN 36][681780 PurgedN 36][726120 CacheMissForP 44][726122 BeginBuildP 44][729115 CacheMissForP 45][729117 BeginBuildP 45][763092 AddedP 44][765739 PWonRaceOverP 44 45][765748 DeletionP 45][809892 DeletionP 44][810815 DeletionN 19][811080 DeletionN 36]
11: 2121723309 [449928 CacheMissForN 23][449933 BeginBuildN 23][487293 AddedN 23][510196 PurgedN 23][628536 WaitBeginN 23][643697 CacheMissForN 39][643704 BeginBuildN 39][660238 WaitEndN 23][681790 AddedN 39][681994 WaitBeginN 39][682021 WaitEndN 39][726081 PurgedN 39][763117 CacheMissForP 46][763119 BeginBuildP 46][765768 CacheMissForP 47][765770 BeginBuildP 47][799689 AddedP 46][802116 PWonRaceOverP 46 47][802124 DeletionP 47][809889 DeletionP 46][810914 DeletionN 23][811141 DeletionN 39]
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
                } else if (!strcmp(l.fTag, "PurgedP") ||
                           !strcmp(l.fTag, "PurgedN")) {
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
