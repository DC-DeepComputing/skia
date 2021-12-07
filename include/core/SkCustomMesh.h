/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCustomMesh_DEFINED
#define SkCustomMesh_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"

#include <atomic>
#include <vector>

namespace SkSL { struct Program; }

class SkCustomMeshSpecification : public SkNVRefCnt<SkCustomMeshSpecification> {
public:
    struct Attribute {
        enum class Type : uint32_t {  // CPU representation     Shader Type
            kFloat,                   // float                  float
            kFloat2,                  // two floats             float2
            kFloat3,                  // three floats           float3
            kFloat4,                  // four floats            float4
            kUByte4_unorm,            // four bytes             half4

            kLast = kUByte4_unorm
        };
        Type     type;
        size_t   offset;
        SkString name;
    };

    struct Varying {
        enum class Type : uint32_t {
            kFloat,   // "float"
            kFloat2,  // "float2"
            kFloat3,  // "float3"
            kFloat4,  // "float4"
            kHalf,    // "half"
            kHalf2,   // "half2"
            kHalf3,   // "half3"
            kHalf4,   // "half4"

            kLast = kHalf4
        };
        Type     type;
        SkString name;
    };

    ~SkCustomMeshSpecification();

    struct Result {
        sk_sp<SkCustomMeshSpecification> specification;
        SkString                         error;
    };
    static Result Make(SkSpan<const Attribute> attributes,
                       size_t                  stride,
                       SkSpan<const Varying>   varyings,
                       const SkString&         vs,
                       const SkString&         fs);

    const SkSpan<const Attribute> attributes() const { return SkMakeSpan(fAttributes); }

    size_t stride() const { return fStride; }

private:
    friend struct SkCustomMeshSpecificationPriv;

    static Result MakeFromSourceWithStructs(SkSpan<const Attribute> attributes,
                                            size_t                  stride,
                                            SkSpan<const Varying>   varyings,
                                            const SkString&         vs,
                                            const SkString&         fs);

    SkCustomMeshSpecification(SkSpan<const Attribute>,
                              size_t,
                              SkSpan<const Varying>,
                              std::unique_ptr<SkSL::Program>,
                              std::unique_ptr<SkSL::Program>);

    SkCustomMeshSpecification(const SkCustomMeshSpecification&) = delete;
    SkCustomMeshSpecification(SkCustomMeshSpecification&&) = delete;

    SkCustomMeshSpecification& operator=(const SkCustomMeshSpecification&) = delete;
    SkCustomMeshSpecification& operator=(SkCustomMeshSpecification&&) = delete;

    const std::vector<Attribute>       fAttributes;
    const std::vector<Varying>         fVaryings;
    std::unique_ptr<SkSL::Program>     fVS;
    std::unique_ptr<SkSL::Program>     fFS;
    size_t                             fStride;
    uint32_t                           fHash;
    bool                               fHasColors;
};

struct SkCustomMesh {
    enum class Mode { kTriangles, kTriangleStrip };
    sk_sp<SkCustomMeshSpecification>  spec;
    Mode                              mode    = Mode::kTriangles;
    SkRect                            bounds  = SkRect::MakeEmpty();
    const void*                       vb      = nullptr;
    int                               vcount  = 0;
};

#endif
