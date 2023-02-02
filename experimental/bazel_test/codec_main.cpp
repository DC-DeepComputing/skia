/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"

#include <cstdio>

int main(int argc, char** argv) {
    constexpr int WIDTH = 255;
    constexpr int HEIGHT = 255;
    constexpr int BYTES_PER_PIXEL = 4;
    sk_sp<SkData> pixels = SkData::MakeZeroInitialized(WIDTH * HEIGHT * BYTES_PER_PIXEL);

    auto data = static_cast<uint8_t*>(pixels.writable_data());
    for (int r = 0; r < HEIGHT; r++) {
        for (int c = 0; c < WIDTH; c++) {
            // Red
            data[r * WIDTH + c * BYTES_PER_PIXEL + 0] = 128;
            // Green
            data[r * WIDTH + c * BYTES_PER_PIXEL + 1] = r;
            // Blue
            data[r * WIDTH + c * BYTES_PER_PIXEL + 2] = c;
            // Alpha
            data[r * WIDTH + c * BYTES_PER_PIXEL + 3] = 240;
        }
    }

    SkImageInfo ii(kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB())
    SkPixmap pMap(ii, pixels, BYTES_PER_PIXEL * WIDTH);

    SkFILEWStream output("output.png");
    if (!output.isValid()) {
        printf("Cannot open file\n");
        return 1;
    }

    SkPngEncoder::Options opts;
    opts.fZlibLevel = 9;
    if (SkPngEncoder::Encode(&output, pMap, opts)) {
        output.flush();
        output.fsync();
        printf("Success\n");
        return 0;
    }
    printf("Failed to encode\n");
    return 1;
}
