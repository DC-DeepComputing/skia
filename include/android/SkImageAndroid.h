/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageAndroid_DEFINED
#define SkImageAndroid_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"

class SkColorSpace;
class GrDirectContext;
class SkPixmap;
struct AHardwareBuffer;

namespace sk_image_factory {

/** (See Skia bug 7447)
    Creates SkImage from Android hardware buffer.
    Returned SkImage takes a reference on the buffer.
    Only available on Android, when __ANDROID_API__ is defined to be 26 or greater.
    @param hardwareBuffer  AHardwareBuffer Android hardware buffer
    @param colorSpace      range of colors; may be nullptr
    @return                created SkImage, or nullptr
*/
sk_sp<SkImage> MakeFromAHardwareBuffer(
        AHardwareBuffer* hardwareBuffer,
        SkAlphaType alphaType = kPremul_SkAlphaType);
sk_sp<SkImage> MakeFromAHardwareBuffer(
        AHardwareBuffer* hardwareBuffer,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        GrSurfaceOrigin surfaceOrigin = kTopLeft_GrSurfaceOrigin);

/** Creates SkImage from Android hardware buffer and uploads the data from the SkPixmap to it.
    Returned SkImage takes a reference on the buffer.
    Only available on Android, when __ANDROID_API__ is defined to be 26 or greater.
    @param context         GPU context
    @param pixmap          SkPixmap that contains data to be uploaded to the AHardwareBuffer
    @param hardwareBuffer  AHardwareBuffer Android hardware buffer
    @param surfaceOrigin   surface origin for resulting image
    @return                created SkImage, or nullptr
*/
sk_sp<SkImage> MakeFromAHardwareBufferWithData(
        GrDirectContext* context,
        const SkPixmap& pixmap,
        AHardwareBuffer* hardwareBuffer,
        GrSurfaceOrigin surfaceOrigin = kTopLeft_GrSurfaceOrigin);

}

#endif // SkImageAndroid_DEFINED
