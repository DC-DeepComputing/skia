/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVertices_DEFINED
#define SkVertices_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class SkVerticesPriv;

/**
 * An immutable set of vertex data that can be used with SkCanvas::drawVertices.
 */
class SK_API SkVertices : public SkNVRefCnt<SkVertices> {
    struct Desc;
    struct Sizes;
public:
    enum VertexMode {
        kTriangles_VertexMode,
        kTriangleStrip_VertexMode,
        kTriangleFan_VertexMode,

        kLast_VertexMode = kTriangleFan_VertexMode,
    };

    /**
     *  Create a vertices by copying the specified arrays. texs, colors may be empty,
     *  and indices is ignored if it is empty.
     */
    static sk_sp<SkVertices> MakeCopy(VertexMode mode,
                                      SkSpan<const SkPoint>  positions,
                                      SkSpan<const SkPoint>  texs,
                                      SkSpan<const SkColor>  colors,
                                      SkSpan<const uint16_t> indices);

    static sk_sp<SkVertices> MakeCopyx(VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkPoint texs[],
                                      const SkColor colors[],
                                      int indexCount,
                                      const uint16_t indices[]) {
        return MakeCopy(mode,
                        {positions, vertexCount},
                        {texs,      vertexCount},
                        {colors,    vertexCount},
                        {indices,   indexCount});
    }

    static sk_sp<SkVertices> MakeCopyx(VertexMode mode, int vertexCount,
                                      const SkPoint positions[],
                                      const SkPoint texs[],
                                      const SkColor colors[]) {
        return MakeCopy(mode,
                        {positions, vertexCount},
                        {texs,      vertexCount},
                        {colors,    vertexCount},
                        {});
    }

    enum BuilderFlags {
        kHasTexCoords_BuilderFlag   = 1 << 0,
        kHasColors_BuilderFlag      = 1 << 1,
    };
    class SK_API Builder {
    public:
        Builder(VertexMode mode, size_t vertexCount, size_t indexCount, uint32_t flags);

        bool isValid() const { return fVertices != nullptr; }

        SkSpan<SkPoint> positionSpan();
        SkSpan<SkPoint> texSpan();
        SkSpan<SkColor> colorSpan();
        SkSpan<uint16_t> indexSpan();

        // deprected: use span versions
        SkPoint* positionsx();
        uint16_t* indicesx();        // returns null if there are no indices
        // If we have custom attributes, these will always be null
        SkPoint* texCoordsx();       // returns null if there are no texCoords
        SkColor* colorsx();          // returns null if there are no colors

        // Detach the built vertices object. After the first call, this will always return null.
        sk_sp<SkVertices> detach();

    private:
        Builder(const Desc&);

        void init(const Desc&);

        // holds a partially complete object. only completed in detach()
        sk_sp<SkVertices> fVertices;
        // Extra storage for intermediate vertices in the case where the client specifies indexed
        // triangle fans. These get converted to indexed triangles when the Builder is finalized.
        std::unique_ptr<uint8_t[]> fIntermediateFanIndices;

        friend class SkVertices;
        friend class SkVerticesPriv;
    };

    uint32_t uniqueID() const { return fUniqueID; }
    const SkRect& bounds() const { return fBounds; }

    // returns approximate byte size of the vertices object
    size_t approximateSize() const;

    // Provides access to functions that aren't part of the public API.
    SkVerticesPriv priv();
    const SkVerticesPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    SkVertices() {}

    friend class SkVerticesPriv;

    // these are needed since we've manually sized our allocation (see Builder::init)
    friend class SkNVRefCnt<SkVertices>;
    void operator delete(void* p);

    Sizes getSizes() const;

    // we store this first, to pair with the refcnt in our base-class, so we don't have an
    // unnecessary pad between it and the (possibly 8-byte aligned) ptrs.
    uint32_t fUniqueID;

    // these point inside our allocation, so none of these can be "freed"
    SkPoint*     fPositions;        // [vertexCount]
    uint16_t*    fIndices;          // [indexCount] or null
    SkPoint*     fTexs;             // [vertexCount] or null
    SkColor*     fColors;           // [vertexCount] or null

    SkRect  fBounds;    // computed to be the union of the fPositions[]
    size_t  fVertexCount;
    size_t  fIndexCount;

    VertexMode fMode;
    // below here is where the actual array data is stored.
};

#endif
