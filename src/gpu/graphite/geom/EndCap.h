/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_EndCap_DEFINED
#define skgpu_graphite_geom_EndCap_DEFINED

#include "src/gpu/graphite/geom/Rect.h"
#include <cstdint>

namespace skgpu::graphite {

template <uint16_t TILE_WIDTH, uint16_t TILE_HEIGHT>
class EndCap {
public:
    EndCap(uint16_t x, uint16_t y, uint16_t width, uint32_t alphaIndex, Recorder* recorder)
            : fX(x), fY(y), fWidth(width), fAlphaIndex(alphaIndex), fRecorder(recorder) {}

    Rect bounds() const {
        return Rect::XYWH(fX, fY, fWidth, TILE_HEIGHT);
    }

    uint16_t x()          const { return fX;          }
    uint16_t y()          const { return fY;          }
    uint16_t width()      const { return fWidth;      }
    uint32_t alphaIndex() const { return fAlphaIndex; }
    Recorder* recorder()  const { return fRecorder;   }

private:
    uint16_t fX;
    uint16_t fY;
    uint16_t fWidth;
    uint32_t fAlphaIndex;
    Recorder* fRecorder;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_EndCap_DEFINED
