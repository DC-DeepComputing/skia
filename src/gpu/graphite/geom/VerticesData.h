/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_VerticesData_DEFINED
#define skgpu_graphite_geom_VerticesData_DEFINED

#include "include/core/SkBlender.h"
#include "include/core/SkVertices.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/graphite/geom/Rect.h"

namespace skgpu::graphite {

/**
 * VerticesData is a wrapper around SkVertices that contains all the data we need to render Vertices
 */
class VerticesData {
public:
    VerticesData() = delete;
    VerticesData(const VerticesData& vertices) = default;
    VerticesData(VerticesData&&) = delete;

    VerticesData(sk_sp<SkVertices> vertices, sk_sp<SkBlender> blender, bool skipColorXForm = false)
        : fVertices(vertices)
        , fBlender(blender)
        , fSkipColorXForm(skipColorXForm) {}

    ~VerticesData() = default;

    // NOTE: None of the geometry types benefit from move semantics, so we don't bother
    // defining a move assignment operator for VerticesData.
    VerticesData& operator=(VerticesData&&) = delete;
    VerticesData& operator=(const VerticesData& that) {
        fVertices = that.fVertices;
        return *this;
    }

    // The bounding box of the vertices data.
    Rect bounds() const { return fVertices->bounds(); }
    void reset() { fVertices.~sk_sp<SkVertices>(); }
    const SkVertices* vertices() const { return fVertices.get(); }
    bool skipColorXForm() const { return fSkipColorXForm; }
    sk_sp<SkBlender> refBlender() const { return fBlender; }

    // TODO - Create public methods to access SkVerticesPriv information?
    // Or call VerticesData.vertices()->priv().[privateInfoGetter]() ?

private:
    sk_sp<SkVertices> fVertices;
    sk_sp<SkBlender> fBlender;
    const bool fSkipColorXForm;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_VerticesData_DEFINED
