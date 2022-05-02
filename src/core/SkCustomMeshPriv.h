/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCustomMeshPriv_DEFINED
#define SkCustomMeshPriv_DEFINED

#include "include/core/SkCustomMesh.h"

#ifdef SK_ENABLE_SKSL
#include "include/core/SkData.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSLTypeShared.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrClientBufferManager.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#endif

struct SkCustomMeshSpecificationPriv {
    using Varying   = SkCustomMeshSpecification::Varying;
    using Attribute = SkCustomMeshSpecification::Attribute;
    using ColorType = SkCustomMeshSpecification::ColorType;

    static SkSpan<const Varying> Varyings(const SkCustomMeshSpecification& spec) {
        return SkMakeSpan(spec.fVaryings);
    }

    static const SkSL::Program* VS(const SkCustomMeshSpecification& spec) { return spec.fVS.get(); }
    static const SkSL::Program* FS(const SkCustomMeshSpecification& spec) { return spec.fFS.get(); }

    static int Hash(const SkCustomMeshSpecification& spec) { return spec.fHash; }

    static ColorType GetColorType(const SkCustomMeshSpecification& spec) { return spec.fColorType; }
    static bool HasColors(const SkCustomMeshSpecification& spec) {
        return GetColorType(spec) != ColorType::kNone;
    }

    static SkColorSpace* ColorSpace(const SkCustomMeshSpecification& spec) {
        return spec.fColorSpace.get();
    }

    static SkAlphaType AlphaType(const SkCustomMeshSpecification& spec) { return spec.fAlphaType; }

    static bool HasLocalCoords(const SkCustomMeshSpecification& spec) {
        return spec.fHasLocalCoords;
    }

    static SkSLType VaryingTypeAsSLType(Varying::Type type) {
        switch (type) {
            case Varying::Type::kFloat:  return SkSLType::kFloat;
            case Varying::Type::kFloat2: return SkSLType::kFloat2;
            case Varying::Type::kFloat3: return SkSLType::kFloat3;
            case Varying::Type::kFloat4: return SkSLType::kFloat4;
            case Varying::Type::kHalf:   return SkSLType::kHalf;
            case Varying::Type::kHalf2:  return SkSLType::kHalf2;
            case Varying::Type::kHalf3:  return SkSLType::kHalf3;
            case Varying::Type::kHalf4:  return SkSLType::kHalf4;
        }
        SkUNREACHABLE;
    }

    static GrVertexAttribType AttrTypeAsVertexAttribType(Attribute::Type type) {
        switch (type) {
            case Attribute::Type::kFloat:        return kFloat_GrVertexAttribType;
            case Attribute::Type::kFloat2:       return kFloat2_GrVertexAttribType;
            case Attribute::Type::kFloat3:       return kFloat3_GrVertexAttribType;
            case Attribute::Type::kFloat4:       return kFloat4_GrVertexAttribType;
            case Attribute::Type::kUByte4_unorm: return kUByte4_norm_GrVertexAttribType;
        }
        SkUNREACHABLE;
    }

    static SkSLType AttrTypeAsSLType(Attribute::Type type) {
        switch (type) {
            case Attribute::Type::kFloat:        return SkSLType::kFloat;
            case Attribute::Type::kFloat2:       return SkSLType::kFloat2;
            case Attribute::Type::kFloat3:       return SkSLType::kFloat3;
            case Attribute::Type::kFloat4:       return SkSLType::kFloat4;
            case Attribute::Type::kUByte4_unorm: return SkSLType::kHalf4;
        }
        SkUNREACHABLE;
    }
};

struct SkCustomMeshPriv {
    class Buffer {
    public:
        virtual ~Buffer() = default;
        virtual sk_sp<const SkData> asData() const { return nullptr; }
        virtual sk_sp<const GrGpuBuffer> asGpuBuffer() const { return nullptr; }
        virtual size_t size() const = 0;
    };

    class IB : public Buffer, public SkCustomMesh::IndexBuffer {};
    class VB : public Buffer, public SkCustomMesh::VertexBuffer {};

    template <typename Base> class CpuBuffer : public Base {
    public:
        CpuBuffer() = default;
        ~CpuBuffer() override = default;

        static sk_sp<Base> Make(sk_sp<const SkData> data);

        sk_sp<const SkData> asData() const override { return fData; }

        size_t size() const override { return fData->size(); }

    private:
        sk_sp<const SkData> fData;
    };

    using CpuIndexBuffer  = CpuBuffer<IB>;
    using CpuVertexBuffer = CpuBuffer<VB>;

#if SK_SUPPORT_GPU
    template <typename Base> class GpuBuffer : public Base {
    public:
        GpuBuffer() = default;
        ~GpuBuffer() override;

        static sk_sp<Base> Make(sk_sp<GrGpuBuffer>, GrDirectContext::DirectContextID);

        sk_sp<const GrGpuBuffer> asGpuBuffer() const override { return fBuffer; }

        size_t size() const override { return fBuffer->size(); }

    private:
        sk_sp<GrGpuBuffer> fBuffer;
        GrDirectContext::DirectContextID fContextID;
    };

    using GpuIndexBuffer  = GpuBuffer<IB>;
    using GpuVertexBuffer = GpuBuffer<VB>;
#endif  // SK_SUPPORT_GPU
};

template <typename Base>
sk_sp<Base> SkCustomMeshPriv::CpuBuffer<Base>::Make(sk_sp<const SkData> data) {
    auto result = new CpuBuffer<Base>;
    result->fData = std::move(data);
    return sk_sp<Base>(result);
}
#if SK_SUPPORT_GPU

template <typename Base>
SkCustomMeshPriv::GpuBuffer<Base>::~GpuBuffer()  {
    GrClientBufferManager::BufferFinishedMessage message(std::move(fBuffer), fContextID);
}

template <typename Base>
sk_sp<Base> SkCustomMeshPriv::GpuBuffer<Base>::Make(
        sk_sp<GrGpuBuffer> buffer,
        GrDirectContext::DirectContextID contextID) {
    if (!buffer) {
        return nullptr;
    }
    auto result = new GpuBuffer<Base>;
    result->fBuffer    = std::move(buffer);
    result->fContextID = contextID;
    return sk_sp<Base>(result);
}

#endif  // SK_SUPPORT_GPU

#endif  // SK_ENABLE_SKSL

#endif
