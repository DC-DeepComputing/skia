/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkRandom.h"
#include "tests/Test.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

namespace {

struct Tile {
    uint32_t fX;
    uint32_t fId;
};

struct RLETile {
    uint32_t fX;
    uint32_t fId;
    uint32_t fRunLength;
};

void generate_test_data(SkRandom* rand,
                        int numRuns,
                        std::vector<Tile>* outTiles,
                        std::vector<RLETile>* outRLETiles) {
    uint32_t currentId = 0;

    std::vector<RLETile> tempRLE;
    tempRLE.reserve(numRuns);

    // --- CURRENT CODE: Random X within a window of 16 ---
    for (int i = 0; i < numRuns; ++i) {
        uint32_t runLength = rand->nextRangeU(2, 6);
        uint32_t randomX = rand->nextULessThan(16);

        tempRLE.push_back({randomX, currentId, runLength});
        currentId++;
    }

    // 2. Shuffle the compressed runs so our sorting logic actually has to work
    for (int i = numRuns - 1; i > 0; --i) {
        int j = rand->nextULessThan(i + 1);
        std::swap(tempRLE[i], tempRLE[j]);
    }

    *outRLETiles = tempRLE;

    // 3. Expand the *shuffled* runs into the flat tile vector
    for (const auto& rle : *outRLETiles) {
        for (uint32_t j = 0; j < rle.fRunLength; ++j) {
            outTiles->push_back({rle.fX + j, rle.fId});
        }
    }
}

}  // namespace

DEF_TEST(RLETileSequence_TraversalTest, reporter) {
    auto now = std::chrono::high_resolution_clock::now();
    uint32_t seed = static_cast<uint32_t>(now.time_since_epoch().count());
    SkRandom rand(seed);

    std::vector<Tile> tiles;
    std::vector<RLETile> rleTiles;

    constexpr int kNumRuns = 10000;
    generate_test_data(&rand, kNumRuns, &tiles, &rleTiles);

    // -------------------------------------------------------------------------
    // Baseline: Expected Behavior
    // -------------------------------------------------------------------------
    // Sort the flat tiles by X, then by ID to ensure deterministic ordering
    std::sort(tiles.begin(), tiles.end(), [](const Tile& a, const Tile& b) {
        if (a.fX != b.fX) {
            return a.fX < b.fX;
        }
        return a.fId < b.fId;
    });

    // -------------------------------------------------------------------------
    // Novel Technique: Compressed Traversal (with Lookup Optimization)
    // -------------------------------------------------------------------------

    // 1. Sort primarily by fX (ascending), then by fRunLength (descending).
    std::sort(rleTiles.begin(), rleTiles.end(), [](const RLETile& a, const RLETile& b) {
        if (a.fX != b.fX) {
            return a.fX < b.fX;
        }
        return a.fRunLength > b.fRunLength;
    });

    std::vector<Tile> actualTiles;
    actualTiles.reserve(tiles.size());

    if (!rleTiles.empty()) {
        uint32_t minX = rleTiles.front().fX;
        uint32_t maxX = 0;
        for (const auto& rle : rleTiles) {
            maxX = std::max(maxX, rle.fX + rle.fRunLength);
        }

        // 2. Pre-pass: Build the fast-skip lookup table
        // firstIdxForX[x] gives the index of the first RLETile where fX >= x.
        std::vector<size_t> firstIdxForX(maxX + 2, rleTiles.size());

        // Iterate backwards to capture the earliest index for each X
        for (int i = static_cast<int>(rleTiles.size()) - 1; i >= 0; --i) {
            firstIdxForX[rleTiles[i].fX] = i;
        }

        // Backfill gaps (if an X has no runs, it points to the next available X's index)
        for (int x = static_cast<int>(maxX); x >= 0; --x) {
            if (firstIdxForX[x] == rleTiles.size()) {
                firstIdxForX[x] = firstIdxForX[x + 1];
            }
        }

        // 3. Traversal: Sweep through the X coordinates
        size_t activeStartIdx = 0;
        for (uint32_t currentX = minX; currentX < maxX; ++currentX) {

            // OPTIMIZATION 1: Prune fully expired X-blocks from the front
            // If the longest run for an fX is expired, all runs for that fX are expired.
            while (activeStartIdx < rleTiles.size() &&
                   rleTiles[activeStartIdx].fX + rleTiles[activeStartIdx].fRunLength <= currentX) {
                // Jump the starting line entirely past this dead fX
                activeStartIdx = firstIdxForX[rleTiles[activeStartIdx].fX + 1];
            }

            for (size_t i = activeStartIdx; i < rleTiles.size(); ++i) {
                const auto& rle = rleTiles[i];

                // If the run starts after our current X, stop scanning entirely.
                if (rle.fX > currentX) {
                    break;
                }

                // If the run covers currentX, push it.
                if (currentX < rle.fX + rle.fRunLength) {
                    actualTiles.push_back({currentX, rle.fId});
                } else {
                    // OPTIMIZATION 2: Subsumed run skipping using our lookup table
                    // We reached a run that is too short. Skip directly to the next fX block.
                    i = firstIdxForX[rle.fX + 1] - 1; // -1 because the loop will ++i
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // Verification
    // -------------------------------------------------------------------------

    // Normalize the actualTiles to match the Baseline's strict sorting requirements
    // (In production, the renderer likely doesn't need this, but the unit test does).
    std::sort(actualTiles.begin(), actualTiles.end(), [](const Tile& a, const Tile& b) {
        if (a.fX != b.fX) {
            return a.fX < b.fX;
        }
        return a.fId < b.fId;
    });

    if (tiles.size() != actualTiles.size()) {
        ERRORF(reporter,
               "CRITICAL: Count mismatch! Baseline has %zu tiles, Compressed Traversal produced %zu",
               tiles.size(), actualTiles.size());
        return;
    }

    int errorCount = 0;
    constexpr int kMaxErrorsToReport = 5;

    for (size_t i = 0; i < tiles.size(); ++i) {
        if (tiles[i].fX != actualTiles[i].fX || tiles[i].fId != actualTiles[i].fId) {
            if (errorCount < kMaxErrorsToReport) {
                ERRORF(reporter, "Mismatch at index %zu: baseline(%u, %u) vs rle(%u, %u)",
                       i, tiles[i].fX, tiles[i].fId, actualTiles[i].fX, actualTiles[i].fId);
            }
            errorCount++;
        }
    }

    if (errorCount > 0) {
        ERRORF(reporter, "[RLE Traversal] Failed with %d total mismatches.", errorCount);
    } else {
        INFOF(reporter, "[RLE Traversal] Success! Compressed sequence matches baseline (%zu tiles).",
              tiles.size());
    }
}

}  // namespace skgpu::graphite
