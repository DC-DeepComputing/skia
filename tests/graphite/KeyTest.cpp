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

DEF_GRAPHITE_TEST_FOR_CONTEXTS(KeyTest, reporter, context) {

    auto dict = context->priv().shaderCodeDictionary();

    int dummySnippetID = dict->addUserDefinedSnippet();

    std::unique_ptr<SkPaintParamsKey> key;

    {
        SkPaintParamsKeyBuilder builder(dict);

        builder.beginBlock(dummySnippetID);

        for (int i = 0; i < 1024; ++i) {
            builder.addByte(i % 256);
        }

        builder.endBlock();

        key = builder.snap();
    }



}
