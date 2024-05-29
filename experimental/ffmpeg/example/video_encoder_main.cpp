/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "experimental/ffmpeg/SkVideoEncoder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkPaint.h"
#include "include/core/SkStream.h"

#include <cstdio>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <name.pdf>", argv[0]);
        return 1;
    }
    SkFILEWStream output(argv[1]);
    if (!output.isValid()) {
        printf("Cannot open output file %s\n", argv[1]);
        return 1;
    }

    SkVideoEncoder encoder;
    if (!encoder.beginRecording({400, 300}, 10)) {
        printf("Cannot begin recording\n");
        return 1;
    }

    for (int i = 0; i < 30; i++) {
        SkCanvas canvas = encoder.beginFrame();
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SK_ColorBLUE);

        canvas.drawCircle(200, 150, 50 + 2*i, paint);
        if (!encoder.endFrame()) {
            printf("Cannot end frame %d\n", i);
            return 1;
        }
    }

    sk_sp<SkData> data = encoder.endRecording();
    if (!data) {
        printf("Cannot produce data\n");
        return 1;
    }

    if (!output.write(data->data(), data->size())) {
        printf("Cannot write data to %s\n", argv[1]);
        return 1;
    }
    printf("Data written");
    return 0;
}
