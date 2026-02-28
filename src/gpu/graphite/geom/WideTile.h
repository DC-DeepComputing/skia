/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_WideTile_DEFINED
#define skgpu_graphite_geom_WideTile_DEFINED

#include "src/gpu/graphite/geom/Rect.h"
#include <cstdint>

namespace skgpu::graphite {

template <uint16_t HEIGHT>
class WideTile {
public:
    WideTile(uint16_t x, uint16_t y, uint16_t width)
            : fX(x), fY(y), fWidth(width) {
        SkASSERT(fWidth <= kMaxWidth);
    }

    Rect bounds() const {
        return Rect::XYWH(fX, fY, fWidth, HEIGHT);
    }

private:
    static constexpr uint32_t kMaxWidth = 256;

    uint16_t fX;
    uint16_t fY;
    uint16_t fWidth;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_WideTile_DEFINED
