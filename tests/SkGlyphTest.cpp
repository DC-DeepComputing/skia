/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/gpu/PackedGPUGlyphID.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <optional>

DEF_TEST(SkGlyphRectBasic, reporter) {
    using namespace skglyph;
    SkGlyphRect r{1, 1, 10, 10};
    REPORTER_ASSERT(reporter, !r.empty());
    SkGlyphRect a = rect_union(r, empty_rect());
    REPORTER_ASSERT(reporter, a.rect() == SkRect::MakeLTRB(1, 1, 10, 10));
    auto widthHeight = a.widthHeight();
    REPORTER_ASSERT(reporter, widthHeight.x() == 9 && widthHeight.y() == 9);

    a = rect_intersection(r, full_rect());
    REPORTER_ASSERT(reporter, a.rect() == SkRect::MakeLTRB(1, 1, 10, 10));

    SkGlyphRect acc = full_rect();
    for (int x = -10; x < 10; x++) {
        for(int y = -10; y < 10; y++) {
            acc = rect_intersection(acc, SkGlyphRect(x, y, x + 20, y + 20));
        }
    }
    REPORTER_ASSERT(reporter, acc.rect() == SkRect::MakeLTRB(9, 9, 10, 10));

    acc = empty_rect();
    for (int x = -10; x < 10; x++) {
        for(int y = -10; y < 10; y++) {
            acc = rect_union(acc, SkGlyphRect(x, y, x + 20, y + 20));
        }
    }
    REPORTER_ASSERT(reporter, acc.rect() == SkRect::MakeLTRB(-10, -10, 29, 29));
}

class SkGlyphTestPeer {
public:
    static void SetGlyph1(SkGlyph* glyph) {
        glyph->fAdvanceX = 10;
        glyph->fAdvanceY = 11;
        glyph->fLeft = -1;
        glyph->fTop = -2;
        glyph->fWidth = 8;
        glyph->fHeight = 9;
        glyph->fMaskFormat = SkMask::Format::kA8_Format;
    }

    static void SetGlyph2(SkGlyph* glyph) {
        glyph->fAdvanceX = 10;
        glyph->fAdvanceY = 11;
        glyph->fLeft = 0;
        glyph->fTop = -1;
        glyph->fWidth = 8;
        glyph->fHeight = 9;
        glyph->fMaskFormat = SkMask::Format::kA8_Format;
    }
};

DEF_TEST(SkPackedGlyphID_32Bit, reporter) {
    for (SkGlyphID glyph : {0u, 1u, 0xFFFFu, 0x10000u, 0x10001u, 0xFFFFFFu,
                           0x80000000u, 0xFFFFFFFFu}) {
        for (uint32_t x = 0; x < 4; ++x) {
            for (uint32_t y = 0; y < 4; ++y) {
                SkPackedGlyphID id(glyph, x, y);
                REPORTER_ASSERT(reporter, id.glyphID() == glyph);
                REPORTER_ASSERT(reporter, id.getSubXFixed() == SkFixed(x << 14));
                REPORTER_ASSERT(reporter, id.getSubYFixed() == SkFixed(y << 14));
                REPORTER_ASSERT(reporter, id != SkPackedGlyphID());
                REPORTER_ASSERT(reporter, SkPackedGlyphID::FromRaw(id.value()) == id);
                REPORTER_ASSERT(reporter,
                                SkPackedGlyphID(glyph, SkFixed(x << 14), SkFixed(y << 14)) == id);
                REPORTER_ASSERT(reporter,
                                SkPackedGlyphID(glyph, {x * 0.25f - 1, y * 0.25f - 1},
                                                SkPackedGlyphID::kXYFieldMask) == id);
                SkBinaryWriteBuffer writer({});
                id.flatten(writer);
                auto data = writer.snapshotAsData();
                SkReadBuffer reader(data->data(), data->size());
                REPORTER_ASSERT(reporter, SkPackedGlyphID::MakeFromBuffer(reader) == id);
                REPORTER_ASSERT(reporter, reader.isValid());

                SkGlyph src(id);
                SkGlyphTestPeer::SetGlyph1(&src);
                SkGlyphDigest digest(123, src);
                REPORTER_ASSERT(reporter, SkGlyphDigest::GetKey(digest) == id);
                REPORTER_ASSERT(reporter, digest.index() == 123);
                SkBinaryWriteBuffer metricsWriter({});
                src.flattenMetrics(metricsWriter);
                auto metricsData = metricsWriter.snapshotAsData();
                SkReadBuffer metricsReader(metricsData->data(), metricsData->size());
                auto dst = SkGlyph::MakeFromBuffer(metricsReader);
                REPORTER_ASSERT(reporter, dst && dst->getPackedID() == id);
            }
        }
    }
    REPORTER_ASSERT(reporter, SkPackedGlyphID(1).hash() != SkPackedGlyphID(0x10001).hash());
}

DEF_TEST(PackedGPUGlyphID_32Bit, reporter) {
    using sktext::gpu::PackedGPUGlyphID;
    for (SkGlyphID glyph : {1u, 0x10001u, 0xFFFFFFFFu}) {
        SkPackedGlyphID id(glyph, 2u, 3u);
        for (auto format : {skgpu::MaskFormat::kA8, skgpu::MaskFormat::kA565,
                            skgpu::MaskFormat::kARGB}) {
            for (int padding = 0; padding <= 2; ++padding) {
                for (bool isSDF : {false, true}) {
                    PackedGPUGlyphID gpuID(id, format, padding, isSDF);
                    REPORTER_ASSERT(reporter, gpuID.packedGlyphID() == id);
                    REPORTER_ASSERT(reporter, gpuID.maskFormat() == format);
                    REPORTER_ASSERT(reporter, gpuID.padding() == padding);
                    REPORTER_ASSERT(reporter, gpuID.isSDF() == isSDF);
                }
            }
        }
    }
}

DEF_TEST(SkPackedGlyphID_16BitDeserialize, reporter) {
    SkBinaryWriteBuffer writer({});
    writer.writeUInt((0xFFFFu << 2) | 2 | (3u << 18));
    auto data = writer.snapshotAsData();
    SkReadBuffer reader(data->data(), data->size());
    reader.setVersion(SkPicturePriv::k32BitGlyphIDs - 1);
    REPORTER_ASSERT(reporter,
                    SkPackedGlyphID::MakeFromBuffer(reader) == SkPackedGlyphID(0xFFFFu, 2u, 3u));
    REPORTER_ASSERT(reporter, reader.isValid());
}

DEF_TEST(SkGlyph_SendMetrics, reporter) {
    {
        SkGlyph srcGlyph{SkPackedGlyphID{(SkGlyphID)12}};
        SkGlyphTestPeer::SetGlyph1(&srcGlyph);

        SkBinaryWriteBuffer writeBuffer({});
        srcGlyph.flattenMetrics(writeBuffer);

        sk_sp<SkData> data = writeBuffer.snapshotAsData();

        SkReadBuffer readBuffer{data->data(), data->size()};
        std::optional<SkGlyph> dstGlyph = SkGlyph::MakeFromBuffer(readBuffer);
        REPORTER_ASSERT(reporter, readBuffer.isValid());
        REPORTER_ASSERT(reporter, dstGlyph.has_value());
        REPORTER_ASSERT(reporter, srcGlyph.advanceVector() == dstGlyph->advanceVector());
        REPORTER_ASSERT(reporter, srcGlyph.rect() == dstGlyph->rect());
        REPORTER_ASSERT(reporter, srcGlyph.maskFormat() == dstGlyph->maskFormat());
    }
    {
        SkGlyph srcGlyph{SkPackedGlyphID{(SkGlyphID)12}};
        SkGlyphTestPeer::SetGlyph2(&srcGlyph);

        SkBinaryWriteBuffer writeBuffer({});
        srcGlyph.flattenMetrics(writeBuffer);

        sk_sp<SkData> data = writeBuffer.snapshotAsData();

        SkReadBuffer readBuffer{data->data(), data->size()};
        std::optional<SkGlyph> dstGlyph = SkGlyph::MakeFromBuffer(readBuffer);
        REPORTER_ASSERT(reporter, readBuffer.isValid());
        REPORTER_ASSERT(reporter, dstGlyph.has_value());
        REPORTER_ASSERT(reporter, srcGlyph.advanceVector() == dstGlyph->advanceVector());
        REPORTER_ASSERT(reporter, srcGlyph.rect() == dstGlyph->rect());
        REPORTER_ASSERT(reporter, srcGlyph.maskFormat() == dstGlyph->maskFormat());
    }

    uint8_t badData[] = {1, 2, 3, 4, 5, 6, 7, 8};
    SkReadBuffer badBuffer{badData, std::size(badData)};
    std::optional<SkGlyph> dstGlyph = SkGlyph::MakeFromBuffer(badBuffer);
    REPORTER_ASSERT(reporter, !badBuffer.isValid());
    REPORTER_ASSERT(reporter, !dstGlyph.has_value());
}

DEF_TEST(SkGlyph_SendWithImage, reporter) {
    SkArenaAlloc alloc{256};
    SkGlyph srcGlyph{SkPackedGlyphID{(SkGlyphID)12}};
    SkGlyphTestPeer::SetGlyph1(&srcGlyph);

    static constexpr uint8_t X = 0xff;
    static constexpr uint8_t O = 0x00;
    uint8_t imageData[][8] = {
        {X,X,X,X,X,X,X,X},
        {X,O,O,O,O,O,O,X},
        {X,O,O,O,O,O,O,X},
        {X,O,O,O,O,O,O,X},
        {X,O,O,X,X,O,O,X},
        {X,O,O,O,O,O,O,X},
        {X,O,O,O,O,O,O,X},
        {X,O,O,O,O,O,O,X},
        {X,X,X,X,X,X,X,X},
    };

    srcGlyph.setImage(&alloc, imageData);

    SkBinaryWriteBuffer writeBuffer({});
    srcGlyph.flattenMetrics(writeBuffer);
    srcGlyph.flattenImage(writeBuffer);

    sk_sp<SkData> data = writeBuffer.snapshotAsData();

    SkReadBuffer readBuffer{data->data(), data->size()};
    std::optional<SkGlyph> dstGlyph = SkGlyph::MakeFromBuffer(readBuffer);
    REPORTER_ASSERT(reporter, readBuffer.isValid());
    REPORTER_ASSERT(reporter, dstGlyph.has_value());
    REPORTER_ASSERT(reporter, srcGlyph.advanceVector() == dstGlyph->advanceVector());
    REPORTER_ASSERT(reporter, srcGlyph.rect() == dstGlyph->rect());
    REPORTER_ASSERT(reporter, srcGlyph.maskFormat() == dstGlyph->maskFormat());

    dstGlyph->addImageFromBuffer(readBuffer, &alloc);
    REPORTER_ASSERT(reporter, readBuffer.isValid());
    const uint8_t* dstImage = (const uint8_t*)dstGlyph->image();
    for (int y = 0; y < dstGlyph->height(); ++y) {
        for (int x = 0; x < dstGlyph->width(); ++x) {
            REPORTER_ASSERT(reporter, imageData[y][x] == dstImage[y * dstGlyph->rowBytes() + x]);
        }
    }

    // Add good metrics, but mess up image data
    SkBinaryWriteBuffer badWriteBuffer({});
    srcGlyph.flattenMetrics(badWriteBuffer);
    badWriteBuffer.writeInt(7);
    badWriteBuffer.writeInt(8);

    data = badWriteBuffer.snapshotAsData();

    SkReadBuffer badReadBuffer{data->data(), data->size()};
    dstGlyph = SkGlyph::MakeFromBuffer(badReadBuffer);
    REPORTER_ASSERT(reporter, badReadBuffer.isValid());  // Reading glyph metrics is okay.
    REPORTER_ASSERT(reporter, dstGlyph.has_value());
    REPORTER_ASSERT(reporter, srcGlyph.advanceVector() == dstGlyph->advanceVector());
    REPORTER_ASSERT(reporter, srcGlyph.rect() == dstGlyph->rect());
    REPORTER_ASSERT(reporter, srcGlyph.maskFormat() == dstGlyph->maskFormat());

    dstGlyph->addImageFromBuffer(badReadBuffer, &alloc);
    REPORTER_ASSERT(reporter, !badReadBuffer.isValid());
    REPORTER_ASSERT(reporter, !dstGlyph->setImageHasBeenCalled());
}

DEF_TEST(SkGlyph_SendWithPath, reporter) {
    SkArenaAlloc alloc{256};
    SkGlyph srcGlyph{SkPackedGlyphID{(SkGlyphID)12}};
    SkGlyphTestPeer::SetGlyph1(&srcGlyph);

    SkPath srcPath = SkPath::Rect(srcGlyph.rect());

    srcGlyph.setPath(&alloc, &srcPath, false, false);

    SkBinaryWriteBuffer writeBuffer({});
    srcGlyph.flattenMetrics(writeBuffer);
    srcGlyph.flattenPath(writeBuffer);

    sk_sp<SkData> data = writeBuffer.snapshotAsData();

    SkReadBuffer readBuffer{data->data(), data->size()};
    std::optional<SkGlyph> dstGlyph = SkGlyph::MakeFromBuffer(readBuffer);
    REPORTER_ASSERT(reporter, readBuffer.isValid());
    REPORTER_ASSERT(reporter, dstGlyph.has_value());
    REPORTER_ASSERT(reporter, srcGlyph.advanceVector() == dstGlyph->advanceVector());
    REPORTER_ASSERT(reporter, srcGlyph.rect() == dstGlyph->rect());
    REPORTER_ASSERT(reporter, srcGlyph.maskFormat() == dstGlyph->maskFormat());

    dstGlyph->addPathFromBuffer(readBuffer, &alloc);
    REPORTER_ASSERT(reporter, readBuffer.isValid());
    REPORTER_ASSERT(reporter, dstGlyph->setPathHasBeenCalled());
    const SkPath* dstPath = dstGlyph->path();
    REPORTER_ASSERT(reporter, *dstPath == srcPath);

    {
        // Add good metrics, but mess up path data
        SkBinaryWriteBuffer badWriteBuffer({});
        srcGlyph.flattenMetrics(badWriteBuffer);
        // Force a false value to be read in addPathFromBuffer for hasPath.
        badWriteBuffer.writeInt(8);
        badWriteBuffer.writeInt(9);

        data = badWriteBuffer.snapshotAsData();

        SkReadBuffer badReadBuffer{data->data(), data->size()};
        dstGlyph = SkGlyph::MakeFromBuffer(badReadBuffer);
        REPORTER_ASSERT(reporter, dstGlyph.has_value());
        REPORTER_ASSERT(reporter, srcGlyph.advanceVector() == dstGlyph->advanceVector());
        REPORTER_ASSERT(reporter, srcGlyph.rect() == dstGlyph->rect());
        REPORTER_ASSERT(reporter, srcGlyph.maskFormat() == dstGlyph->maskFormat());

        dstGlyph->addPathFromBuffer(badReadBuffer, &alloc);
        REPORTER_ASSERT(reporter, !badReadBuffer.isValid());
        REPORTER_ASSERT(reporter, !dstGlyph->setPathHasBeenCalled());
    }
    {
        // Add good metrics, but no path data.
        SkBinaryWriteBuffer badWriteBuffer({});
        srcGlyph.flattenMetrics(badWriteBuffer);

        data = badWriteBuffer.snapshotAsData();

        SkReadBuffer badReadBuffer{data->data(), data->size()};
        dstGlyph = SkGlyph::MakeFromBuffer(badReadBuffer);
        REPORTER_ASSERT(reporter, badReadBuffer.isValid());  // Reading glyph metrics is okay.
        REPORTER_ASSERT(reporter, dstGlyph.has_value());
        REPORTER_ASSERT(reporter, srcGlyph.advanceVector() == dstGlyph->advanceVector());
        REPORTER_ASSERT(reporter, srcGlyph.rect() == dstGlyph->rect());
        REPORTER_ASSERT(reporter, srcGlyph.maskFormat() == dstGlyph->maskFormat());

        dstGlyph->addPathFromBuffer(badReadBuffer, &alloc);
        REPORTER_ASSERT(reporter, !badReadBuffer.isValid());
        REPORTER_ASSERT(reporter, !dstGlyph->setPathHasBeenCalled());
    }
}

DEF_TEST(SkGlyph_SendWithDrawable, reporter) {
    SkArenaAlloc alloc{256};
    SkGlyph srcGlyph{SkPackedGlyphID{(SkGlyphID)12}};
    SkGlyphTestPeer::SetGlyph1(&srcGlyph);

    class TestDrawable final : public SkDrawable {
    public:
        TestDrawable(SkRect rect) : fRect(rect) {}
    private:
        const SkRect fRect;
        SkRect onGetBounds() override { return fRect;  }
        size_t onApproximateBytesUsed() override {
            return 0;
        }
        void onDraw(SkCanvas* canvas) override {
            SkPaint paint;
            canvas->drawRect(fRect, paint);
        }
    };

    sk_sp<SkDrawable> srcDrawable = sk_make_sp<TestDrawable>(srcGlyph.rect());
    srcGlyph.setDrawable(&alloc, srcDrawable);
    REPORTER_ASSERT(reporter, srcGlyph.setDrawableHasBeenCalled());

    SkBinaryWriteBuffer writeBuffer({});
    srcGlyph.flattenMetrics(writeBuffer);
    srcGlyph.flattenDrawable(writeBuffer);

    sk_sp<SkData> data = writeBuffer.snapshotAsData();

    SkReadBuffer readBuffer{data->data(), data->size()};
    std::optional<SkGlyph> dstGlyph = SkGlyph::MakeFromBuffer(readBuffer);
    REPORTER_ASSERT(reporter, readBuffer.isValid());
    REPORTER_ASSERT(reporter, dstGlyph.has_value());
    REPORTER_ASSERT(reporter, srcGlyph.advanceVector() == dstGlyph->advanceVector());
    REPORTER_ASSERT(reporter, srcGlyph.rect() == dstGlyph->rect());
    REPORTER_ASSERT(reporter, srcGlyph.maskFormat() == dstGlyph->maskFormat());

    dstGlyph->addDrawableFromBuffer(readBuffer, &alloc);
    REPORTER_ASSERT(reporter, readBuffer.isValid());
    REPORTER_ASSERT(reporter, dstGlyph->setDrawableHasBeenCalled());
    SkDrawable* dstDrawable = dstGlyph->drawable();
    REPORTER_ASSERT(reporter, dstDrawable->getBounds() == srcDrawable->getBounds());

    // Add good metrics, but mess up drawable data
    SkBinaryWriteBuffer badWriteBuffer({});
    srcGlyph.flattenMetrics(badWriteBuffer);
    badWriteBuffer.writeInt(7);
    badWriteBuffer.writeInt(8);

    data = badWriteBuffer.snapshotAsData();

    SkReadBuffer badReadBuffer{data->data(), data->size()};
    dstGlyph = SkGlyph::MakeFromBuffer(badReadBuffer);
    REPORTER_ASSERT(reporter, badReadBuffer.isValid());  // Reading glyph metrics is okay.
    REPORTER_ASSERT(reporter, dstGlyph.has_value());
    REPORTER_ASSERT(reporter, srcGlyph.advanceVector() == dstGlyph->advanceVector());
    REPORTER_ASSERT(reporter, srcGlyph.rect() == dstGlyph->rect());
    REPORTER_ASSERT(reporter, srcGlyph.maskFormat() == dstGlyph->maskFormat());

    dstGlyph->addDrawableFromBuffer(badReadBuffer, &alloc);
    REPORTER_ASSERT(reporter, !badReadBuffer.isValid());
    REPORTER_ASSERT(reporter, !dstGlyph->setDrawableHasBeenCalled());
}

DEF_TEST(SkPictureBackedGlyphDrawable_Basic, reporter) {
    class TestDrawable final : public SkDrawable {
    public:
        TestDrawable(SkRect rect) : fRect(rect) {}
    private:
        const SkRect fRect;
        SkRect onGetBounds() override { return fRect;  }
        size_t onApproximateBytesUsed() override {
            return 0;
        }
        void onDraw(SkCanvas* canvas) override {
            SkPaint paint;
            canvas->drawRect(fRect, paint);
        }
    };

    sk_sp<SkDrawable> srcDrawable = sk_make_sp<TestDrawable>(SkRect::MakeWH(10, 20));
    SkBinaryWriteBuffer writeBuffer({});
    SkPictureBackedGlyphDrawable::FlattenDrawable(writeBuffer, srcDrawable.get());

    sk_sp<SkData> data = writeBuffer.snapshotAsData();

    SkReadBuffer readBuffer{data->data(), data->size()};

    sk_sp<SkPictureBackedGlyphDrawable> dstDrawable =
            SkPictureBackedGlyphDrawable::MakeFromBuffer(readBuffer);

    REPORTER_ASSERT(reporter, readBuffer.isValid());
    REPORTER_ASSERT(reporter, dstDrawable != nullptr);
    REPORTER_ASSERT(reporter, srcDrawable->getBounds() == dstDrawable->getBounds());

    SkBinaryWriteBuffer badWriteBuffer({});
    badWriteBuffer.writeInt(7);
    badWriteBuffer.writeInt(8);

    data = badWriteBuffer.snapshotAsData();

    SkReadBuffer badReadBuffer{data->data(), data->size()};

    sk_sp<SkPictureBackedGlyphDrawable> badDrawable =
            SkPictureBackedGlyphDrawable::MakeFromBuffer(badReadBuffer);
    REPORTER_ASSERT(reporter, badDrawable == nullptr);
    REPORTER_ASSERT(reporter, !badReadBuffer.isValid());
}

namespace {

// There is currently on SkMaskFilter that directly creates runtime effects
enum class SkSLSource {
    kShader, kColorFilter, kBlender, kImageFilter
};

} // anonymous namespace

static sk_sp<SkDrawable> make_sksl_drawable(SkSLSource source) {
    SkRect rect = SkRect::MakeWH(50, 50);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(rect);

    SkPaint paint;

    switch (source) {
        case SkSLSource::kShader: {
            const sk_sp<SkRuntimeEffect> effect = SkRuntimeEffect::MakeForShader(
                    SkString("half4 main(float2 xy) { return half4(0, 1, 0, 1); }"))
                    .effect;
            SkASSERT(effect);

            paint.setShader(effect->makeShader(/*uniforms=*/nullptr, /*children=*/{}));
            break;
        }

        case SkSLSource::kColorFilter: {
            const sk_sp<SkRuntimeEffect> effect = SkRuntimeEffect::MakeForColorFilter(
                    SkString("half4 main(half4 color) { return 0.5 * color; }"))
                    .effect;
            SkASSERT(effect);

            paint.setColorFilter(effect->makeColorFilter(/*uniforms=*/nullptr, /*children=*/{}));
            break;
        }

        case SkSLSource::kBlender: {
            const sk_sp<SkRuntimeEffect> effect = SkRuntimeEffect::MakeForBlender(
                    SkString("half4 main(half4 src, half4 dst) { return src + dst; }"))
                    .effect;
            SkASSERT(effect);

            paint.setBlender(effect->makeBlender(/*uniforms=*/nullptr, /*children=*/{}));
            break;
        }

        case SkSLSource::kImageFilter: {
            const sk_sp<SkRuntimeEffect> effect =
                    SkRuntimeEffect::MakeForShader(
                            SkString("uniform shader child;"
                                     "half4 main(float2 xy) { return 0.5 * child.eval(xy); }"))
                            .effect;
            SkASSERT(effect);

            SkRuntimeEffectBuilder builder{std::move(effect)};
            paint.setImageFilter(SkImageFilters::RuntimeShader(builder, "child", nullptr));
            break;
        }
    }

    // See note in make_nested_sksl_drawable: We include enough ops that this drawable will be
    // preserved as a sub-picture when we wrap it in a second layer.
    for (int i = 0; i < kMaxPictureOpsToUnrollInsteadOfRef + 1; ++i) {
        canvas->drawRect(rect, paint);
    }

    return recorder.finishRecordingAsDrawable();
}

static sk_sp<SkDrawable> make_nested_sksl_drawable(SkSLSource source) {
    SkRect rect = SkRect::MakeWH(50, 50);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(rect);

    auto sksl_drawable = make_sksl_drawable(source);
    sk_sp<SkPicture> sksl_picture = sksl_drawable->makePictureSnapshot();

    // We need to ensure that the op count of our picture is larger than this threshold, so we
    // actually get a nested (embedded) picture, rather than just playing the ops back.
    SkASSERT(sksl_picture->approximateOpCount() > kMaxPictureOpsToUnrollInsteadOfRef);
    canvas->drawPicture(sksl_picture);

    return recorder.finishRecordingAsDrawable();
}

DEF_TEST(SkPictureBackedGlyphDrawable_RejectsAnyShadersThatNeedSkSL, reporter) {
    for (SkSLSource source : {SkSLSource::kShader,
                              SkSLSource::kColorFilter,
                              SkSLSource::kBlender,
                              SkSLSource::kImageFilter}) {
        for (const sk_sp<SkDrawable>& drawable : {make_sksl_drawable(source),
                                                  make_nested_sksl_drawable(source)}) {
            // Make sure that regardless if the passed in buffer has sksl set or not, we
            // don't deserialize it.
            for (bool allowSkSL : {true, false}) {
                skiatest::ReporterContext ctx{reporter,
                        SkStringPrintf("source: %d, allow sksl: %d", (int) source, allowSkSL)};

                REPORTER_ASSERT(reporter, drawable);

                SkBinaryWriteBuffer writeBuffer({});
                SkPictureBackedGlyphDrawable::FlattenDrawable(writeBuffer, drawable.get());

                sk_sp<SkData> data = writeBuffer.snapshotAsData();
                REPORTER_ASSERT(reporter, data);
                REPORTER_ASSERT(reporter, data->size() > 0);

                SkReadBuffer readBuffer{data->data(), data->size()};
                readBuffer.setAllowSkSL(allowSkSL);

                sk_sp<SkPictureBackedGlyphDrawable> dstDrawable =
                        SkPictureBackedGlyphDrawable::MakeFromBuffer(readBuffer);

                REPORTER_ASSERT(reporter, !readBuffer.isValid());
                REPORTER_ASSERT(reporter, !dstDrawable);
            }
        }
    }
}
