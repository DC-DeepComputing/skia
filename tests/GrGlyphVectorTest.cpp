/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkWriteBuffer.h"
#include "src/core/SkZip.h"
#include "src/text/StrikeForGPU.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <initializer_list>
#include <limits.h>
#include <optional>
#include <utility>

using GlyphVector = sktext::gpu::GlyphVector;
using SubRunAllocator = sktext::gpu::SubRunAllocator;

namespace sktext::gpu {
class GlyphVectorTestingPeer {
public:
    static const SkDescriptor& GetDescriptor(const GlyphVector& v) {
        return v.fStrikePromise.descriptor();
    }
    static SkPackedGlyphID GetPackedID(const GlyphVector& v, int i) {
        const auto& g = v.fGlyphs[i];
        if (v.fGetGlyphID) {
            return v.fGetGlyphID(g.data());
        }
        return *reinterpret_cast<const SkPackedGlyphID*>(g.data());
    }
};

DEF_TEST(GlyphVector_Serialization, r) {
    SkFont font = ToolUtils::DefaultFont();
    auto [strikeSpec, _] = SkStrikeSpec::MakeCanonicalized(font);

    SubRunAllocator alloc;

    const int N = 10;
    SkPackedGlyphID* glyphs = alloc.makePODArray<SkPackedGlyphID>(N);
    for (int i = 0; i < N; i++) {
        glyphs[i] = SkPackedGlyphID(0x10000u + i, uint32_t(i % 4), uint32_t((i + 1) % 4));
    }

    SkStrikePromise promise{strikeSpec.findOrCreateStrike()};

    GlyphVector src = GlyphVector::Make(std::move(promise), SkSpan(glyphs, N), &alloc);

    SkBinaryWriteBuffer wBuffer({});
    src.flatten(wBuffer);

    auto data = wBuffer.snapshotAsData();
    SkReadBuffer rBuffer{data->data(), data->size()};
    auto dst = GlyphVector::MakeFromBuffer(rBuffer, nullptr, &alloc);
    REPORTER_ASSERT(r, dst.has_value());
    REPORTER_ASSERT(r,
                    GlyphVectorTestingPeer::GetDescriptor(src) ==
                            GlyphVectorTestingPeer::GetDescriptor(*dst));

    REPORTER_ASSERT(r, src.glyphCount() == dst->glyphCount());
    auto n = std::min(src.glyphCount(), dst->glyphCount());
    for (int i = 0; i < n; ++i) {
        auto s = GlyphVectorTestingPeer::GetPackedID(src, i);
        auto d = GlyphVectorTestingPeer::GetPackedID(*dst, i);
        REPORTER_ASSERT(r, s == d);
    }
}

DEF_TEST(GlyphVector_16BitDeserialize, r) {
    auto [spec, _] = SkStrikeSpec::MakeCanonicalized(ToolUtils::DefaultFont());
    SkStrikePromise promise{spec.findOrCreateStrike()};
    SkBinaryWriteBuffer writer({});
    promise.flatten(writer);
    writer.write32(3);
    for (uint32_t glyph : {0u, 0xFFFEu, 0xFFFFu}) {
        writer.writeUInt((glyph << 2) | 2u | (3u << 18));
    }
    auto data = writer.snapshotAsData();
    SkReadBuffer reader(data->data(), data->size());
    reader.setVersion(SkPicturePriv::k32BitGlyphIDs - 1);
    SubRunAllocator alloc;
    auto vector = GlyphVector::MakeFromBuffer(reader, nullptr, &alloc);
    REPORTER_ASSERT(r, vector && reader.isValid());
    if (vector) {
        REPORTER_ASSERT(r, vector->getPackedGlyphID(0) == SkPackedGlyphID(0u, 2u, 3u));
        REPORTER_ASSERT(r, vector->getPackedGlyphID(1) == SkPackedGlyphID(0xFFFEu, 2u, 3u));
        REPORTER_ASSERT(r, vector->getPackedGlyphID(2) == SkPackedGlyphID(0xFFFFu, 2u, 3u));
    }
}

DEF_TEST(GlyphVector_BadLengths, r) {
    auto [strikeSpec, _] = SkStrikeSpec::MakeCanonicalized(ToolUtils::DefaultFont());

    // Strike to keep in the strike cache.
    auto strike = strikeSpec.findOrCreateStrike();

    // Be sure to keep the strike alive. The promise to serialize as the first part of the
    // GlyphVector.
    SkStrikePromise promise{sk_sp<SkStrike>(strike)};
    {
        // Make broken stream by hand - zero length
        SkBinaryWriteBuffer wBuffer({});
        promise.flatten(wBuffer);
        wBuffer.write32(0);  // length
        auto data = wBuffer.snapshotAsData();
        SkReadBuffer rBuffer{data->data(), data->size()};
        SubRunAllocator alloc;
        auto dst = GlyphVector::MakeFromBuffer(rBuffer, nullptr, &alloc);
        REPORTER_ASSERT(r, !dst.has_value());
    }

    {
        // Make broken stream by hand - zero length
        SkBinaryWriteBuffer wBuffer({});
        promise.flatten(wBuffer);
        // Make broken stream by hand - stream is too short
        wBuffer.write32(5);  // length
        wBuffer.writeUInt(12);  // random data
        wBuffer.writeUInt(12);  // random data
        wBuffer.writeUInt(12);  // random data
        auto data = wBuffer.snapshotAsData();
        SkReadBuffer rBuffer{data->data(), data->size()};
        SubRunAllocator alloc;
        auto dst = GlyphVector::MakeFromBuffer(rBuffer, nullptr, &alloc);
        REPORTER_ASSERT(r, !dst.has_value());
    }

    {
        // Make broken stream by hand - length out of range of safe calculations
        SkBinaryWriteBuffer wBuffer({});
        promise.flatten(wBuffer);
        wBuffer.write32(INT_MAX - 10);  // length
        wBuffer.writeUInt(12);  // random data
        wBuffer.writeUInt(12);  // random data
        wBuffer.writeUInt(12);  // random data
        auto data = wBuffer.snapshotAsData();
        SkReadBuffer rBuffer{data->data(), data->size()};
        SubRunAllocator alloc;
        auto dst = GlyphVector::MakeFromBuffer(rBuffer, nullptr, &alloc);
        REPORTER_ASSERT(r, !dst.has_value());
    }
}

}  // namespace sktext::gpu
