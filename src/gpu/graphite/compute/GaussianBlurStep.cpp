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
#include "include/private/SkMath.h"
#include "include/private/SkTo.h"
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
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace skgpu::graphite {

namespace {

// The weights of the widest 1D kernel, padded to a multiple of 4 so they can be packed in float4s.
constexpr int kMaxKernelWidth = SkShaderBlurAlgorithm::KernelWidth(GaussianBlurStep::kMaxRadius);
constexpr int kWeightCount = (kMaxKernelWidth + 3) / 4 * 4;

// The kernel has code for each bucket radius, which are the multiples of the tuning's bucket step
// up to kMaxRadius. The code is unrolled so that its sums stay in registers and its weights are
// indexed by constants. Each pass runs the code of the smallest radius that covers the blur's
// radius along its axis, with its weights centered on that radius and zero padded, so an
// asymmetric blur only pays for the taps of each axis. Radius 0 copies, for 1D blurs. The radii are
// uniform, so every invocation takes the same branches.
//
// Before the barrier, the vertical pass blurs the tile's columns, plus the columns of the
// horizontal halo, from the source. Each of its items streams V + 2 * (vertical radius) texels down
// one column into V sums held in registers, where V is the tuning's fVerticalSumsPerThread, so the
// tile's rows are split into (tile height / V) blocks. The sums of each tile row are stored to
// workgroup memory, which the horizontal pass reads after the barrier. Each of its invocations
// blurs H + 2 * (horizontal radius) consecutive sums of one row into H outputs, where H is the
// tuning's fHorizontalSumsPerThread, so each row is split into (tile width / H) strips. The
// workgroup has an invocation for each strip of each row. If it has fewer invocations than the
// vertical pass can have items, each invocation loops over the items.

// The output tile of each workgroup is tile_width() x tile_height().
constexpr int tile_width(const GaussianBlurTuning& tuning) { return 1 << tuning.fTileWidthLog; }
constexpr int tile_height(const GaussianBlurTuning& tuning) { return 1 << tuning.fTileHeightLog; }

constexpr int vertical_blocks(const GaussianBlurTuning& tuning) {
    return tile_height(tuning) / tuning.fVerticalSumsPerThread;
}

constexpr int horizontal_strips(const GaussianBlurTuning& tuning) {
    return tile_width(tuning) / tuning.fHorizontalSumsPerThread;
}

constexpr int workgroup_size(const GaussianBlurTuning& tuning) {
    return horizontal_strips(tuning) * tile_height(tuning);
}

// The most columns that the vertical pass blurs, those of the tile and of the widest halo.
constexpr int max_columns(const GaussianBlurTuning& tuning) {
    return tile_width(tuning) + 2 * GaussianBlurStep::kMaxRadius;
}

// If `pad` is set, the sums are stored with a pad entry after every 4 entries. With the default
// tuning, the horizontal pass's invocations read entries 4 apart, which would otherwise conflict.
// The rows are packed at the stride of the widest halo, so that the vertical pass, which doesn't
// know the horizontal radius at compile time, stores its sums at constant offsets. With the default
// tuning, the horizontal pass's reads don't conflict at any stride: the 16 strips of a row are 5
// entries apart, so they cover all 32 4-byte banks.
//
// H is a multiple of 4 when the sums are padded, so the SkSL can rely on
// padded_index(H * strip + i) == strip * padded_index(H) + padded_index(i).
constexpr int padded_index(int i, bool pad) { return pad ? i + (i >> 2) : i; }

constexpr int row_stride(const GaussianBlurTuning& tuning) {
    return padded_index(max_columns(tuning) - 1, tuning.fPadSums) + 1;
}

// The bytes of the step's workgroup arrays, whose entries are f16 RGBA.
constexpr size_t workgroup_storage_size(const GaussianBlurTuning& tuning) {
    return static_cast<size_t>(row_stride(tuning)) * tile_height(tuning) * 2 * sizeof(uint32_t);
}

// DispatchGroup::Builder checks the step's workgroup size against the device, but not its static
// workgroup arrays. IsSupported() checks both, and the default tuning stays within Caps' minimums,
// which Graphite requires to enable compute.
constexpr GaussianBlurTuning kDefaultTuning = {};
static_assert(workgroup_storage_size(kDefaultTuning) <= Caps::kMinComputeWorkgroupStorageSize);
static_assert(static_cast<uint32_t>(workgroup_size(kDefaultTuning)) <=
              Caps::kMinComputeInvocationsPerWorkgroup);
// With the default tuning, every invocation blurs at most one item in the vertical pass, which
// splits them into blocks with a single compare.
static_assert(max_columns(kDefaultTuning) * vertical_blocks(kDefaultTuning) <=
              workgroup_size(kDefaultTuning));
static_assert(vertical_blocks(kDefaultTuning) == 2);

// Returns why `tuning` is invalid, or an empty string if it's valid. The device's compute limits are
// only checked if `caps` isn't null.
std::string validate(const GaussianBlurTuning& tuning, const Caps* caps) {
    // The upper bounds keep the tiles, and the code unrolled for them, within reason.
    if (tuning.fTileWidthLog < 2 || tuning.fTileWidthLog > 10) {
        return SkSL::String::printf("fTileWidthLog %d isn't in [2, 10]", tuning.fTileWidthLog);
    }
    if (tuning.fTileHeightLog < 0 || tuning.fTileHeightLog > 10) {
        return SkSL::String::printf("fTileHeightLog %d isn't in [0, 10]", tuning.fTileHeightLog);
    }
    const int tileWidth = tile_width(tuning);
    const int tileHeight = tile_height(tuning);
    const int v = tuning.fVerticalSumsPerThread;
    if (v < 1 || !SkIsPow2(v) || v > std::min(tileHeight, 64)) {
        return SkSL::String::printf(
                "fVerticalSumsPerThread %d isn't a power of two in [1, min(tile height %d, 64)]",
                v,
                tileHeight);
    }
    const int h = tuning.fHorizontalSumsPerThread;
    const int minH = tuning.fPadSums ? 4 : 1;
    if (h < minH || !SkIsPow2(h) || h > std::min(tileWidth, 64)) {
        std::string reason = SkSL::String::printf(
                "fHorizontalSumsPerThread %d isn't a power of two in [%d, min(tile width %d, 64)]",
                h,
                minH,
                tileWidth);
        if (tuning.fPadSums) {
            reason += " with fPadSums";
        }
        return reason;
    }
    if (tuning.fBucketStep < 1 || GaussianBlurStep::kMaxRadius % tuning.fBucketStep != 0) {
        return SkSL::String::printf("fBucketStep %d doesn't divide %d",
                                    tuning.fBucketStep,
                                    GaussianBlurStep::kMaxRadius);
    }
    if (caps) {
        const uint32_t workgroupSize = SkTo<uint32_t>(workgroup_size(tuning));
        if (workgroupSize > caps->maxComputeInvocationsPerWorkgroup() ||
            workgroupSize > caps->maxComputeWorkgroupSize().fWidth) {
            return SkSL::String::printf(
                    "workgroup size %u exceeds the device's limits of %u invocations and width %u",
                    workgroupSize,
                    caps->maxComputeInvocationsPerWorkgroup(),
                    caps->maxComputeWorkgroupSize().fWidth);
        }
        const size_t storageSize = workgroup_storage_size(tuning);
        if (storageSize > caps->maxComputeWorkgroupStorageSize()) {
            return SkSL::String::printf("workgroup storage of %zu bytes exceeds the device's limit "
                                        "of %u",
                                        storageSize,
                                        caps->maxComputeWorkgroupStorageSize());
        }
    }
    return {};
}

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

}  // namespace

bool GaussianBlurStep::IsSupported(const Caps* caps,
                                   const GaussianBlurTuning& tuning,
                                   std::string* error) {
    // Graphite only enables compute on devices that meet Caps' minimum compute limits, which the
    // default tuning stays within (see above).
    if (!caps->computeSupport() ||
        !caps->isStorage(caps->getDefaultStorageTextureInfo(kColorType))) {
        return false;
    }
    std::string reason = validate(tuning, caps);
    if (!reason.empty()) {
        if (error) {
            *error = std::move(reason);
        }
        return false;
    }
    return true;
}

GaussianBlurStep::GaussianBlurStep(const GaussianBlurTuning& tuning)
        : ComputeStep(
                  /*name=*/"GaussianBlur",
                  /*localDispatchSize=*/{SkTo<uint32_t>(workgroup_size(tuning)), 1, 1},
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
                  }})
        , fTuning(tuning)
        , fTileWidth(tile_width(tuning))
        , fTileHeight(tile_height(tuning))
        , fVerticalBlocks(vertical_blocks(tuning))
        , fHorizontalStrips(horizontal_strips(tuning))
        , fWorkgroupSize(workgroup_size(tuning))
        , fRowStride(row_stride(tuning)) {
    SkASSERT(SkBlurEngine::SigmaToRadius(kMaxSigma) == kMaxRadius);
    SkASSERTF(validate(tuning, /*caps=*/nullptr).empty(),
              "%s",
              validate(tuning, /*caps=*/nullptr).c_str());
    for (int radius = 0; radius <= kMaxRadius; radius += tuning.fBucketStep) {
        fBucketRadii.push_back(radius);
    }
    SkASSERT(fBucketRadii.back() == kMaxRadius);
}

GaussianBlurStep::~GaussianBlurStep() = default;

int GaussianBlurStep::bucketRadius(int radius) const {
    SkASSERT(radius <= kMaxRadius);
    int bucket = 0;
    while (fBucketRadii[bucket] < radius) {
        ++bucket;
    }
    return fBucketRadii[bucket];
}

int GaussianBlurStep::paddedIndex(int i) const { return padded_index(i, fTuning.fPadSums); }

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
    const int bucketRadiusX = this->bucketRadius(radiusX);
    const int bucketRadiusY = this->bucketRadius(radiusY);
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
    dispatch.fGlobalSize = WorkgroupSize((dstRect.width() + fTileWidth - 1) / fTileWidth,
                                         (dstRect.height() + fTileHeight - 1) / fTileHeight,
                                         1);

    //printf("blurring\n");
    return recorder->priv().computeScheduler()->enqueue(dispatch);
}

// Appends the vertical pass of the bucket with `radius`. Texel i of a column is tap i - j of the
// column's sum j. The numbers of columns and blocks of rows depend on the horizontal radius and on
// how much of the tile is inside the output, so they're parameters.
void GaussianBlurStep::appendBlurVertical(std::string* sksl, int radius) const {
    const int v = fTuning.fVerticalSumsPerThread;
    // Each item of the vertical pass is a column of a block of rows. If the workgroup can have
    // fewer invocations than items, each invocation loops over them.
    const bool loop = (fTileWidth + 2 * kMaxRadius) * fVerticalBlocks > fWorkgroupSize;
    const char* item = loop ? "item" : "threadIdx";
    SkSL::String::appendf(sksl,
        "void BlurVertical%d(uint threadIdx, int2 inputOrigin, uint columns, uint blocks)\n"
        "{\n",
        radius);
    if (loop) {
        SkSL::String::appendf(sksl,
            "    for (uint item = threadIdx; item < columns * blocks; item += %d)\n",
            fWorkgroupSize);
    } else {
        sksl->append("    if (threadIdx < columns * blocks)\n");
    }
    sksl->append("    {\n");
    if (fVerticalBlocks <= 2) {
        // Up to 2 blocks are split with a single compare.
        SkSL::String::appendf(sksl,
            "        uint block = 0;\n"
            "        uint col = %s;\n",
            item);
        if (fVerticalBlocks == 2) {
            sksl->append(
                "        if (col >= columns)\n"
                "        {\n"
                "            block = 1;\n"
                "            col -= columns;\n"
                "        }\n");
        }
    } else {
        SkSL::String::appendf(sksl,
            "        uint block = %s / columns;\n"
            "        uint col = %s - block * columns;\n",
            item, item);
    }
    SkSL::String::appendf(sksl,
        "        uint s = block * %d + col%s;\n"
        "        int x = TileCoord(inputOrigin.x + int(col), srcRect.x, srcRect.z, tileModeAndRadii.x);\n"
        "        if (x < 0)\n"
        "        {\n",
        v * fRowStride,
        fTuning.fPadSums ? " + (col >> 2)" : "");
    for (int j = 0; j < v; ++j) {
        SkSL::String::appendf(sksl, "            s_vert[s + %d] = uint2(0);\n", j * fRowStride);
    }
    SkSL::String::appendf(sksl,
        "            %s;\n"
        "        }\n"
        "        int y = inputOrigin.y + int(block * V_PER_THREAD);\n",
        loop ? "continue" : "return");
    for (int j = 0; j < v; ++j) {
        SkSL::String::appendf(sksl, "        half4 v%d = half4(0);\n", j);
    }
    sksl->append("        half4 t;\n");
    for (int i = 0; i < v + 2 * radius; ++i) {
        SkSL::String::appendf(sksl, "        t = LoadTexel(x, y + %d);\n", i);
        for (int j = std::max(0, i - 2 * radius); j <= std::min(v - 1, i); ++j) {
            SkSL::String::appendf(
                    sksl, "        v%d += half(weightsY[%d][%d]) * t;\n", j, (i - j) >> 2, (i - j) & 3);
        }
    }
    for (int j = 0; j < v; ++j) {
        SkSL::String::appendf(
                sksl, "        s_vert[s + %d] = PackHalf4(v%d);\n", j * fRowStride, j);
    }
    sksl->append(
        "    }\n"
        "}\n"
        "\n");
}

// Appends the horizontal pass of the bucket with `radius`. Entry i of a strip is tap i - j of the
// strip's output j.
void GaussianBlurStep::appendBlurHorizontal(std::string* sksl, int radius) const {
    const int h = fTuning.fHorizontalSumsPerThread;
    SkSL::String::appendf(sksl,
        "void BlurHorizontal%d(uint threadIdx, int2 tileOrigin)\n"
        "{\n"
        "    uint row = threadIdx >> H_STRIPS_LOG;\n"
        "    uint strip = threadIdx & H_STRIPS_MASK;\n"
        "    int2 coord = tileOrigin + int2(int(strip << H_PER_THREAD_LOG), int(row));\n"
        "    if (coord.x < dstOriginAndSize.z && coord.y < dstOriginAndSize.w)\n"
        "    {\n"
        "        uint s = row * %d + strip * %d;\n",
        radius, fRowStride, this->paddedIndex(h));
    for (int j = 0; j < h; ++j) {
        SkSL::String::appendf(sksl, "        half4 h%d = half4(0);\n", j);
    }
    sksl->append("        half4 t;\n");
    for (int i = 0; i < h + 2 * radius; ++i) {
        SkSL::String::appendf(
                sksl, "        t = UnpackHalf4(s_vert[s + %d]);\n", this->paddedIndex(i));
        for (int j = std::max(0, i - 2 * radius); j <= std::min(h - 1, i); ++j) {
            SkSL::String::appendf(
                    sksl, "        h%d += half(weightsX[%d][%d]) * t;\n", j, (i - j) >> 2, (i - j) & 3);
        }
    }
    sksl->append("        textureWrite(dstTexture, uint2(coord), h0);\n");
    for (int j = 1; j < h; ++j) {
        SkSL::String::appendf(sksl,
            "        if (coord.x + %d < dstOriginAndSize.z)\n"
            "            textureWrite(dstTexture, uint2(coord + int2(%d, 0)), h%d);\n",
            j, j, j);
    }
    sksl->append(
        "    }\n"
        "}\n"
        "\n");
}

// Appends a call with `args` to the `function` of the bucket whose radius is the uniform SkSL
// expression `radius`, which must be one of fBucketRadii.
void GaussianBlurStep::appendBucketCalls(std::string* sksl,
                                         const char* radius,
                                         const char* function,
                                         const char* args) const {
    const int bucketCount = fBucketRadii.size();
    for (int i = 0; i < bucketCount; ++i) {
        if (i == 0) {
            SkSL::String::appendf(sksl, "    if (%s == %d)\n", radius, fBucketRadii[i]);
        } else if (i < bucketCount - 1) {
            SkSL::String::appendf(sksl, "    else if (%s == %d)\n", radius, fBucketRadii[i]);
        } else {
            sksl->append("    else\n");
        }
        SkSL::String::appendf(sksl, "        %s%d(%s);\n", function, fBucketRadii[i], args);
    }
}

std::string GaussianBlurStep::computeSkSL() const {
    // The tile mode is applied with integer math against srcRect, so no sampler is needed.
    //
    // The workgroup is 1D: its threads cooperatively blur one TILE_WIDTH x TILE_HEIGHT output tile.
    // The grid of tiles stays 2D, since a 1D grid would pass the 65535 workgroups-per-dimension
    // minimum at large outputs.
    //
    // Each bucket's passes are generated as straight-line code (see appendBlurVertical() and
    // appendBlurHorizontal()), so that the sums are named registers rather than arrays, and
    // every weight is read at a constant index.
    const int v = fTuning.fVerticalSumsPerThread;
    const int hLog = SkPrevLog2(SkTo<uint32_t>(fTuning.fHorizontalSumsPerThread));
    std::string sksl = kSharedBlurSkSL;
    SkSL::String::appendf(&sksl, R"(
        const uint TILE_WIDTH_LOG = %d;
        const uint TILE_HEIGHT_LOG = %d;
        const uint V_PER_THREAD = %d;
        const uint H_PER_THREAD_LOG = %d;
        const uint H_STRIPS_LOG = %d;
        const uint H_STRIPS_MASK = %d;

        //The vertical blur of the tile's rows, at its columns and those of the horizontal halo, as
        //f16 RGBA. The rows are packed at the widest halo's stride%s
        workgroup uint2 s_vert[%d];

        //Returns the source texel in column x (already verified >= 0) and row p
        half4 LoadTexel(int x, int p)
        {
            int y = TileCoord(p, srcRect.y, srcRect.w, tileModeAndRadii.x);
            half4 color = half4(0);
            if (y >= 0)
                color = textureRead(srcTexture, uint2(x, y));
            return color;
        }

    )",
        fTuning.fTileWidthLog,
        fTuning.fTileHeightLog,
        v,
        hLog,
        fTuning.fTileWidthLog - hLog,
        fHorizontalStrips - 1,
        fTuning.fPadSums ? ", with a pad entry after every 4\n"
                           "        //entries so that the strided reads of the horizontal pass "
                           "don't conflict"
                         : "",
        fRowStride * fTileHeight);

    for (int radius : fBucketRadii) {
        this->appendBlurVertical(&sksl, radius);
        this->appendBlurHorizontal(&sksl, radius);
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
        "    uint columns = uint(inside.x + 2 * radii.x);\n",
        fTileWidth, fTileHeight);
    if (fVerticalBlocks == 1) {
        sksl.append("    uint blocks = 1u;\n");
    } else if (fVerticalBlocks == 2) {
        sksl.append("    uint blocks = inside.y > int(V_PER_THREAD) ? 2u : 1u;\n");
    } else {
        SkSL::String::appendf(&sksl,
                              "    uint blocks = uint(inside.y + %d) >> %d;\n",
                              v - 1,
                              SkPrevLog2(SkTo<uint32_t>(v)));
    }
    this->appendBucketCalls(
            &sksl, "radii.y", "BlurVertical", "threadIdx, inputOrigin, columns, blocks");
    sksl.append("    workgroupBarrier();\n");
    this->appendBucketCalls(&sksl, "radii.x", "BlurHorizontal", "threadIdx, tileOrigin");
    sksl.append("}\n");
    return sksl;
}

std::tuple<SkISize, SkColorType> GaussianBlurStep::calculateTextureParameters(
        int resourceIndex, const ResourceDesc&) const {
    SkASSERT(resourceIndex == 1);
    // The size is unused since the output is assigned rather than allocated by the DispatchGroup.
    return {{fTileWidth, fTileHeight}, kColorType};
}

}  // namespace skgpu::graphite
