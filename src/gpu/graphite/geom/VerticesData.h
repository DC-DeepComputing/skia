/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_VerticesData_DEFINED
#define skgpu_graphite_geom_VerticesData_DEFINED

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

    VerticesData(sk_sp<SkVertices> vertices, bool skipColorXForm = false)
        : fVertices(vertices)
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

    // Access the individual elements of the subrun data.
    // const sktext::gpu::AtlasSubRun* subRun() const { return fSubRun; }
    // int startGlyphIndex() const { return fStartGlyphIndex; }
    // int glyphCount() const { return fGlyphCount; }
    // Recorder* recorder() const { return fRecorder; }

private:
    sk_sp<SkVertices> fVertices;
    const bool fSkipColorXForm;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_geom_VerticesData_DEFINED
