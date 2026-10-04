/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/DownsampleStep.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkMath.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/BufferManager.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ComputeScheduler.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/compute/SharedBlurSksl.h"
#include "src/sksl/SkSLString.h"

#include <algorithm>
#include <cstdint>
#include <utility>

namespace skgpu::graphite {

namespace {

// The workgroup is 1D. Its clusters cover a tile of output texels that's as square as possible.
constexpr int kWorkgroupSizeLog = 8;
constexpr int kWorkgroupSize = 1 << kWorkgroupSizeLog;
static_assert(static_cast<uint32_t>(kWorkgroupSize) <= Caps::kMinComputeInvocationsPerWorkgroup);
static_assert(SkIsPow2(DownsampleStep::kMaxClusterSize) &&
              DownsampleStep::kMaxClusterSize <= kWorkgroupSize);
static_assert(SkIsPow2(DownsampleStep::kMaxFactor));

// Each lane of a cluster takes at least 1 << kMinSamplesPerLaneLog samples, since the shuffles cost
// more than they save with fewer.
constexpr int kMinSamplesPerLaneLog = 2;

// The width and height of a workgroup's tile of output texels, whose clusters have 1 << clusterLog
// lanes. Must match main() below.
SkISize tile_size(int clusterLog) {
    const int tileLog = kWorkgroupSizeLog - clusterLog;
    return {1 << ((tileLog + 1) >> 1), 1 << (tileLog >> 1)};
}

// The uniforms of one downsample. Every member is 16-byte aligned, so this has the same layout as
// the `DownsampleParams` block in the SkSL under std140, std430, Metal and WGSL rules.
struct DownsampleParams {
    // The LTRB of the rect that the tile mode applies to, in source texels.
    int32_t fSrcRect[4];
    // The source texel at the top-left corner of the block that output texel (0, 0) averages (xy),
    // and the downscale factors (zw).
    int32_t fBlockOriginAndFactors[4];
    // The size of the output (xy), and the number of texels that a sample averages along x and y,
    // which is 2 along an axis that's downscaled and 1 along one that isn't (zw).
    int32_t fDstSizeAndPairSize[4];
    // The texels between consecutive samples of a lane along x and y (xy), and the number of
    // samples of a lane along x and y (zw), when the cluster fits in the subgroup.
    int32_t fStepAndCount[4];
    // The log2 of the cluster size (x), the log2 of the number of columns of samples that the
    // cluster's lanes start at (y), and the SkTileMode (z).
    int32_t fClusterAndTileMode[4];
    // The reciprocals of the source texture's dimensions (xy), and of the number of samples that
    // an output texel averages (z).
    float fInvSourceSize[4];
};
static_assert(sizeof(DownsampleParams) == 6 * 16);

// Must match the declaration of DownsampleParams above.
constexpr char kDownsampleParamsSkSL[] =
        "DownsampleParams {"
            "int4 srcRect;"
            "int4 blockOriginAndFactors;"
            "int4 dstSizeAndPairSize;"
            "int4 stepAndCount;"
            "int4 clusterAndTileMode;"
            "float4 invSourceSize;"
        "}";

constexpr SkColorType kColorType = kRGBA_8888_SkColorType;

}  // namespace

bool DownsampleStep::IsSupported(const Caps* caps) {
    // Graphite only enables compute on devices that support subgroups and meet Caps' minimum
    // compute limits, which the step stays within (see above).
    return caps->computeSupport() && caps->isStorage(caps->getDefaultStorageTextureInfo(kColorType));
}

DownsampleStep::DownsampleStep()
        : ComputeStep(
                  /*name=*/"Downsample",
                  /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
                  /*resources=*/{{
                          {
                                  /*type=*/ResourceType::kSampledTexture,
                                  /*flow=*/DataFlow::kShared,
                                  /*policy=*/ResourcePolicy::kNone,
                                  /*slot=*/ComputeScheduler::kSourceSlot,
                                  /*sksl=*/"srcTexture",
                          },
                          {
                                  /*type=*/ResourceType::kWriteOnlyStorageTexture,
                                  /*flow=*/DataFlow::kShared,
                                  /*policy=*/ResourcePolicy::kNone,
                                  /*slot=*/ComputeScheduler::kOutputSlot,
                                  /*sksl=*/"dstTexture",
                          },
                          {
                                  /*type=*/ResourceType::kUniformBuffer,
                                  /*flow=*/DataFlow::kShared,
                                  /*policy=*/ResourcePolicy::kMapped,
                                  /*slot=*/ComputeScheduler::kParamsSlot,
                                  /*sksl=*/kDownsampleParamsSkSL,
                          },
                  }}) {}

DownsampleStep::~DownsampleStep() = default;

sk_sp<Image> DownsampleStep::downsample(Recorder* recorder,
                                        SkISize factors,
                                        const Image& src,
                                        const SkIRect& srcRect,
                                        SkTileMode tileMode,
                                        SkIPoint gridOrigin,
                                        const SkIRect& dstRect) const {
    SkASSERT(recorder);

    const int fx = factors.width();
    const int fy = factors.height();
    if (fx < 1 || fy < 1 || fx > kMaxFactor || fy > kMaxFactor || !SkIsPow2(fx) ||
        !SkIsPow2(fy)) {
        return nullptr;
    }
    // Under repeat tiling, a pair of texels that straddles an edge of the source rect maps to its
    // opposite ends, which one sample can't average. The pairs start at even offsets from the grid
    // origin, so none straddle if the edges are at even offsets too.
    auto pairsAlign = [](int factor, int origin, int lo, int hi) {
        return factor == 1 || (((lo - origin) | (hi - origin)) & 1) == 0;
    };
    if (tileMode == SkTileMode::kRepeat &&
        !(pairsAlign(fx, gridOrigin.fX, srcRect.fLeft, srcRect.fRight) &&
          pairsAlign(fy, gridOrigin.fY, srcRect.fTop, srcRect.fBottom))) {
        return nullptr;
    }

    const SkColorInfo& srcInfo = src.imageInfo().colorInfo();
    const TextureProxy* srcProxy = src.textureProxyView().proxy();
    if (srcInfo.colorType() != kColorType ||
        // The downsample averages the stored values, which must be premultiplied.
        srcInfo.alphaType() == kUnpremul_SkAlphaType ||
        src.textureProxyView().swizzle() != Swizzle::RGBA() ||
        // The sample coordinates are normalized by the texture's dimensions, which a fully lazy
        // proxy doesn't have yet.
        srcProxy->isFullyLazy() ||
        // Storage textures are never protected, so they can't hold protected content.
        recorder->priv().isProtected() == Protected::kYes ||
        srcRect.isEmpty() || dstRect.isEmpty()) {
        return nullptr;
    }
    const SkISize srcDimensions = srcProxy->dimensions();
    SkASSERT(SkIRect::MakeSize(srcDimensions).contains(srcRect));

    const Caps* caps = recorder->priv().caps();
    sk_sp<TextureProxy> output = TextureProxy::Make(caps,
                                                    recorder->priv().resourceProvider(),
                                                    GetApproxSize(dstRect.size()),
                                                    caps->getDefaultStorageTextureInfo(kColorType),
                                                    Budgeted::kYes,
                                                    "DownsampleComputeOutput");
    if (!output) {
        return nullptr;
    }

    // Each sample averages a pair of texels along each axis that's downscaled, so an output texel
    // takes (fx / pairX) * (fy / pairY) samples. Its cluster has enough lanes for each to take
    // 1 << kMinSamplesPerLaneLog of them, up to kMaxClusterSize, and they start at consecutive
    // samples along x, then along y.
    const int pairX = std::min(fx, 2);
    const int pairY = std::min(fy, 2);
    const int samplesXLog = SkPrevLog2(fx / pairX);
    const int samplesYLog = SkPrevLog2(fy / pairY);
    const int clusterLog = std::clamp(samplesXLog + samplesYLog - kMinSamplesPerLaneLog,
                                      0,
                                      SkPrevLog2(kMaxClusterSize));
    const int columnsLog = std::min(samplesXLog, clusterLog);
    const int rowsLog = clusterLog - columnsLog;

    DownsampleParams params = {
            {srcRect.fLeft, srcRect.fTop, srcRect.fRight, srcRect.fBottom},
            {gridOrigin.fX + dstRect.fLeft * fx, gridOrigin.fY + dstRect.fTop * fy, fx, fy},
            {dstRect.width(), dstRect.height(), pairX, pairY},
            {pairX << columnsLog,
             pairY << rowsLog,
             1 << (samplesXLog - columnsLog),
             1 << (samplesYLog - rowsLog)},
            {clusterLog, columnsLog, static_cast<int32_t>(tileMode), 0},
            {1.f / srcDimensions.width(),
             1.f / srcDimensions.height(),
             1.f / (1 << (samplesXLog + samplesYLog)),
             0.f},
    };

    auto [writer, paramsBinding, _] =
            recorder->priv().drawBufferManager()->getMappedUniformBuffer(sizeof(DownsampleParams),
                                                                         /*headroom=*/0);
    if (!paramsBinding) {
        return nullptr;
    }
    writer.write(&params, sizeof(params));

    ComputeScheduler::Dispatch dispatch;
    dispatch.fStep = this;
    dispatch.fSource = &src;
    const TextureFormat outputFormat = output->format();
    dispatch.fOutput =
            TextureProxyView(std::move(output), ReadSwizzleForColorType(kColorType, outputFormat));
    dispatch.fOutputColorInfo =
            SkColorInfo(kColorType, kPremul_SkAlphaType, srcInfo.refColorSpace());
    dispatch.fParams = paramsBinding;
    const SkISize tile = tile_size(clusterLog);
    dispatch.fGlobalSize = WorkgroupSize((dstRect.width() + tile.width() - 1) / tile.width(),
                                         (dstRect.height() + tile.height() - 1) / tile.height(),
                                         1);
    return recorder->priv().computeScheduler()->enqueue(dispatch);
}

std::string DownsampleStep::computeSkSL() const {
    // The tile mode is applied with integer math against srcRect, and every sample lands on or
    // between texels inside srcRect, so the sampler's address mode is never used.
    //
    // The samples of an output texel are split across the lanes of its cluster, which each sum
    // theirs in registers. A butterfly of subgroup shuffles then adds up the cluster's sums, which
    // stays within the cluster since its lanes are aligned to its size within the subgroup.
    std::string sksl = kSharedBlurSkSL;
    SkSL::String::appendf(&sksl, R"(
        const uint WORKGROUP_SIZE_LOG = %d;

        //Returns the coordinate (x) and weight (y) of the sample that averages the source texels p
        //and p + size - 1 along an axis of [lo, hi), where size is 1 or 2. The tile mode maps them
        //to the same texel or to adjacent ones, so the sample lands on the center of that texel or
        //between the two, where bilinear filtering averages them. Under decal, the weight is the
        //fraction of the texels that are inside [lo, hi)
        float2 PairSample(int p, int size, int lo, int hi)
        {
            int q = p + size - 1;
            if (p >= lo && q < hi)
                return float2(float(p) + 0.5 * float(size), 1);
            int mode = clusterAndTileMode.z;
            if (mode == DECAL)
            {
                if (q < lo || p >= hi)
                    return float2(0);
                return float2(float(p < lo ? lo : hi - 1) + 0.5, 0.5);
            }
            return float2(0.5 * float(TileCoord(p, lo, hi, mode) + TileCoord(q, lo, hi, mode)) + 0.5, 1);
        }

        void main()
        {
            //A cluster's lanes must share a subgroup, but the layout of subgroups within the
            //workgroup is implementation-defined, so the lanes are indexed through their subgroup
            uint clusterLog = uint(clusterAndTileMode.x);
            uint clusterSize = 1u << clusterLog;
            uint index = sk_SubgroupID * sk_SubgroupSize + sk_SubgroupInvocationID;
            uint lane = index & (clusterSize - 1);
            uint cluster = index >> clusterLog;

            //The workgroup's clusters cover its tile of output texels in rows
            uint tileLog = WORKGROUP_SIZE_LOG - clusterLog;
            uint tileWidthLog = (tileLog + 1) >> 1;
            int2 coord = int2(int((sk_WorkgroupID.x << tileWidthLog) +
                                  (cluster & ((1u << tileWidthLog) - 1))),
                              int((sk_WorkgroupID.y << (tileLog >> 1)) + (cluster >> tileWidthLog)));
            int2 dstSize = dstSizeAndPairSize.xy;
            bool inside = coord.x < dstSize.x && coord.y < dstSize.y;

            //The lanes of a cluster start at consecutive samples of the texel's block, and each
            //steps over the cluster's span of samples to its next one. A subgroup that's smaller
            //than a cluster can't shuffle within it, so the cluster's first lane takes every sample
            int2 pairSize = dstSizeAndPairSize.zw;
            int2 factors = blockOriginAndFactors.zw;
            bool shuffle = sk_SubgroupSize >= clusterSize;
            int2 first = int2(0);
            int2 step = pairSize;
            int2 count = lane == 0 ? factors / pairSize : int2(0);
            if (shuffle)
            {
                uint columnsLog = uint(clusterAndTileMode.y);
                first = int2(int(lane & ((1u << columnsLog) - 1)), int(lane >> columnsLog)) * pairSize;
                step = stepAndCount.xy;
                count = stepAndCount.zw;
            }
            float4 sum = float4(0);
            if (inside)
            {
                int2 block = blockOriginAndFactors.xy + coord * factors;
                int2 start = block + first;
                if (block.x >= srcRect.x && block.x + factors.x <= srcRect.z &&
                    block.y >= srcRect.y && block.y + factors.y <= srcRect.w)
                {
                    //Every sample is between texels inside srcRect, so no tiling applies
                    float2 base = (float2(start) + 0.5 * float2(pairSize)) * invSourceSize.xy;
                    float2 delta = float2(step) * invSourceSize.xy;
                    for (int y = 0; y < count.y; ++y)
                    {
                        for (int x = 0; x < count.x; ++x)
                            sum += float4(sampleLod(srcTexture, base + float2(x, y) * delta, 0));
                    }
                }
                else
                {
                    for (int x = 0; x < count.x; ++x)
                    {
                        float2 sx = PairSample(start.x + x * step.x, pairSize.x, srcRect.x, srcRect.z);
                        if (sx.y > 0)
                        {
                            for (int y = 0; y < count.y; ++y)
                            {
                                float2 sy = PairSample(start.y + y * step.y, pairSize.y, srcRect.y, srcRect.w);
                                if (sy.y > 0)
                                {
                                    float2 uv = float2(sx.x, sy.x) * invSourceSize.xy;
                                    sum += (sx.y * sy.y) * float4(sampleLod(srcTexture, uv, 0));
                                }
                            }
                        }
                    }
                }
            }

            //The butterfly leaves the cluster's total in each of its lanes
            if (shuffle)
            {
                for (uint m = 1; m < clusterSize; m <<= 1)
                    sum += subgroupShuffle(sum, sk_SubgroupInvocationID ^ m);
            }
            if (lane == 0 && inside)
                textureWrite(dstTexture, uint2(coord), half4(sum * invSourceSize.z));
        }
    )", kWorkgroupSizeLog);
    return sksl;
}

std::tuple<SkISize, SkColorType> DownsampleStep::calculateTextureParameters(
        int resourceIndex, const ResourceDesc&) const {
    SkASSERT(resourceIndex == 1);
    // The size is unused since the output is assigned rather than allocated by the DispatchGroup.
    return {{1, 1}, kColorType};
}

SamplerDesc DownsampleStep::calculateSamplerParameters(int resourceIndex,
                                                      const ResourceDesc&) const {
    SkASSERT(resourceIndex == 0);
    return {SkFilterMode::kLinear, SkTileMode::kClamp};
}

}  // namespace skgpu::graphite
