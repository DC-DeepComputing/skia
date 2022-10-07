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

static void FuzzSkMeshSpecification(SkSpan<const uint8_t> data) {
    using Attribute = SkMeshSpecification::Attribute;
    using Varying = SkMeshSpecification::Varying;

    SkSTArray<SkMeshSpecification::kMaxAttributes, Attribute> attributes;
    SkSTArray<SkMeshSpecification::kMaxVaryings,   Varying>   varyings;
    size_t vertexStride;
    SkString vs, fs;

    auto fuzzByteToSkSL = [&](uint8_t c, SkString* str) -> bool {
        // In the 0x00 - 0x80 range, treat characters as ASCII.
        // Unsupported control characters (including \0) are all treated as stop bytes.
        if (c <= 127) {
            if (c >= 32 || c == '\r' || c == '\n' || c == '\t') {
                char ascii = c;
                str->append(&ascii, 1);
                return true;
            }
            return false;
        }
        c -= 128;

        // Dedicate a few bytes to injecting our attribute and varying names.
        if (c < SkMeshSpecification::kMaxAttributes) {
            str->append(attributes[c % attributes.size()].name);
            return true;
        }
        c -= SkMeshSpecification::kMaxAttributes;

        if (c < SkMeshSpecification::kMaxVaryings) {
            str->append(varyings[c % varyings.size()].name);
            return true;
        }
        c -= SkMeshSpecification::kMaxVaryings;

        // Replace the remaining high-ASCII bytes with valid SkSL operators and keywords in order to
        // improve our chances of generating a program. (We omit single-character operators since
        // single-byte versions of those already exist in the low-ASCII space.)
        static constexpr std::string_view kSkSLData[] = {
                " true ",
                " false ",
                " if ",
                " @if ",
                " else ",
                " for ",
                " while ",
                " do ",
                " switch ",
                " @switch ",
                " case ",
                " default ",
                " break ",
                " continue ",
                " discard ",
                " return ",
                " in ",
                " out ",
                " inout ",
                " uniform ",
                " const ",
                " flat ",
                " noperspective ",
                " inline ",
                " noinline ",
                " $pure ",
                " readonly ",
                " writeonly ",
                " buffer ",
                " struct ",
                " layout ",
                " highp ",
                " mediump ",
                " lowp ",
                " $es3 ",
                " $export ",
                " threadgroup ",
                " << ",
                " >> ",
                " && ",
                " || ",
                " ^^ ",
                " == ",
                " != ",
                " <= ",
                " >= ",
                " += ",
                " -= ",
                " *= ",
                " /= ",
                " %= ",
                " <<= ",
                " >>= ",
                " &= ",
                " |= ",
                " ^= ",
                " ++ ",
                " -- ",
                " //",
                " /*",
                "*/ ",
        };

        c %= std::size(kSkSLData);
        str->append(kSkSLData[c].data(), kSkSLData[c].size());
        return true;
    };

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
                while (uint8_t c = extract<char>(data)) {
                    if (!fuzzByteToSkSL(c, &a.name)) {
                        break;
                    }
                }
                break;
            }
            case 1: {
                // Add a varying.
                Varying& v = varyings.push_back();
                v.type = (Varying::Type)(extract<uint8_t>(data) % ((int)Varying::Type::kLast + 1));
                while (uint8_t c = extract<char>(data)) {
                    if (!fuzzByteToSkSL(c, &v.name)) {
                        break;
                    }
                }
                break;
            }
            case 2: {
                // Append data to the vertex program until we reach a non-ASCII byte.
                while (uint8_t c = extract<char>(data)) {
                    if (!fuzzByteToSkSL(c, &vs)) {
                        break;
                    }
                }
                break;
            }
            case 3: {
                // Append data to the fragment program until we reach a non-ASCII byte.
                while (uint8_t c = extract<char>(data)) {
                    if (!fuzzByteToSkSL(c, &vs)) {
                        break;
                    }
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
