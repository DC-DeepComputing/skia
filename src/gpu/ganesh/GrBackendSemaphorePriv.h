/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSemaphorePriv_DEFINED
#define GrBackendSemaphorePriv_DEFINED

#include "include/gpu/GrBackendSemaphore.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

class GrBackendSemaphoreData {
public:
    virtual ~GrBackendSemaphoreData();

#if defined(SK_DEBUG)
    virtual GrBackendApi type() const = 0;
#endif
protected:
    GrBackendSemaphoreData() = default;
private:
    friend class GrBackendSemaphore;
    virtual GrBackendSemaphoreData* copy() const = 0;
};

#endif