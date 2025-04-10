/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkContext_DEFINED
#define SkContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class SK_API SkContext : public SkRefCnt {
public:
    enum class Type {
        kRaster,
        kGanesh,
        kGraphite,
    };

    virtual Type type() const = 0;
};

#endif
