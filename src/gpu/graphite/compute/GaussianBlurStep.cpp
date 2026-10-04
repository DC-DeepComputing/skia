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
#include "src/sksl/SkSLString.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

namespace skgpu::graphite {

namespace {

// The weights of the widest 1D kernel, padded to a multiple of 4 so they can be packed in float4s.
constexpr int kMaxKernelWidth = SkShaderBlurAlgorithm::KernelWidth(GaussianBlurStep::kMaxRadius);
constexpr int kWeightCount = (kMaxKernelWidth + 3) / 4 * 4;

// The kernel has code for each of these radii, unrolled so that its sums stay in registers and its
// weights are indexed by constants. Each pass runs the code of the smallest radius that covers the
// blur's radius along its axis, with its weights centered on that radius and zero padded, so an
// asymmetric blur only pays for the taps of each axis. Radius 0 copies, for 1D blurs. The radii are
// uniform, so every invocation takes the same branches.
constexpr std::array<int, 7> kBucketRadii = {0, 2, 4, 6, 8, 10, 12};
constexpr int kBucketCount = kBucketRadii.size();
static_assert(kBucketRadii.front() == 0);
static_assert(kBucketRadii.back() == GaussianBlurStep::kMaxRadius);

// Before the barrier, the vertical pass blurs the tile's columns, plus the columns of the
// horizontal halo, from the source. Each invocation streams kVerticalSumsPerThread + 2 * (vertical
// radius) texels down one column into kVerticalSumsPerThread sums held in registers. The sums of
// each tile row are stored to workgroup memory, which the horizontal pass reads after the barrier.
// Each of its invocations blurs kHorizontalSumsPerThread + 2 * (horizontal radius) consecutive
// sums of one row into kHorizontalSumsPerThread outputs.
constexpr int kVerticalSumsPerThread = 8;
constexpr int kVerticalBlocks = GaussianBlurStep::kTileHeight / kVerticalSumsPerThread;
constexpr int kHorizontalSumsPerThread = 4;
constexpr int kHorizontalStrips = GaussianBlurStep::kTileWidth / kHorizontalSumsPerThread;
constexpr int kWorkgroupSize = kHorizontalStrips * GaussianBlurStep::kTileHeight;
static_assert(GaussianBlurStep::kTileHeight % kVerticalSumsPerThread == 0);
static_assert(GaussianBlurStep::kTileWidth % kHorizontalSumsPerThread == 0);
// Every invocation blurs at most one column block in the vertical pass.
static_assert((GaussianBlurStep::kTileWidth + 2 * GaussianBlurStep::kMaxRadius) * kVerticalBlocks <=
              kWorkgroupSize);
// The vertical pass splits its invocations into column blocks with a single compare.
static_assert(kVerticalBlocks == 2);

// The sums are stored with a pad entry after every 4 entries. The horizontal pass's invocations
// read entries kHorizontalSumsPerThread = 4 apart, which would otherwise conflict. The rows are
// packed at the stride of the widest halo, so that the vertical pass, which doesn't know the
// horizontal radius at compile time, stores its sums at constant offsets. The horizontal pass's
// reads don't conflict at any stride: the 16 strips of a row are 5 entries apart, so they cover
// all 32 4-byte banks.
constexpr int PaddedIndex(int i) { return i + (i >> 2); }
constexpr int kRowStride =
        PaddedIndex(GaussianBlurStep::kTileWidth + 2 * GaussianBlurStep::kMaxRadius - 1) + 1;
// The SkSL relies on PaddedIndex(4 * strip + i) == 5 * strip + PaddedIndex(i).
static_assert(kHorizontalSumsPerThread == 4);

// The SkSL below hard-codes these.
static_assert(GaussianBlurStep::kTileWidth == 1 << 6);
static_assert(GaussianBlurStep::kTileHeight == 1 << 4);
static_assert(kHorizontalStrips == 1 << 4);
static_assert(static_cast<int>(SkTileMode::kClamp) == 0);
static_assert(static_cast<int>(SkTileMode::kRepeat) == 1);
static_assert(static_cast<int>(SkTileMode::kMirror) == 2);
static_assert(static_cast<int>(SkTileMode::kDecal) == 3);

// DispatchGroup::Builder checks the step's workgroup size against the device, but not its static
// workgroup arrays. Both must stay within Caps' minimums, which Graphite requires to enable compute.
constexpr int kVerticalSumsSize = kRowStride * GaussianBlurStep::kTileHeight;
static_assert(kVerticalSumsSize * 2 * sizeof(uint32_t) <= Caps::kMinComputeWorkgroupStorageSize);
static_assert(static_cast<uint32_t>(kWorkgroupSize) <= Caps::kMinComputeInvocationsPerWorkgroup);

// The uniforms of one blur. Every member is 16-byte aligned and every array has a 16-byte stride,
// so this has the same layout as the `BlurParams` block in the SkSL under std140, std430, Metal and
// WGSL rules.
struct BlurParams {
    // The LTRB of the rect that the tile mode applies to, in source texels.
    int32_t fSrcRect[4];
    // The source texel under the first output texel (xy), and the size of the output (zw).
    int32_t fDstOriginAndSize[4];
    // The SkTileMode (x), and the bucket radii of the horizontal (y) and vertical (z) passes.
    int32_t fTileModeAndRadii[4];
    // The normalized 1D kernels, centered on their pass's bucket radius and zero padded. Tap k is
    // at offset k - (bucket radius).
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
            "int4 tileModeAndRadii;"
            "float4 weightsX[7];"
            "float4 weightsY[7];"
        "}";

constexpr SkColorType kColorType = kRGBA_8888_SkColorType;

// Returns the radius of the smallest bucket that covers `radius`.
int bucket_radius(int radius) {
    SkASSERT(radius <= GaussianBlurStep::kMaxRadius);
    int bucket = 0;
    while (kBucketRadii[bucket] < radius) {
        ++bucket;
    }
    return kBucketRadii[bucket];
}

// Fills `weights` with the shader blur's 1D kernel for `sigma`, centered on `bucketRadius`.
void compute_weights(float sigma, int radius, int bucketRadius, SkSpan<float> weights) {
    SkASSERT(radius <= bucketRadius);
    std::array<float, kWeightCount> kernel;
    SkShaderBlurAlgorithm::Compute1DBlurKernel(sigma, radius, SkSpan<float>(kernel));
    std::fill(weights.begin(), weights.end(), 0.f);
    std::copy_n(kernel.begin(),
                SkShaderBlurAlgorithm::KernelWidth(radius),
                weights.subspan(bucketRadius - radius).begin());
}

// Appends the vertical pass of the bucket with `radius`. Texel i of a column is tap i - j of the
// column's sum j. The numbers of columns and blocks of rows depend on the horizontal radius and on
// how much of the tile is inside the output, so they're parameters.
void append_blur_vertical(std::string* sksl, int radius) {
    SkSL::String::appendf(sksl,
        "void BlurVertical%d(uint threadIdx, int2 inputOrigin, uint columns, uint blocks)\n"
        "{\n"
        "    if (threadIdx < columns * blocks)\n"
        "    {\n"
        "        uint block = 0;\n"
        "        uint col = threadIdx;\n"
        "        if (col >= columns)\n"
        "        {\n"
        "            block = 1;\n"
        "            col -= columns;\n"
        "        }\n"
        "        int x = TileCoord(inputOrigin.x + int(col), srcRect.x, srcRect.z);\n"
        "        int y = inputOrigin.y + int(block * V_PER_THREAD);\n",
        radius);
    for (int j = 0; j < kVerticalSumsPerThread; ++j) {
        SkSL::String::appendf(sksl, "        float4 v%d = float4(0);\n", j);
    }
    sksl->append("        float4 t;\n");
    for (int i = 0; i < kVerticalSumsPerThread + 2 * radius; ++i) {
        SkSL::String::appendf(sksl, "        t = LoadTexel(x, y + %d);\n", i);
        for (int j = std::max(0, i - 2 * radius); j <= std::min(kVerticalSumsPerThread - 1, i);
             ++j) {
            SkSL::String::appendf(
                    sksl, "        v%d += weightsY[%d][%d] * t;\n", j, (i - j) >> 2, (i - j) & 3);
        }
    }
    SkSL::String::appendf(sksl,
        "        uint s = block * %d + col + (col >> 2);\n",
        kVerticalSumsPerThread * kRowStride);
    for (int j = 0; j < kVerticalSumsPerThread; ++j) {
        SkSL::String::appendf(
                sksl, "        s_vert[s + %d] = PackHalf4(v%d);\n", j * kRowStride, j);
    }
    sksl->append(
        "    }\n"
        "}\n"
        "\n");
}

// Appends the horizontal pass of the bucket with `radius`. Entry i of a strip is tap i - j of the
// strip's output j.
void append_blur_horizontal(std::string* sksl, int radius) {
    SkSL::String::appendf(sksl,
        "void BlurHorizontal%d(uint threadIdx, int2 tileOrigin)\n"
        "{\n"
        "    uint row = threadIdx >> H_STRIPS_LOG;\n"
        "    uint strip = threadIdx & H_STRIPS_MASK;\n"
        "    uint s = row * %d + strip * 5;\n",
        radius, kRowStride);
    for (int j = 0; j < kHorizontalSumsPerThread; ++j) {
        SkSL::String::appendf(sksl, "    float4 h%d = float4(0);\n", j);
    }
    sksl->append("    float4 t;\n");
    for (int i = 0; i < kHorizontalSumsPerThread + 2 * radius; ++i) {
        SkSL::String::appendf(sksl, "    t = UnpackHalf4(s_vert[s + %d]);\n", PaddedIndex(i));
        for (int j = std::max(0, i - 2 * radius); j <= std::min(kHorizontalSumsPerThread - 1, i);
             ++j) {
            SkSL::String::appendf(
                    sksl, "    h%d += weightsX[%d][%d] * t;\n", j, (i - j) >> 2, (i - j) & 3);
        }
    }
    sksl->append(
        "    int2 coord = tileOrigin + int2(int(strip << H_PER_THREAD_LOG), int(row));\n"
        "    if (coord.y < dstOriginAndSize.w)\n"
        "    {\n");
    for (int j = 0; j < kHorizontalSumsPerThread; ++j) {
        SkSL::String::appendf(sksl,
            "        if (coord.x + %d < dstOriginAndSize.z)\n"
            "            textureWrite(dstTexture, uint2(coord + int2(%d, 0)), half4(h%d));\n",
            j, j, j);
    }
    sksl->append(
        "    }\n"
        "}\n"
        "\n");
}

// Appends a call with `args` to the `function` of the bucket whose radius is the uniform SkSL
// expression `radius`, which must be one of kBucketRadii.
void append_bucket_calls(std::string* sksl,
                         const char* radius,
                         const char* function,
                         const char* args) {
    for (int i = 0; i < kBucketCount; ++i) {
        if (i == 0) {
            SkSL::String::appendf(sksl, "    if (%s == %d)\n", radius, kBucketRadii[i]);
        } else if (i < kBucketCount - 1) {
            SkSL::String::appendf(sksl, "    else if (%s == %d)\n", radius, kBucketRadii[i]);
        } else {
            sksl->append("    else\n");
        }
        SkSL::String::appendf(sksl, "        %s%d(%s);\n", function, kBucketRadii[i], args);
    }
}

}  // namespace

bool GaussianBlurStep::IsSupported(const Caps* caps) {
    // Graphite only enables compute on devices that meet Caps' minimum compute limits, which the
    // step stays within (see above).
    return caps->computeSupport() && caps->isStorage(caps->getDefaultStorageTextureInfo(kColorType));
}

GaussianBlurStep::GaussianBlurStep()
        : ComputeStep(
                  /*name=*/"GaussianBlur",
                  /*localDispatchSize=*/{kWorkgroupSize, 1, 1},
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

    // Each pass runs the bucket of its own axis' radius, so a 1D or asymmetric blur only pays for
    // the taps that each axis needs.
    const int bucketRadiusX = bucket_radius(radiusX);
    const int bucketRadiusY = bucket_radius(radiusY);
    BlurParams params = {
            {srcRect.fLeft, srcRect.fTop, srcRect.fRight, srcRect.fBottom},
            {dstRect.fLeft, dstRect.fTop, dstRect.width(), dstRect.height()},
            {static_cast<int32_t>(tileMode), bucketRadiusX, bucketRadiusY, 0},
            {},
            {},
    };
    // These match the weights of the shader blur. They're accumulated in f32 by the kernel, except
    // that the vertical sums are stored as f16 in workgroup memory.
    compute_weights(sigma.width(), radiusX, bucketRadiusX, SkSpan<float>(params.fWeightsX));
    compute_weights(sigma.height(), radiusY, bucketRadiusY, SkSpan<float>(params.fWeightsY));

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
    dispatch.fGlobalSize = WorkgroupSize((dstRect.width() + kTileWidth - 1) / kTileWidth,
                                         (dstRect.height() + kTileHeight - 1) / kTileHeight,
                                         1);

    //printf("blurring\n");
    return recorder->priv().computeScheduler()->enqueue(dispatch);
}

std::string GaussianBlurStep::computeSkSL() const {
    // The tile mode is applied with integer math against srcRect, so no sampler is needed.
    //
    // The workgroup is 1D: its threads cooperatively blur one TILE_WIDTH x TILE_HEIGHT output tile.
    // The grid of tiles stays 2D, since a 1D grid would pass the 65535 workgroups-per-dimension
    // minimum at large outputs.
    //
    // Each bucket's passes are generated as straight-line code (see append_blur_vertical() and
    // append_blur_horizontal()), so that the sums are named registers rather than arrays, and
    // every weight is read at a constant index.
    std::string sksl = SkSL::String::printf(R"(
        const uint TILE_WIDTH_LOG = 6;
        const uint TILE_HEIGHT_LOG = 4;
        const uint V_PER_THREAD = %d;
        const uint H_PER_THREAD_LOG = 2;
        const uint H_STRIPS_LOG = 4;
        const uint H_STRIPS_MASK = 15;

        const int CLAMP = 0;
        const int REPEAT = 1;
        const int MIRROR = 2;
        const int DECAL = 3;

        //The vertical blur of the tile's rows, at its columns and those of the horizontal halo, as
        //f16 RGBA. The rows are packed at the widest halo's stride, with a pad entry after every 4
        //entries so that the strided reads of the horizontal pass don't conflict
        workgroup uint2 s_vert[%d];

        //Returns a modulo n (which must be positive) in [0, n). The result of %% with negative
        //operands differs between backends, so a is made non-negative first
        int PositiveMod(int a, int n)
        {
            if (a < 0)
                a += n * ((n - 1 - a) / n);
            return a %% n;
        }

        //Returns the source coordinate that p maps to along an axis of [lo, hi) with the tile
        //mode, or -1 if it maps to transparent black
        int TileCoord(int p, int lo, int hi)
        {
            if (p >= lo && p < hi)
                return p;
            int mode = tileModeAndRadii.x;
            if (mode == DECAL)
                return -1;
            if (mode == CLAMP)
                return p < lo ? lo : hi - 1;
            int n = hi - lo;
            if (mode == REPEAT)
                return lo + PositiveMod(p - lo, n);
            int m = PositiveMod(p - lo, 2 * n);
            return lo + (m < n ? m : 2 * n - 1 - m);
        }

        //Returns the source texel in column x, which TileCoord has already mapped, and row p
        float4 LoadTexel(int x, int p)
        {
            int y = TileCoord(p, srcRect.y, srcRect.w);
            float4 color = float4(0);
            if (x >= 0 && y >= 0)
                color = float4(textureRead(srcTexture, uint2(x, y)));
            return color;
        }

        uint2 PackHalf4(float4 v)
        {
            return uint2(packHalf2x16(v.xy), packHalf2x16(v.zw));
        }

        float4 UnpackHalf4(uint2 v)
        {
            return float4(unpackHalf2x16(v.x), unpackHalf2x16(v.y));
        }

    )", kVerticalSumsPerThread, kVerticalSumsSize);

    for (int radius : kBucketRadii) {
        append_blur_vertical(&sksl, radius);
        append_blur_horizontal(&sksl, radius);
    }

    SkSL::String::appendf(&sksl,
        "void main()\n"
        "{\n"
        "    uint threadIdx = sk_LocalInvocationIndex;\n"
        "    int2 tileOrigin = int2(int(sk_WorkgroupID.x << TILE_WIDTH_LOG),\n"
        "                           int(sk_WorkgroupID.y << TILE_HEIGHT_LOG));\n"
        "    int2 radii = tileModeAndRadii.yz;\n"
        "    int2 inputOrigin = dstOriginAndSize.xy + tileOrigin - radii;\n"
        "    //At the right and bottom edges of the output, the vertical pass skips the columns and\n"
        "    //blocks of rows that only the horizontal pass's unwritten outputs read\n"
        "    int2 inside = min(dstOriginAndSize.zw - tileOrigin, int2(%d, %d));\n"
        "    uint columns = uint(inside.x + 2 * radii.x);\n"
        "    uint blocks = inside.y > int(V_PER_THREAD) ? 2u : 1u;\n",
        kTileWidth, kTileHeight);
    append_bucket_calls(
            &sksl, "radii.y", "BlurVertical", "threadIdx, inputOrigin, columns, blocks");
    sksl.append("    workgroupBarrier();\n");
    append_bucket_calls(&sksl, "radii.x", "BlurHorizontal", "threadIdx, tileOrigin");
    sksl.append("}\n");
    return sksl;
}

std::tuple<SkISize, SkColorType> GaussianBlurStep::calculateTextureParameters(
        int resourceIndex, const ResourceDesc&) const {
    SkASSERT(resourceIndex == 1);
    // The size is unused since the output is assigned rather than allocated by the DispatchGroup.
    return {{kTileWidth, kTileHeight}, kColorType};
}

}  // namespace skgpu::graphite
