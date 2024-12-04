/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVertState_DEFINED
#define SkVertState_DEFINED

#include "include/core/SkVertices.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"

#include <algorithm>
#include <cstdint>

/** \struct VertState
    This is a helper for drawVertices(). It is used to iterate over the triangles
    that are to be rendered based on an SkCanvas::VertexMode and (optionally) an
    index array. It does not copy the index array and the client must ensure it
    remains valid for the lifetime of the VertState object.
*/

struct VertState {
    int f0, f1, f2;

    /**
     *  Construct a VertState from a vertex count, index array, and index count.
     *  If the vertices are unindexed pass nullptr for indices.
     */
    VertState(int vCount, const uint16_t indices[], int indexCount)
            : fIndices(indices)
            , fCount(indices ? indexCount : vCount)
            , fVertCount(vCount)
            , fCurrIndex(0)
    {
        SkASSERT(vCount > 0);
    }

    typedef bool (*Proc)(VertState*);

    /**
     *  Choose an appropriate function to traverse the vertices.
     *  @param mode    Specifies the SkCanvas::VertexMode.
     */
    Proc chooseProc(SkVertices::VertexMode mode);

private:
    uint16_t getIndex(int i) const {
        SkASSERT(fIndices);
        SkASSERT(i < fCount);
        SkASSERT(fVertCount > 0);
        return std::min(fIndices[i], SkToU16(fVertCount - 1));
    }

    const uint16_t* fIndices;
    const int       fCount;
    const int       fVertCount;
    int             fCurrIndex;

    static bool Triangles(VertState*);
    static bool TrianglesX(VertState*);
    static bool TriangleStrip(VertState*);
    static bool TriangleStripX(VertState*);
    static bool TriangleFan(VertState*);
    static bool TriangleFanX(VertState*);
};

#endif
