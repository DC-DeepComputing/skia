/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "src/core/SkBitmapDevice.h"

class SkBlender;
class SkMesh;
class SkPaint;

void SkBitmapDevice::drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) {
    // TODO(brianosman): Implement
}
