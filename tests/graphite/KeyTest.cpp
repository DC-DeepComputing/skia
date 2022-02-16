/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkShaderCodeDictionary.h"

#include "experimental/graphite/src/ContextPriv.h"

namespace {

std::unique_ptr<SkPaintParamsKey> create_key(SkShaderCodeDictionary* dict,
                                             int dummySnippetID,
                                             int size) {

    SkASSERT(size <= 1024);
    static const uint8_t kDummyData[1024] = { 0 };

    SkPaintParamsKeyBuilder builder(dict);

    builder.beginBlock(dummySnippetID);

    builder.addBytes(size, kDummyData);

    builder.endBlock();

    return builder.snap();
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyTest, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();

    static const int kMaxBlockDataSize = SkPaintParamsKey::kMaxBlockSize -
                                         SkPaintParamsKey::kBlockHeaderSizeInBytes;

    std::unique_ptr<SkPaintParamsKey> key;

    // invalid code snippet ID
    {
        key = create_key(dict, kBuiltInCodeSnippetIDCount, kMaxBlockDataSize);
        // key creation fails
        REPORTER_ASSERT(reporter, key->sizeInBytes() == SkPaintParamsKey::kBlockHeaderSizeInBytes);
    }

    // _Just_ on the edge of being too big
    {
        static constexpr int kNumDataPayloadEntries1 = 1;
        static constexpr SkDataPayloadInfo kDataPayloadEntries1[kNumDataPayloadEntries1] = {
            { "data", DataPayloadType::kByte, kMaxBlockDataSize },
        };

        int dummySnippetID1 = dict->addUserDefinedSnippet(SkMakeSpan(kDataPayloadEntries1,
                                                                     kNumDataPayloadEntries1));

        key = create_key(dict, dummySnippetID1, kMaxBlockDataSize);
        REPORTER_ASSERT(reporter, key->sizeInBytes() == SkPaintParamsKey::kMaxBlockSize);
    }

    // Too big
    {
        static constexpr int kNumDataPayloadEntries2 = 1;
        static constexpr SkDataPayloadInfo kDataPayloadEntries2[kNumDataPayloadEntries2] = {
                { "data", DataPayloadType::kByte, 1024 },
        };

        int dummySnippetID2 = dict->addUserDefinedSnippet(SkMakeSpan(kDataPayloadEntries2,
                                                                     kNumDataPayloadEntries2));

        key = create_key(dict, dummySnippetID2, 1024);
        // key creation fails
        REPORTER_ASSERT(reporter, key->sizeInBytes() == SkPaintParamsKey::kBlockHeaderSizeInBytes);
    }
}
