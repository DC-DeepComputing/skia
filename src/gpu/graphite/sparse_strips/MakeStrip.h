/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_MakeStrip_DEFINED
#define skgpu_graphite_MakeStrip_DEFINED

#include "include/core/SkRect.h"
#include "include/private/base/SkTDArray.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/MakeStrip_impl.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripTypes.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConstants.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <functional>
#include <type_traits>

namespace skgpu::graphite {

#if defined(GPU_TEST_UTILS)
template <typename MSAAType> using MsaaExactMaskObserver = std::function<void(MSAAType exactMask)>;
#endif

class MakeStrip {
public:
    template <uint16_t TileWidth, uint16_t TileHeight, typename MSAAType>
    static void MakeStripsMsaa(const Tiles<TileWidth, TileHeight>& tileContainer,
                               SkTDArray<Strip>& stripBuf,
                               SkTDArray<MSAAType>& alphaBuf,
                               SkPathFillType fillType,
                               const Polyline& polyline,
                               const SkTDArray<MSAAType>& maskLut,
                               WideTiles<TileHeight>& wides,
                               bool isCulled,
                               const WindingHistogram& windingHistogram,
                               const SkIRect& clipRect
#if defined(GPU_TEST_UTILS)
                               , MsaaExactMaskObserver<MSAAType> observer = nullptr
#endif
    ) {
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            MakeStripImpl::MakeStripsImpl<TileWidth, TileHeight, kIsWinding, MSAAType>(
                    const_cast<Tiles<TileWidth, TileHeight>&>(tileContainer),  // Cast off const
                    stripBuf,
                    alphaBuf,
                    isInverse,
                    polyline,
                    maskLut,
                    wides,
                    isCulled,
                    windingHistogram,
                    clipRect
#if defined(GPU_TEST_UTILS)
                    , observer
#endif
            );
        });
    }

    template <uint16_t TileWidth, uint16_t TileHeight, typename MSAAType>
    static void MakeStripsMsaaSimd(const Tiles<TileWidth, TileHeight>& tileContainer,
                                   SkTDArray<Strip>& stripBuf,
                                   SkTDArray<MSAAType>& alphaBuf,
                                   SkPathFillType fillType,
                                   const Polyline& polyline,
                                   const SkTDArray<MSAAType>& maskLut,
                                   WideTiles<TileHeight>& wides,
                                   bool isCulled,
                                   const WindingHistogram& windingHistogram,
                                   const SkIRect& clipRect
#if defined(GPU_TEST_UTILS)
                                   , MsaaExactMaskObserver<MSAAType> observer = nullptr
#endif
    ) {
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            MakeStripImpl::MakeStripsSimdImpl<TileWidth, TileHeight, kIsWinding, MSAAType>(
                    const_cast<Tiles<TileWidth, TileHeight>&>(tileContainer),
                    stripBuf,
                    alphaBuf,
                    isInverse,
                    polyline,
                    maskLut,
                    wides,
                    isCulled,
                    windingHistogram,
                    clipRect
#if defined(GPU_TEST_UTILS)
                    , observer
#endif
            );
        });
    }

    template <uint16_t TileWidth, uint16_t TileHeight, uint32_t kWorkGroupSize, typename MSAAType>
    static uint32_t MakeStripsGpu(
            Tiles<TileWidth, TileHeight, GpuTile<TileWidth, TileHeight>>& tileContainer,
            SkTDArray<Strip>& stripBuf,
            SkPathFillType fillType,
            const Polyline& polyline,
            WideTiles<TileHeight>& wides,
            bool isCulled,
            const WindingHistogram& windingHistogram,
            const SkIRect& clipRect,
            int32_t tileOffset,
            int32_t alphaOffset
    ) {
        uint32_t localAlphaIdx = 0;
        Dispatch(fillType, [&](auto isWindingTag, bool isInverse) {
            constexpr bool kIsWinding = decltype(isWindingTag)::value;
            localAlphaIdx = MakeStripImpl::
                    MakeStripsGpuImpl<TileWidth, TileHeight, kWorkGroupSize, kIsWinding, MSAAType>(
                            tileContainer, stripBuf, isInverse, fillType, polyline, wides,
                            isCulled, windingHistogram, clipRect, tileOffset, alphaOffset);
        });
        return localAlphaIdx;
    }

private:
#if defined(GPU_TEST_UTILS)
    template <uint16_t, uint16_t> friend class ClipTestRunner;
    template <uint16_t, uint16_t, typename> friend class CoverageTestRunner;
    template <uint16_t, uint16_t> friend class CoverageBenchSimd;
#endif
    template <typename> friend class StripGenerator;

    template <typename F> static void Dispatch(SkPathFillType fillType, F&& f) {
        switch (fillType) {
            case SkPathFillType::kWinding:
                f(std::bool_constant<true>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseWinding:
                f(std::bool_constant<true>{}, /*isInverse=*/true);
                return;
            case SkPathFillType::kEvenOdd:
                f(std::bool_constant<false>{}, /*isInverse=*/false);
                return;
            case SkPathFillType::kInverseEvenOdd:
                f(std::bool_constant<false>{}, /*isInverse=*/true);
                return;
        }
        SkUNREACHABLE;
    }
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_MakeStrip_DEFINED
