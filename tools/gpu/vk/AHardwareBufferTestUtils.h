/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_gpu_test_AHardwareBufferTestUtils_DEFINED
#define sk_gpu_test_AHardwareBufferTestUtils_DEFINED

// #if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26 && defined(SK_VULKAN)

// todo confirm all these are needed
// todo fwd declare what we can here
#include "include/android/SkImageAndroid.h"
#include "include/android/SkSurfaceAndroid.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"

#include <android/hardware_buffer.h>
#include <cinttypes>
namespace sk_gpu_test {

static constexpr int DEV_W = 16, DEV_H = 16;

SkPMColor GetSrcColor(int x, int y);

void CleanupResources(AHardwareBuffer*);

SkBitmap MakeSrcBitmap();

bool CheckRead(const SkBitmap& expectedBitmap, const SkBitmap& actualBitmap, skiatest::Reporter*);

void SetupAHardwareBuffer(AHardwareBuffer*, AHardwareBuffer_Desc*, skiatest::Reporter*);

void PopulateBitmap(const SkBitmap& srcBitmap, AHardwareBuffer*, uint32_t stride, skgpu::Origin,
                     skiatest::Reporter*);

void DrawImageAndReadBack(const SkBitmap& srcBitmap, SkSurface&, skiatest::Reporter*);

} // namespace sk_gpu_test

// #endif // defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26 && defined(SK_VULKAN)
#endif // sk_gpu_test_AHardwareBufferTestUtils_DEFINED
