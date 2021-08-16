/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AAHairLinePathRenderer_DEFINED
#define AAHairLinePathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"

namespace skgpu::v1 {

class AAHairLinePathRenderer : public GrPathRenderer {
public:
    AAHairLinePathRenderer() = default;

    const char* name() const final { return "AAHairline"; }

    typedef SkTArray<SkPoint, true> PtArray;
    typedef SkTArray<int, true> IntArray;
    typedef SkTArray<float, true> FloatArray;

private:
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const final;

    bool onDrawPath(const DrawPathArgs&) final;

    using INHERITED = GrPathRenderer;
};

} // namespace skgpu::v1

#endif // AAHairLinePathRenderer_DEFINED
