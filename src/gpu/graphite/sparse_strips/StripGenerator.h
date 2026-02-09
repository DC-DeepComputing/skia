/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_StripGenerator_DEFINED
#define skgpu_graphite_StripGenerator_DEFINED

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>

namespace skgpu::graphite {

class StripGenerator {
public:
    static constexpr uint32_t kMSAADim = 64;
    static constexpr float    kPackingScale = 0.5;

    StripGenerator() {}
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_StripGenerator_DEFINED
