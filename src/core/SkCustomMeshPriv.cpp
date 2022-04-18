/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCustomMeshPriv.h"

#ifdef SK_ENABLE_SKSL

#include "include/core/SkData.h"
#include "src/core/SkSafeMath.h"

static size_t min_vcount_for_mode(SkCustomMesh::Mode mode) {
    switch (mode) {
        case SkCustomMesh::Mode::kTriangles:     return 3;
        case SkCustomMesh::Mode::kTriangleStrip: return 3;
    }
    SkUNREACHABLE;
}

bool SkValidateCustomMesh(const SkCustomMesh& cm) {
    if (!cm.spec) {
        return false;
    }

    if (!cm.vb) {
        return false;
    }

    if (cm.vcount == 0) {
        return false;
    }

    auto vb = static_cast<SkCustomMeshPriv::AB*>(cm.vb.get());
    auto ib = static_cast<SkCustomMeshPriv::IB*>(cm.ib.get());

    SkSafeMath sm;
    size_t vsize = sm.mul(cm.spec->stride(), cm.vcount);
    if (sm.add(vsize, cm.voffset) > vb->size()) {
        return false;
    }

    if (!SkIsAlign4(cm.voffset)) {
        return false;
    }

    if (ib) {
        if (cm.icount < min_vcount_for_mode(cm.mode)) {
            return false;
        }
        size_t isize = sm.mul(sizeof(uint16_t), cm.icount);
        if (sm.add(isize, cm.ioffset) > ib->size()) {
            return false;
        }
        // If we allow 32 bit indices then this should enforce 4 byte alignment in that case.
        if (!SkIsAlign2(cm.ioffset)) {
            return false;
        }
    } else {
        if (cm.vcount < min_vcount_for_mode(cm.mode)) {
            return false;
        }
        if (cm.icount > 0 || cm.ioffset > 0) {
            return false;
        }
    }

    return sm.ok();
}

//////////////////////////////////////////////////////////////////////////////

SkCustomMeshPriv::CpuIndexBuffer::CpuIndexBuffer() = default;

SkCustomMeshPriv::CpuIndexBuffer::~CpuIndexBuffer() = default;

sk_sp<SkCustomMesh::IndexBuffer> SkCustomMeshPriv::CpuIndexBuffer::Make(sk_sp<const SkData> data) {
    auto result = new CpuIndexBuffer;
    result->fData = std::move(data);
    return sk_sp<IndexBuffer>(result);
}

sk_sp<const SkData> SkCustomMeshPriv::CpuIndexBuffer::asData() const { return fData; }

size_t SkCustomMeshPriv::CpuIndexBuffer::size() const { return fData->size(); }

//////////////////////////////////////////////////////////////////////////////

SkCustomMeshPriv::CpuAttributeBuffer::CpuAttributeBuffer() = default;

SkCustomMeshPriv::CpuAttributeBuffer::~CpuAttributeBuffer() = default;

sk_sp<SkCustomMesh::AttributeBuffer>
SkCustomMeshPriv::CpuAttributeBuffer::Make(sk_sp<const SkData> data) {
    auto result = new CpuAttributeBuffer;
    result->fData = std::move(data);
    return sk_sp<AttributeBuffer>(result);
}

sk_sp<const SkData> SkCustomMeshPriv::CpuAttributeBuffer::asData() const { return fData; }

size_t SkCustomMeshPriv::CpuAttributeBuffer::size() const { return fData->size(); }
#endif
