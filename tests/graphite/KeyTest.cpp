/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/core/SkPaintParamsKey.h"
#include "src/core/SkShaderCodeDictionary.h"

#include "src/gpu/graphite/ContextPriv.h"

namespace {

SkPaintParamsKey create_key(SkPaintParamsKeyBuilder* builder, SkSnippetID snippetID, int size) {

    SkASSERT(size <= 1024);
    static const uint8_t kDummyData[1024] = { 0 };

    SkDEBUGCODE(builder->checkReset());

    builder->beginBlock(snippetID);

    builder->addBytes(size, kDummyData);

    builder->endBlock();

    return builder->lockAsKey();
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyTest, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();

    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    // invalid code snippet ID
    {
        SkPaintParamsKey key1 = create_key(&builder, SkSnippetID(), 32);
        // key creation fails
        REPORTER_ASSERT(reporter, key1.isErrorKey());

        SkPaintParamsKey key2 = create_key(&builder, SkSnippetID(kBuiltInCodeSnippetIDCount2), 32);
        // key creation fails
        REPORTER_ASSERT(reporter, key2.isErrorKey());
    }

    // _Just_ on the edge of being too big
    {
        static const int kMaxBlockDataSize = SkPaintParamsKey::kMaxBlockSize -
                                             SkPaintParamsKey::kBlockHeaderSizeInBytes;

        static constexpr int kNumFields1 = 1;
        static constexpr SkPaintParamsKey::DataPayloadField kDataFields1[kNumFields1] = {
            { "data", SkPaintParamsKey::DataPayloadType::kByte, kMaxBlockDataSize },
        };

        SkSnippetID snippetID1 = dict->addUserDefinedSnippet("keyAlmostTooBig",
                                                             SkMakeSpan(kDataFields1, kNumFields1));

        SkPaintParamsKey key = create_key(&builder, snippetID1, kMaxBlockDataSize);
        REPORTER_ASSERT(reporter, key.sizeInBytes() == SkPaintParamsKey::kMaxBlockSize);
    }

    // Too big
    {
        static constexpr int kNumFields2 = 1;
        static constexpr SkPaintParamsKey::DataPayloadField kDataFields2[kNumFields2] = {
                { "data", SkPaintParamsKey::DataPayloadType::kByte, 1024 },
        };

        SkSnippetID snippetID2 = dict->addUserDefinedSnippet("keyTooBig",
                                                             SkMakeSpan(kDataFields2, kNumFields2));

        SkPaintParamsKey key = create_key(&builder, snippetID2, 1024);
        // key creation fails
        REPORTER_ASSERT(reporter, key.isErrorKey());
    }
}
