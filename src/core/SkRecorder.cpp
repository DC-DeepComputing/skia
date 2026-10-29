/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRecorder.h"

#include "include/core/SkImage.h"
#include "include/cpu/Recorder.h"
#include "src/capture/SkCaptureAsset.h"

#include <utility>

SkRecorder* SkRecorder::TODO() {
    return skcpu::Recorder::TODO();
}

SkRecorder::SkRecorder() = default;
SkRecorder::~SkRecorder() = default;

sk_sp<SkImage> SkRecorder::makeImage(sk_sp<SkImage> image) { return image; }
