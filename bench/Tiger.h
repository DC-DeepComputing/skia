/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Tiger_DEFINED
#define Tiger_DEFINED

#include <cstdint>
#include <vector>

struct SkPoint;
class SkPath;

class Tiger {
public:
    static constexpr uint16_t kImageWidth = 500;
    static constexpr uint16_t kImageHeight = 600;

    static std::vector<SkPath> GetTigerPaths();
};

#endif  // Tiger_DEFINED
