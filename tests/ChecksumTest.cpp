/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkAlign.h"
#include "src/base/SkRandom.h"
#include "src/core/SkChecksum.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

DEF_TEST(Checksum, r) {
    // Put 128 random bytes into two identical buffers.  Any multiple of 4 will do.
    const size_t kBytes = SkAlign4(128);
    SkRandom rand;
    uint32_t data[kBytes/4], tweaked[kBytes/4];
    for (size_t i = 0; i < std::size(tweaked); ++i) {
        data[i] = tweaked[i] = rand.nextU();
    }

    // Hash of nullptr is always 0.
    REPORTER_ASSERT(r, SkChecksum::Hash(nullptr, 0) == 0);

    const uint32_t hash = SkChecksum::Hash(data, kBytes);
    // Should be deterministic.
    REPORTER_ASSERT(r, hash == SkChecksum::Hash(data, kBytes));

    // Changing any single element should change the hash.
    for (size_t j = 0; j < std::size(tweaked); ++j) {
        const uint32_t saved = tweaked[j];
        tweaked[j] = rand.nextU();
        const uint32_t tweakedHash = SkChecksum::Hash(tweaked, kBytes);
        REPORTER_ASSERT(r, tweakedHash != hash);
        REPORTER_ASSERT(r, tweakedHash == SkChecksum::Hash(tweaked, kBytes));
        tweaked[j] = saved;
    }
}

DEF_TEST(GoodHash, r) {
    // 4 bytes --> hits SkChecksum::Mix fast path.
    REPORTER_ASSERT(r, SkGoodHash()(( int32_t)4) ==  614249093);
    REPORTER_ASSERT(r, SkGoodHash()((uint32_t)4) ==  614249093);
}

DEF_TEST(ChecksumCollisions, r) {
    // We noticed a few workloads that would cause hash collisions due to the way our old
    // optimized hashes split into three concurrent hashes and merged those hashes together.
    //
    // One of these two workloads ought to cause an unintentional hash collision on very similar
    // data in those old algorithms, the float version on 32-bit x86 and double elsewhere.
    {
        float a[9] = { 0, 1, 2,
                       3, 4, 5,
                       6, 7, 8, };
        float b[9] = { 1, 2, 0,
                       4, 5, 3,
                       7, 8, 6, };

        REPORTER_ASSERT(r, SkChecksum::Hash(a, sizeof(a)) != SkChecksum::Hash(b, sizeof(b)));
    }
    {
        double a[9] = { 0, 1, 2,
                        3, 4, 5,
                        6, 7, 8, };
        double b[9] = { 1, 2, 0,
                        4, 5, 3,
                        7, 8, 6, };

        REPORTER_ASSERT(r, SkChecksum::Hash(a, sizeof(a)) != SkChecksum::Hash(b, sizeof(b)));
    }
}

DEF_TEST(ChecksumConsistent, r) {
    // We've decided to make SkChecksum::Hash() always return consistent results, so spot check a few:
    uint8_t bytes[256];
    for (int i = 0; i < 256; i++) {
        bytes[i] = i;
    }
    REPORTER_ASSERT(r, SkChecksum::Hash(bytes,  0) == 0x00000000, "%08x", SkChecksum::Hash(bytes,  0));
    REPORTER_ASSERT(r, SkChecksum::Hash(bytes,  1) == 0x63e36db1, "%08x", SkChecksum::Hash(bytes,  1));
    REPORTER_ASSERT(r, SkChecksum::Hash(bytes,  2) == 0xdeaf2c91, "%08x", SkChecksum::Hash(bytes,  2));
    REPORTER_ASSERT(r, SkChecksum::Hash(bytes,  7) == 0x3e7941e3, "%08x", SkChecksum::Hash(bytes,  7));
    REPORTER_ASSERT(r, SkChecksum::Hash(bytes, 32) == 0x72a751d8, "%08x", SkChecksum::Hash(bytes, 32));
    REPORTER_ASSERT(r, SkChecksum::Hash(bytes, 63) == 0xb5692afe, "%08x", SkChecksum::Hash(bytes, 63));
    REPORTER_ASSERT(r, SkChecksum::Hash(bytes, 64) == 0x5a8cd37e, "%08x", SkChecksum::Hash(bytes, 64));
    REPORTER_ASSERT(r, SkChecksum::Hash(bytes, 99) == 0x67433e3b, "%08x", SkChecksum::Hash(bytes, 99));
    REPORTER_ASSERT(r, SkChecksum::Hash(bytes,255) == 0x41df9dca, "%08x", SkChecksum::Hash(bytes,255));
}

DEF_TEST(ChecksumStrings, r) {
    constexpr char kMessage[] = "Checksums are supported for SkString, string, and string_view.";
    const uint32_t expectedHash = SkChecksum::Hash(kMessage, strlen(kMessage));

    REPORTER_ASSERT(r, expectedHash == SkGoodHash()(SkString(kMessage)));
    REPORTER_ASSERT(r, expectedHash == SkGoodHash()(std::string(kMessage)));
    REPORTER_ASSERT(r, expectedHash == SkGoodHash()(std::string_view(kMessage)));
}
