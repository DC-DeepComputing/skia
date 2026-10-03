/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/compute/GaussianBlurStep.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkTo.h"
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

#include <cstdint>
#include <utility>

namespace skgpu::graphite {

namespace {

// The weights of the widest 1D kernel, padded to a multiple of 4 so they can be packed in float4s.
constexpr int kMaxKernelWidth = SkShaderBlurAlgorithm::KernelWidth(GaussianBlurStep::kMaxRadius);
constexpr int kWeightCount = (kMaxKernelWidth + 3) / 4 * 4;

// The uniforms of one blur. Every member is 16-byte aligned and every array has a 16-byte stride,
// so this has the same layout as the `BlurParams` block in the SkSL under std140, std430, Metal and
// WGSL rules.
struct BlurParams {
    // The LTRB of the rect that the tile mode applies to, in source texels.
    int32_t fSrcRect[4];
    // The source texel under the first output texel (xy), and the size of the output (zw).
    int32_t fDstOriginAndSize[4];
    // The blur radius along each axis (xy), and the SkTileMode (z).
    int32_t fRadiiAndTileMode[4];
    // The normalized 1D kernels, zero padded. Tap k is at offset k - radius.
    float fWeightsX[kWeightCount];
    float fWeightsY[kWeightCount];
};
static_assert(sizeof(BlurParams) == 3 * 16 + 2 * kWeightCount * 4);

// Must match the declaration of BlurParams below.
static_assert(kWeightCount == 7 * 4);
constexpr char kBlurParamsSkSL[] =
        "BlurParams {"
            "int4 srcRect;"
            "int4 dstOriginAndSize;"
            "int4 radiiAndTileMode;"
            "float4 weightsX[7];"
            "float4 weightsY[7];"
        "}";

// The SkSL below hard-codes these.
static_assert(GaussianBlurStep::kTileSize == 16);
static_assert(GaussianBlurStep::kMaxRadius == 12);
static_assert(static_cast<int>(SkTileMode::kClamp) == 0);
static_assert(static_cast<int>(SkTileMode::kRepeat) == 1);
static_assert(static_cast<int>(SkTileMode::kMirror) == 2);
static_assert(static_cast<int>(SkTileMode::kDecal) == 3);

// DispatchGroup::Builder checks the step's workgroup size against the device, but not its static
// workgroup arrays. Both must stay within Caps' minimums, which Graphite requires to enable compute.
constexpr int kMaxInputSize = GaussianBlurStep::kTileSize + 2 * GaussianBlurStep::kMaxRadius;
constexpr size_t kWorkgroupStorageBytes =
        kMaxInputSize * kMaxInputSize * sizeof(uint32_t) +                   // inputTexels
        kMaxInputSize * GaussianBlurStep::kTileSize * 2 * sizeof(uint32_t);  // horizontalSums
static_assert(kWorkgroupStorageBytes <= Caps::kMinComputeWorkgroupStorageSize);
static_assert(static_cast<uint32_t>(GaussianBlurStep::kTileSize * GaussianBlurStep::kTileSize) <=
              Caps::kMinComputeInvocationsPerWorkgroup);

constexpr SkColorType kColorType = kRGBA_8888_SkColorType;

}  // namespace

bool GaussianBlurStep::IsSupported(const Caps* caps) {
    // Graphite only enables compute on devices that meet Caps' minimum compute limits, which the
    // step stays within (see above).
    return caps->computeSupport() && caps->isStorage(caps->getDefaultStorageTextureInfo(kColorType));
}

GaussianBlurStep::GaussianBlurStep()
        : ComputeStep(
                  /*name=*/"GaussianBlur",
                  /*localDispatchSize=*/{kTileSize, kTileSize, 1},
                  /*resources=*/{{
                          {
                                  /*type=*/ResourceType::kReadOnlyTexture,
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
                                  /*sksl=*/kBlurParamsSkSL,
                          },
                  }}) {
    SkASSERT(SkBlurEngine::SigmaToRadius(kMaxSigma) == kMaxRadius);
}

GaussianBlurStep::~GaussianBlurStep() = default;

sk_sp<Image> GaussianBlurStep::blur(Recorder* recorder,
                                    SkSize sigma,
                                    const Image& src,
                                    const SkIRect& srcRect,
                                    SkTileMode tileMode,
                                    const SkIRect& dstRect) const {
    SkASSERT(recorder);
    SkASSERT(SkIRect::MakeSize(src.dimensions()).contains(srcRect));

    const int radiusX = SkBlurEngine::SigmaToRadius(sigma.width());
    const int radiusY = SkBlurEngine::SigmaToRadius(sigma.height());
    const SkColorInfo& srcInfo = src.imageInfo().colorInfo();
    if (srcInfo.colorType() != kColorType ||
        // The kernel blurs the stored values, which must be premultiplied.
        srcInfo.alphaType() == kUnpremul_SkAlphaType ||
        src.textureProxyView().swizzle() != Swizzle::RGBA() ||
        // Storage textures are never protected, so they can't hold the blur of protected content.
        recorder->priv().isProtected() == Protected::kYes ||
        // The caller rescales larger sigmas, so this only fails because of rounding.
        radiusX > kMaxRadius || radiusY > kMaxRadius ||
        srcRect.isEmpty() || dstRect.isEmpty()) {
        return nullptr;
    }

    // The output is approx-fit like the shader blur's, so that its texture is more likely to be
    // reused by other blurs once it's returned to the scratch pool.
    const Caps* caps = recorder->priv().caps();
    sk_sp<TextureProxy> output = TextureProxy::Make(caps,
                                                    recorder->priv().resourceProvider(),
                                                    GetApproxSize(dstRect.size()),
                                                    caps->getDefaultStorageTextureInfo(kColorType),
                                                    Budgeted::kYes,
                                                    "GaussianBlurComputeOutput");
    if (!output) {
        return nullptr;
    }

    BlurParams params = {
            {srcRect.fLeft, srcRect.fTop, srcRect.fRight, srcRect.fBottom},
            {dstRect.fLeft, dstRect.fTop, dstRect.width(), dstRect.height()},
            {radiusX, radiusY, static_cast<int32_t>(tileMode), 0},
            {},
            {},
    };
    // These match the weights of the shader blur. They're accumulated in f32 by the kernel, except
    // that the horizontal sums are stored as f16 in workgroup memory.
    SkShaderBlurAlgorithm::Compute1DBlurKernel(
            sigma.width(), radiusX, SkSpan<float>(params.fWeightsX, kWeightCount));
    SkShaderBlurAlgorithm::Compute1DBlurKernel(
            sigma.height(), radiusY, SkSpan<float>(params.fWeightsY, kWeightCount));

    auto [writer, paramsBinding, _] =
            recorder->priv().drawBufferManager()->getMappedUniformBuffer(sizeof(BlurParams),
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
    dispatch.fGlobalSize = WorkgroupSize((dstRect.width() + kTileSize - 1) / kTileSize,
                                         (dstRect.height() + kTileSize - 1) / kTileSize,
                                         1);
    return recorder->priv().computeScheduler()->enqueue(dispatch);
}

std::string GaussianBlurStep::computeSkSL() const {
    // The tile mode is applied with integer math against srcRect, so no sampler is needed.
    return R"(
        const int kTileSize = 16;
        const int kMaxRadius = 12;
        const int kInvocations = kTileSize * kTileSize;
        const int kMaxInputSize = kTileSize + 2 * kMaxRadius;

        const int kClamp = 0;
        const int kRepeat = 1;
        const int kMirror = 2;
        const int kDecal = 3;

        // The source texels of the tile and its halo, as unorm8 RGBA. The rows are tightly packed,
        // so smaller radii use less of the array.
        workgroup uint inputTexels[kMaxInputSize * kMaxInputSize];
        // The horizontal blur of every row of inputTexels at the tile's columns, as f16 RGBA.
        workgroup uint2 horizontalSums[kMaxInputSize * kTileSize];

        // Returns `a` modulo `n` (which must be positive) in [0, n). The result of % with negative
        // operands differs between backends, so `a` is made non-negative first.
        int positive_mod(int a, int n) {
            if (a < 0) {
                a += n * ((n - 1 - a) / n);
            }
            return a % n;
        }

        // Returns the source texel that `p` maps to with the tile mode, or -1s if it maps to
        // transparent black.
        int2 tile_texel(int2 p, int4 rect, int mode) {
            int2 size = rect.zw - rect.xy;
            int2 d = p - rect.xy;
            if (mode == kDecal) {
                if (d.x < 0 || d.y < 0 || d.x >= size.x || d.y >= size.y) {
                    return int2(-1);
                }
            } else if (mode == kClamp) {
                d = clamp(d, int2(0), size - 1);
            } else if (mode == kRepeat) {
                d = int2(positive_mod(d.x, size.x), positive_mod(d.y, size.y));
            } else {
                int2 m = int2(positive_mod(d.x, 2 * size.x), positive_mod(d.y, 2 * size.y));
                d = int2(m.x < size.x ? m.x : 2 * size.x - 1 - m.x,
                         m.y < size.y ? m.y : 2 * size.y - 1 - m.y);
            }
            return rect.xy + d;
        }

        void main() {
            int2 radii = radiiAndTileMode.xy;
            int inputWidth = kTileSize + 2 * radii.x;
            int inputHeight = kTileSize + 2 * radii.y;
            int2 tileOrigin = int2(sk_WorkgroupID.xy) * kTileSize;
            int invocation = int(sk_LocalInvocationIndex);

            // Load the tile and its halo.
            int2 inputOrigin = dstOriginAndSize.xy + tileOrigin - radii;
            for (int i = invocation; i < inputWidth * inputHeight; i += kInvocations) {
                int2 texel = tile_texel(inputOrigin + int2(i % inputWidth, i / inputWidth),
                                        srcRect,
                                        radiiAndTileMode.z);
                half4 color = half4(0);
                if (texel.x >= 0) {
                    color = textureRead(srcTexture, uint2(texel));
                }
                inputTexels[i] = packUnorm4x8(float4(color));
            }
            workgroupBarrier();

            // Blur every row of the input horizontally, at the tile's columns.
            for (int i = invocation; i < inputHeight * kTileSize; i += kInvocations) {
                int firstTap = (i / kTileSize) * inputWidth + i % kTileSize;
                float4 sum = float4(0);
                for (int k = 0; k <= 2 * radii.x; ++k) {
                    sum += weightsX[k / 4][k % 4] * unpackUnorm4x8(inputTexels[firstTap + k]);
                }
                horizontalSums[i] = uint2(packHalf2x16(sum.xy), packHalf2x16(sum.zw));
            }
            workgroupBarrier();

            // Blur the horizontal sums vertically, into the output.
            int2 localID = int2(sk_LocalInvocationID.xy);
            int2 outputCoord = tileOrigin + localID;
            if (outputCoord.x < dstOriginAndSize.z && outputCoord.y < dstOriginAndSize.w) {
                float4 sum = float4(0);
                for (int k = 0; k <= 2 * radii.y; ++k) {
                    uint2 h = horizontalSums[(localID.y + k) * kTileSize + localID.x];
                    sum += weightsY[k / 4][k % 4] *
                           float4(unpackHalf2x16(h.x), unpackHalf2x16(h.y));
                }
                textureWrite(dstTexture, uint2(outputCoord), half4(sum));
            }
        }
    )";
}

std::tuple<SkISize, SkColorType> GaussianBlurStep::calculateTextureParameters(
        int resourceIndex, const ResourceDesc&) const {
    SkASSERT(resourceIndex == 1);
    // The size is unused since the output is assigned rather than allocated by the DispatchGroup.
    return {{kTileSize, kTileSize}, kColorType};
}

}  // namespace skgpu::graphite
