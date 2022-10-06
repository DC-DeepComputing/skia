/*
 * Copyright 2022 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMesh.h"

#include "fuzz/Fuzz.h"

template <typename T>
T extract(SkSpan<const uint8_t>& data) {
    T result = 0;
    size_t bytesToCopy = std::min(sizeof(T), data.size());
    memcpy(&result, &data.front(), bytesToCopy);
    data = data.subspan(bytesToCopy);
    return result;
}

static bool is_ascii(char c) {
    return c >= 32;
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
        switch (extract<uint8_t>(data) & 4) {
            case 0: {
                // Add an attribute; intentionally allow some bad values through.
                Attribute& a = attributes.push_back();
                a.type = (Attribute::Type)(extract<uint8_t>(data)% (int)Attribute::Type::kLast + 1);
                a.offset = extract<uint16_t>(data) % (SkMeshSpecification::kMaxStride + 2);
                while (char c = extract<char>(data)) {
                    if (!is_ascii(c)) {
                        break;
                    }
                    a.name.append(&c, /*len=*/1);
                }
                break;
            }
            case 1: {
                // Add a varying.
                Varying& v = varyings.push_back();
                v.type = (Varying::Type)(extract<uint8_t>(data) % (int)Varying::Type::kLast + 1);
                while (char c = extract<char>(data)) {
                    if (!is_ascii(c)) {
                        break;
                    }
                    v.name.append(&c, /*len=*/1);
                }
                break;
            }
            case 2: {
                // Append data to the vertex program until we reach a non-ASCII byte.
                while (char c = extract<char>(data)) {
                    if (!is_ascii(c)) {
                        break;
                    }
                    vs.append(&c, /*len=*/1);
                }
                break;
            }
            case 3: {
                // Append data to the fragment program until we reach a non-ASCII byte.
                while (char c = extract<char>(data)) {
                    if (!is_ascii(c)) {
                        break;
                    }
                    vs.append(&c, /*len=*/1);
                }
                break;
            }
        }
    }

    auto result = SkMeshSpecification::Make(attributes, vertexStride, varyings, vs, fs);
    if (result.error.isEmpty()) {
        SK_ABORT("!");
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
