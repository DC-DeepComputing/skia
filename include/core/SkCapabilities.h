/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCapabilities_DEFINED
#define SkCapabilities_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu::graphite { class Caps; }

class SK_API SkCapabilities : public SkNVRefCnt<SkCapabilities> {
public:
    static sk_sp<SkCapabilities> RasterBackend();

    enum class SkSLVersion {
        /**
         * Desktop GLSL 1.10, GLSL ES 1.00, WebGL 1.0
         */
        k100ES,

        /**
         * Desktop GLSL 3.30, GLSL ES 3.00, WebGL 2.0
         */
        k300ES,
    };

    SkSLVersion skslVersion() const { return fSkSLVersion; }

private:
    SkCapabilities() = default;

    SkSLVersion fSkSLVersion = SkSLVersion::k100ES;

    friend class GrCaps;
    friend class skgpu::graphite::Caps;
};

#endif
