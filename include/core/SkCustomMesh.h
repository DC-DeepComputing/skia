/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCustomMesh_DEFINED
#define SkCustomMesh_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"

#include <atomic>
#include <vector>

namespace SkSL { struct Program; }

class SkCustomMeshSpecification : public SkNVRefCnt<SkCustomMeshSpecification> {
public:
    struct Attribute {
        enum class Type {   // CPU representation     Shader Type
            kFloat4,        // four floats            float4
            kUByte4_unorm,  // four bytes             half4
        };
        Type     type;
        SkString name;
        size_t   offset;
    };

    struct Varying {
        enum class Type {
            kFloat,   // "float"
            kFloat2,  // "float2"
            kFloat3,  // "float3"
            kFloat4,  // "float4"
            kHalf,    // "half"
            kHalf2,   // "half2"
            kHalf3,   // "half3"
            kHalf4,   // "half4"
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
    friend struct SkCustomMeshDescPriv;
    SkCustomMeshSpecification(SkSpan<const Attribute>,
                              size_t stride,
                              SkSpan<const Varying>,
                              std::unique_ptr<SkSL::Program>,
                              std::unique_ptr<SkSL::Program>);
    SkCustomMeshSpecification(const SkCustomMeshSpecification&) = delete;
    SkCustomMeshSpecification(SkCustomMeshSpecification&&) = delete;

    SkCustomMeshSpecification& operator=(const SkCustomMeshSpecification&) = delete;
    SkCustomMeshSpecification& operator=(SkCustomMeshSpecification&&) = delete;

    const std::vector<const Attribute> fAttributes;
    const std::vector<const Varying>   fVaryings;
    std::unique_ptr<SkSL::Program>     fVS;
    std::unique_ptr<SkSL::Program>     fFS;
    size_t                             fStride;
    int                                fID;

    static std::atomic<int> gCurrID;
};

struct SkCustomMesh {
    enum class Mode { kTriangles, kTriangleStrip };
    sk_sp<SkCustomMeshSpecification>  spec;
    Mode                              mode;
    std::unique_ptr<char[]>           vb;
    int                               vcount;
};

#endif
