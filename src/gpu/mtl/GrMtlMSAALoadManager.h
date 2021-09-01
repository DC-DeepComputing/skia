/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlMSAALoadManager_DEFINED
#define GrMtlMSAALoadManager_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#import <Metal/Metal.h>

GR_NORETAIN_BEGIN

class GrAttachment;
class GrBuffer;
class GrMtlAttachment;
class GrMtlGpu;
struct SkIRect;

class GrMtlMSAALoadManager {
public:
    GrMtlMSAALoadManager();

    ~GrMtlMSAALoadManager();

    bool loadMSAAFromResolve(GrMtlGpu* gpu,
                             GrAttachment* dst,
                             GrMtlAttachment* src,
                             const SkIRect& srcRect);
};

GR_NORETAIN_END

#endif
