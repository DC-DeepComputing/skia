/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockAttachment_DEFINED
#define GrMockAttachment_DEFINED

#include "include/core/SkSize.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/mock/GrMockGpu.h"

#include <string_view>

class GrMockAttachment : public GrAttachment {
public:
    GrMockAttachment(GrMockGpu* gpu,
                     SkISize dimensions,
                     UsageFlags supportedUsages,
                     int sampleCnt,
                     GrProtected isProtected,
                     std::string_view label)
            : GrAttachment(gpu,
                           dimensions,
                           supportedUsages,
                           sampleCnt,
                           skgpu::Mipmapped::kNo,
                           isProtected,
                           label) {
        SkASSERT(supportedUsages == UsageFlags::kStencilAttachment);
        this->registerWithCache(skgpu::Budgeted::kYes);
    }

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMock(GrColorType::kUnknown, SkTextureCompressionType::kNone,
                                         /*isStencilFormat*/ true);
    }

private:
};

#endif
