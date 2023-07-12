/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpecialImageGanesh_DEFINED
#define SkSpecialImageGanesh_DEFINED

#include "src/core/SkSpecialImage.h"


namespace SkSpecialImages {

sk_sp<SkSpecialImage> MakeFromTextureImage(GrRecordingContext* rContext,
                                           const SkIRect& subset,
                                           sk_sp<SkImage> image,
                                           const SkSurfaceProps& props);

sk_sp<SkSpecialImage> MakeDeferredFromGpu(GrRecordingContext*,
                                          const SkIRect& subset,
                                          uint32_t uniqueID,
                                          GrSurfaceProxyView,
                                          const GrColorInfo&,
                                          const SkSurfaceProps&);

}

#endif