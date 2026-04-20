/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * OSS-Fuzz harness for SkOTUtils::RenameFont.
 *
 * Exercises the SFNT name-table rewriting path that was vulnerable to a
 * heap-buffer-overflow WRITE when tableEntry.offset exceeded the allocated
 * buffer size (CVE-XXXX-XXXX, reported by XlabAI / Tencent Xuanwu Lab).
 *
 * Build (standalone, with ASAN):
 *   clang++ -fsanitize=address -g -std=c++17 \
 *       -DSK_BUILD_FOR_LIBFUZZER \
 *       -I/skia -I/skia/include/core -I/skia/include/config \
 *       -I/skia/include/private -I/skia/src \
 *       FuzzRenameFont.cpp /skia/out/ASAN/libskia.a \
 *       -o FuzzRenameFont \
 *       $(pkg-config --libs freetype2 fontconfig) -lc++ -lc++abi -ldl -lpthread
 *
 * Run against seed corpus:
 *   ./FuzzRenameFont rename_font_seed_corpus/
 */

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/sfnt/SkOTUtils.h"

bool FuzzRenameFont(const uint8_t* data, size_t size) {
    auto stream = SkMemoryStream::MakeDirect(data, size);
    sk_sp<SkData> result(SkOTUtils::RenameFont(stream.get(), "Fuzz", 4));
    return result != nullptr;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Cap at 1 MB to keep fuzzing throughput high.
    if (size > 1 << 20) {
        return 0;
    }
    FuzzRenameFont(data, size);
    return 0;
}
#endif
