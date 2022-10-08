/*
 * Copyright 2022 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMesh.h"

#include "fuzz/Fuzz.h"
#include "fuzz/oss_fuzz/FuzzSkSLShared.h"

template <typename T>
T extract(SkSpan<const uint8_t>& data) {
    T result = 0;
    size_t bytesToCopy = std::min(sizeof(T), data.size());
    memcpy(&result, &data.front(), bytesToCopy);
    data = data.subspan(bytesToCopy);
    return result;
}

static void FuzzSkMeshSpecification(SkSpan<const uint8_t> data) {
    using Attribute = SkMeshSpecification::Attribute;
    using Varying = SkMeshSpecification::Varying;

    SkSTArray<SkMeshSpecification::kMaxAttributes, Attribute> attributes;
    SkSTArray<SkMeshSpecification::kMaxVaryings,   Varying>   varyings;
    size_t vertexStride;
    SkString vs, fs;

    // Pick a vertex stride; intentionally allow some bad values through.
    vertexStride = extract<uint16_t>(data) % (SkMeshSpecification::kMaxStride + 2);

    while (!data.empty()) {
        uint8_t control = extract<uint8_t>(data) % 4;
        switch (control) {
            case 0: {
                // Add an attribute.
                Attribute& a = attributes.push_back();
                a.type = (Attribute::Type)(extract<uint8_t>(data) %
                                           ((int)Attribute::Type::kLast + 1));
                a.offset = extract<uint16_t>(data) % (SkMeshSpecification::kMaxStride + 2);
                TransformInputToSkSL(&data,
                                     &a.name,
                                     ControlCharacterBehavior::kStop,
                                     HighBitBehavior::kStop);
                break;
            }
            case 1: {
                // Add a varying.
                Varying& v = varyings.push_back();
                v.type = (Varying::Type)(extract<uint8_t>(data) % ((int)Varying::Type::kLast + 1));
                TransformInputToSkSL(&data,
                                     &v.name,
                                     ControlCharacterBehavior::kStop,
                                     HighBitBehavior::kStop);
                break;
            }
            case 2: {
                // Convert the following data into SkSL and add it into the vertex program.
                TransformInputToSkSL(&data,
                                     &vs,
                                     ControlCharacterBehavior::kStop,
                                     HighBitBehavior::kReplaceWithToken);
                break;
            }
            case 3: {
                // Convert the following data into SkSL and add it into the fragment program.
                TransformInputToSkSL(&data,
                                     &fs,
                                     ControlCharacterBehavior::kStop,
                                     HighBitBehavior::kReplaceWithToken);
                break;
            }
        }
    }

    auto result = SkMeshSpecification::Make(attributes, vertexStride, varyings, vs, fs);
    if (result.error.isEmpty()) {
        // TODO: synthesize a mesh with this specification and paint it.
        printf("----\n%s\n----\n\n----\n%s\n----\n\n\n", vs.c_str(), fs.c_str());
    }
}

bool FuzzSkMeshSpecification(sk_sp<SkData> fuzz) {
    FuzzSkMeshSpecification(SkSpan(fuzz->bytes(), fuzz->size()));
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 8000) {
        return 0;
    }
    FuzzSkMeshSpecification(SkSpan<const uint8_t>(data, size));
    return 0;
}
#endif
