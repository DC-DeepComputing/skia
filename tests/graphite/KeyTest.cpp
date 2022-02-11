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

    SkPaintParamsKeyBuilder builder(dict);

    builder.beginBlock(dummySnippetID);

    for (int i = 0; i < size; ++i) {
        builder.addByte(i % 256);
    }

    builder.endBlock();

    return builder.snap();
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyTest, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();

    int dummySnippetID = dict->addUserDefinedSnippet();

    std::unique_ptr<SkPaintParamsKey> key = create_key(dict, dummySnippetID,
                                                       SkPaintParamsKey::kMaxBlockSize -
                                                       SkPaintParamsKey::kBlockHeaderSizeInBytes);
    REPORTER_ASSERT(reporter, key->sizeInBytes() == SkPaintParamsKey::kMaxBlockSize);

    key = create_key(dict, dummySnippetID, 1024);
    REPORTER_ASSERT(reporter, key->sizeInBytes() == SkPaintParamsKey::kBlockHeaderSizeInBytes);

    // TODO: add a test checking that an invalid code snippet id yields an error key
}
