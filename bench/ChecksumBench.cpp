/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkRandom.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkMD5.h"
#include "src/core/SkOpts.h"

extern uint32_t wyhash32(const void* key, uint64_t len, uint32_t seed);
extern uint32_t wyhash64(const void* key, size_t len, uint32_t seed);

enum ChecksumType {
    kMD5_ChecksumType,
    kHash_ChecksumType,
    kWyhash32_ChecksumType,
    kWyhash64_ChecksumType,
};

class ComputeChecksumBench : public Benchmark {
public:
    // All instances of this benchmark compute a checksum over 1MB (or slightly less, if the
    // block size doesn't divide evenly).
    constexpr static size_t kBufferSize = 1024 * 1024;

    ChecksumType fType;
    size_t fBlockSize;
    SkString fName;
    std::unique_ptr<uint8_t[]> fData;
    skia_private::TArray<size_t> fOffsets;

    ComputeChecksumBench(ChecksumType type, size_t blockSize) : fType(type), fBlockSize(blockSize) {
        SkASSERT(blockSize <= kBufferSize);

        switch (fType) {
            case kMD5_ChecksumType: fName = "compute_md5"; break;
            case kHash_ChecksumType: fName = "compute_hash"; break;
            case kWyhash32_ChecksumType: fName = "compute_wyhash32"; break;
            case kWyhash64_ChecksumType: fName = "compute_wyhash64"; break;
        }
        fName.appendf("_%d", static_cast<int>(fBlockSize));
    }

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override { return fName.c_str(); }

    void onPreDraw(SkCanvas*) override {
        fData.reset(new uint8_t[kBufferSize]);

        SkRandom rand;
        for (size_t i = 0; i < kBufferSize; ++i) {
            fData[i] = rand.nextBits(8);
        }

        const size_t blockCount = kBufferSize / fBlockSize;
        fOffsets.resize(blockCount);
        for (size_t i = 0; i < blockCount; ++i) {
            fOffsets[i] = i * fBlockSize;
        }
        for (size_t i = blockCount - 1; i > 0; --i) {
            size_t other = rand.nextULessThan(i + 1);
            std::swap(fOffsets[i], fOffsets[other]);
        }
    }

    void onPostDraw(SkCanvas*) override {
        fData.reset();
        fOffsets.clear();
    }

    void onDraw(int loops, SkCanvas*) override {
        volatile uint32_t result = 0;
        for (int i = 0; i < loops; i++) {
            for (size_t offset : fOffsets) {
                const void* buf = fData.get() + offset;
                switch (fType) {
                    case kMD5_ChecksumType: {
                        SkMD5 md5;
                        md5.write(buf, fBlockSize);
                        (void)md5.finish();
                        break;
                    }
                    case kHash_ChecksumType:
                        result = SkOpts::hash(buf, fBlockSize, 0);
                        break;
                    case kWyhash32_ChecksumType:
                        result = wyhash32(buf, fBlockSize, 0);
                        break;
                    case kWyhash64_ChecksumType:
                        result = wyhash64(buf, fBlockSize, 0);
                        break;
                }
            }
        }
        sk_ignore_unused_variable(result);
    }
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ComputeChecksumBench(kMD5_ChecksumType, 1024); )

#define DEF_CHECKSUM_BENCH(T) \
    DEF_BENCH( return new ComputeChecksumBench(T, 4); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 8); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 15); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 16); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 31); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 32); ) \
    DEF_BENCH( return new ComputeChecksumBench(T, 1024); )

DEF_CHECKSUM_BENCH(kHash_ChecksumType)
DEF_CHECKSUM_BENCH(kWyhash32_ChecksumType)
DEF_CHECKSUM_BENCH(kWyhash64_ChecksumType)
