/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkMemset.h"

using namespace skia_private;

template <typename T>
class MemsetBench : public Benchmark {
public:
    explicit MemsetBench(size_t bytes)
        : fN(bytes / sizeof(T))
        , fBuffer(fN)
        , fName(SkStringPrintf("memset%zu_%zu", sizeof(T)*8, bytes)) {}

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override;

private:
    int fN;
    AutoTMalloc<T> fBuffer;
    SkString fName;
};

template <typename T>
class RectMemsetBench : public Benchmark {
public:
    RectMemsetBench(int w, int h)
        : fW(w), fH(h), fBuffer(w * h)
        , fName(SkStringPrintf("rect_memset%zu_%d_%d", sizeof(T)*8, w, h)) {}

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }
    const char* onGetName() override { return fName.c_str(); }

    void onDraw(int loops, SkCanvas*) override;

private:
    int fW, fH;
    AutoTMalloc<T> fBuffer;
    SkString fName;
};

template <> void MemsetBench<uint64_t>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        SkOpts::memset64(fBuffer.get(), 0xFACEFACEFACEFACE, fN);
    }
}

template <> void RectMemsetBench<uint64_t>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        SkOpts::rect_memset64(fBuffer.get(), 0xFACEFACEFACEFACE, fW, fW * sizeof(uint64_t), fH);
    }
}

template <> void MemsetBench<uint32_t>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        SkOpts::memset32(fBuffer.get(), 0xFACEB004, fN);
    }
}

template <> void RectMemsetBench<uint32_t>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        SkOpts::rect_memset32(fBuffer.get(), 0xFACEB004, fW, fW * sizeof(uint32_t), fH);
    }
}

template <> void MemsetBench<uint16_t>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        SkOpts::memset16(fBuffer.get(), 0x4973, fN);
    }
}

template <> void RectMemsetBench<uint16_t>::onDraw(int loops, SkCanvas*) {
    for (int i = 0; i < 1000*loops; i++) {
        SkOpts::rect_memset16(fBuffer.get(), 0x4973, fW, fW * sizeof(uint16_t), fH);
    }
}

DEF_BENCH(return (new MemsetBench<uint64_t>(16)))
DEF_BENCH(return (new MemsetBench<uint64_t>(64)))
DEF_BENCH(return (new MemsetBench<uint64_t>(256)))
DEF_BENCH(return (new MemsetBench<uint64_t>(512)))
DEF_BENCH(return (new MemsetBench<uint64_t>(768)))
DEF_BENCH(return (new MemsetBench<uint64_t>(1024)))
DEF_BENCH(return (new MemsetBench<uint64_t>(2048)))
DEF_BENCH(return (new MemsetBench<uint64_t>(4096)))
DEF_BENCH(return (new MemsetBench<uint64_t>(65536)))

DEF_BENCH(return (new MemsetBench<uint32_t>(16)))
DEF_BENCH(return (new MemsetBench<uint32_t>(64)))
DEF_BENCH(return (new MemsetBench<uint32_t>(256)))
DEF_BENCH(return (new MemsetBench<uint32_t>(512)))
DEF_BENCH(return (new MemsetBench<uint32_t>(768)))
DEF_BENCH(return (new MemsetBench<uint32_t>(1024)))
DEF_BENCH(return (new MemsetBench<uint32_t>(2048)))
DEF_BENCH(return (new MemsetBench<uint32_t>(4096)))
DEF_BENCH(return (new MemsetBench<uint32_t>(65536)))

DEF_BENCH(return (new MemsetBench<uint16_t>(16)))
DEF_BENCH(return (new MemsetBench<uint16_t>(64)))
DEF_BENCH(return (new MemsetBench<uint16_t>(256)))
DEF_BENCH(return (new MemsetBench<uint16_t>(512)))
DEF_BENCH(return (new MemsetBench<uint16_t>(768)))
DEF_BENCH(return (new MemsetBench<uint16_t>(1024)))
DEF_BENCH(return (new MemsetBench<uint16_t>(2048)))
DEF_BENCH(return (new MemsetBench<uint16_t>(4096)))
DEF_BENCH(return (new MemsetBench<uint16_t>(65536)))

// Real-world rectangle sizes
// 1000x1 and 1920x10: Wide and short rectangles, common for clipping and background fills.
// 30x30: Small UI icons.
// 100x100, 200x200: Typical UI elements/buttons.
// 256x256, 512x512: Tile-based rendering.

DEF_BENCH(return (new RectMemsetBench<uint16_t>(1000, 1)))
DEF_BENCH(return (new RectMemsetBench<uint16_t>(30, 30)))
DEF_BENCH(return (new RectMemsetBench<uint16_t>(100, 100)))
DEF_BENCH(return (new RectMemsetBench<uint16_t>(256, 256)))
DEF_BENCH(return (new RectMemsetBench<uint16_t>(512, 512)))
DEF_BENCH(return (new RectMemsetBench<uint16_t>(1920, 10)))

DEF_BENCH(return (new RectMemsetBench<uint32_t>(1000, 1)))
DEF_BENCH(return (new RectMemsetBench<uint32_t>(30, 30)))
DEF_BENCH(return (new RectMemsetBench<uint32_t>(100, 100)))
DEF_BENCH(return (new RectMemsetBench<uint32_t>(256, 256)))
DEF_BENCH(return (new RectMemsetBench<uint32_t>(512, 512)))
DEF_BENCH(return (new RectMemsetBench<uint32_t>(1920, 10)))

DEF_BENCH(return (new RectMemsetBench<uint64_t>(1000, 1)))
DEF_BENCH(return (new RectMemsetBench<uint64_t>(30, 30)))
DEF_BENCH(return (new RectMemsetBench<uint64_t>(100, 100)))
DEF_BENCH(return (new RectMemsetBench<uint64_t>(256, 256)))
DEF_BENCH(return (new RectMemsetBench<uint64_t>(512, 512)))
DEF_BENCH(return (new RectMemsetBench<uint64_t>(1920, 10)))
