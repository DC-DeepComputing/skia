/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/core/SkPaintParamsKey.h"

namespace {

void force_realloc() {
    static constexpr int kDummyCodeSnippetID = 1000000;
    SkPaintParamsKey key;

    int headerOffset = key.beginBlock(kDummyCodeSnippetID);

    for (int i = 0; i < 1024; ++i) {
        key.addByte(i % 256);
    }

    key.endBlock(headerOffset, kDummyCodeSnippetID);
}


} // anonymous namespace


DEF_GRAPHITE_TEST(KeyTest, reporter) {




}
