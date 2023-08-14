/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AHBUtils_DEFINED
#define AHBUtils_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"

#if defined(SK_GANESH) && defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

struct AHardwareBuffer;

class GrDirectContext;
class SkBitmap;
class SkImage;
class SkSurface;

namespace AHBUtils {

// This entry point simply creates an AHB w/ 'srcBitmap' as its contents. It writes the
// AHB from the CPU so cannot be protected content.
// The caller must explicitly call AHardwareBuffer_release on the returned backing AHB if
// it is non-null.
AHardwareBuffer* CreateAHB(const SkBitmap& srcBitmap, GrSurfaceOrigin surfaceOrigin);

// This entry point can be used to create an AHB-backed surface that contains protected content.
// As such the backing AHB cannot be read from or written to by the CPU.
// The caller must explicitly call AHardwareBuffer_release on the returned backing AHB if the
// returned surface is non-null.
sk_sp<SkSurface> CreateAHBBackedSurface(GrDirectContext* dContext,
                                        int width, int height,
                                        GrSurfaceOrigin surfaceOrigin,
                                        bool isProtected,
                                        AHardwareBuffer** backingAHB);

// This entry point can be used to create an AHB-backed image w/ protected content. As such
// the backing AHB cannot be read from or written to by the CPU.
// The caller must explicitly call AHardwareBuffer_release on the returned backing AHB if the
// return image is non-null.
sk_sp<SkImage> CreateAHBBackedImage(GrDirectContext* dContext,
                                    const SkBitmap& srcBitmap,
                                    GrSurfaceOrigin surfaceOrigin,
                                    bool isProtected,
                                    AHardwareBuffer** backingAHB);

} // namespace AHBUtils

#endif

#endif  // AHBUtils_DEFINED
