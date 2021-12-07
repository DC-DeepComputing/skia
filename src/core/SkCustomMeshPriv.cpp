/*
* Copyright 2021 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/core/SkCustomMeshPriv.h"

#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/sksl/ir/SkSLProgram.h"

void SkCustomMeshSpecificationPriv::Encode(const SkCustomMeshSpecification& spec,
                                           SkWriteBuffer& buffer) {
    buffer.writeString(spec.fVS->fSource->c_str());
    buffer.writeString(spec.fFS->fSource->c_str());
    buffer.writeUInt(SkToU32(spec.stride()));
    buffer.writeUInt(SkToU32(spec.attributes().size()));
    buffer.writeUInt(SkToU32(spec.fVaryings.size()));
    for (const auto& a : spec.attributes()) {
        buffer.writeUInt(SkToU32(a.type));
        buffer.writeUInt(SkToU32(a.offset));
        buffer.writeString(a.name.c_str());
    }
    for (const auto& v : spec.fVaryings) {
        buffer.writeUInt(SkToU32(v.type));
        buffer.writeString(v.name.c_str());
    }
}

sk_sp<SkCustomMeshSpecification> SkCustomMeshSpecificationPriv::Decode(SkReadBuffer& buffer) {
    SkString vs, fs;
    buffer.readString(&vs);
    buffer.readString(&fs);
    size_t stride = buffer.readUInt();
    size_t acount = buffer.readUInt();
    size_t vcount = buffer.readUInt();

    std::vector<Attribute> attrs;
    attrs.reserve(acount);
    for (size_t i = 0; i < acount; ++i) {
        Attribute a;
        a.type   = static_cast<Attribute::Type>(buffer.readUInt());
        a.offset = buffer.readUInt();
        buffer.readString(&a.name);
        if (!buffer.validate(a.type <= Attribute::Type::kLast)) {
            return nullptr;
        }
        attrs.push_back(std::move(a));
    }

    std::vector<Varying> varyings;
    varyings.reserve(acount);
    for (size_t i = 0; i < vcount; ++i) {
        Varying v;
        v.type   = static_cast<Varying::Type>(buffer.readUInt());
        buffer.readString(&v.name);
        if (!buffer.validate(v.type <= Varying::Type::kLast)) {
            return nullptr;
        }
        varyings.push_back(std::move(v));
    }

    // The strings we serialized already have the attr/varying structs prepended.
    auto [spec, error] = SkCustomMeshSpecification::MakeFromSourceWithStructs(SkMakeSpan(attrs),
                                                                              stride,
                                                                              SkMakeSpan(varyings),
                                                                              vs,
                                                                              fs);
    buffer.validate(spec != nullptr);

    return spec;
}

std::unique_ptr<const char[]> SkCopyCustomMeshVB(const SkCustomMesh& cm) {
    SkASSERT(cm.spec);
    size_t size = cm.spec->stride()*cm.vcount;

    auto vb = std::make_unique<char[]>(size);
    std::memcpy(vb.get(), cm.vb, size);

    return std::move(vb);
}

sk_sp<SkOwnedCustomMesh> SkOwnedCustomMesh::Make(SkCustomMesh cm) {
    if (cm.vcount <= 0 || !cm.spec || cm.bounds.isEmpty()) {
        return nullptr;
    }

    std::unique_ptr<const char[]> vb = SkCopyCustomMeshVB(cm);
    sk_sp<SkOwnedCustomMesh> result(new SkOwnedCustomMesh);
    result->fSpec         = std::move(cm.spec);
    result->fMode         = cm.mode;
    result->fBounds       = cm.bounds;
    result->fVertexBuffer = std::move(vb);
    result->fVCount       = cm.vcount;
    return result;
}

void SkOwnedCustomMesh::encode(SkWriteBuffer& buffer) const {
    SkCustomMeshSpecificationPriv::Encode(*fSpec, buffer);
    buffer.writeUInt(static_cast<uint32_t>(fMode));
    buffer.writeRect(fBounds);
    buffer.writeUInt(fVCount);
    buffer.writeByteArray(fVertexBuffer.get(), fVCount * fSpec->stride());
}

sk_sp<SkOwnedCustomMesh> SkOwnedCustomMesh::Decode(SkReadBuffer& buffer) {
    sk_sp<SkCustomMeshSpecification> spec = SkCustomMeshSpecificationPriv::Decode(buffer);
    if (!spec) {
        return nullptr;
    }
    SkCustomMesh::Mode mode = static_cast<SkCustomMesh::Mode>(buffer.readInt());
    SkRect bounds = buffer.readRect();
    int vcount = buffer.readInt();
    if (!buffer.validate(vcount > 0)) {
        return nullptr;
    }
    size_t size = vcount*spec->stride();
    auto vb = std::make_unique<char[]>(size);
    if (!buffer.readByteArray(vb.get(), size)) {
        return nullptr;
    }

    sk_sp<SkOwnedCustomMesh> result(new SkOwnedCustomMesh);
    result->fSpec         = std::move(spec);
    result->fMode         = mode;
    result->fBounds       = bounds;
    result->fVertexBuffer = std::move(vb);
    result->fVCount       = vcount;
    return result;
}
