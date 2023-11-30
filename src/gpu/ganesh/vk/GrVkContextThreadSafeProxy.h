/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkContextThreadSafeProxy_DEFINED
#define GrVkContextThreadSafeProxy_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrTypes.h"

class GrCaps;
struct GrContextOptions;

class GrVkContextThreadSafeProxy : public GrContextThreadSafeProxy {
public:
    GrVkContextThreadSafeProxy(const GrContextOptions&);

    bool isValidCharacterizationForVulkan(sk_sp<const GrCaps>, GrProtected) override;
};

#endif
