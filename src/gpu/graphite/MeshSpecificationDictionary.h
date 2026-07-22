/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MeshSpecificationDictionary_DEFINED
#define skgpu_graphite_MeshSpecificationDictionary_DEFINED

#include "include/core/SkMesh.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkMeshPriv.h"
#include "src/core/SkTHash.h"

namespace skgpu::graphite {

class MeshSpecificationDictionary : public SkRefCnt {
public:
    const SkMeshSpecification* find(int codeSnippetID) const {
        sk_sp<const SkMeshSpecification>* spec = fDict.find(codeSnippetID);
        return spec ? spec->get() : nullptr;
    }

    void set(int codeSnippetID, sk_sp<const SkMeshSpecification> spec) {
        SkASSERT(!fDict.find(codeSnippetID) ||
                 SkMeshSpecificationPriv::Hash(**fDict.find(codeSnippetID))
                    == SkMeshSpecificationPriv::Hash(*spec));
        fDict.set(codeSnippetID, std::move(spec));
    }

    bool empty() const { return fDict.empty(); }

private:
    // Maps ShaderSnippet ids to their respective mesh specification.
    skia_private::THashMap<int32_t, sk_sp<const SkMeshSpecification>> fDict;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MeshSpecificationDictionary_DEFINED
