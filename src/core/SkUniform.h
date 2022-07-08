/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUniform_DEFINED
#define SkUniform_DEFINED

#include "src/core/SkSLTypeShared.h"

#include <string_view>

// TODO: can SkRuntimeEffect::Uniform be absorbed into this class!?

/**
 * Describes a uniform. Uniforms consist of:
 *  type:       The type of the uniform
 *  count:      Number of elements of 'type' in the array or kNonArray if not an array.
 */
class SkUniform {
public:
    static constexpr int kNonArray = 0;

    constexpr SkUniform(std::string_view name, SkSLType type, int count = kNonArray)
            : fType      (static_cast<unsigned>(type))
            , fCount     (static_cast<unsigned>(count))
            , fNameLength(name.size())
            , fName      (name.data()) {}

    constexpr SkUniform(const SkUniform&) = default;
    SkUniform& operator=(const SkUniform&) = default;

    constexpr bool isInitialized() const { return this->type() != SkSLType::kVoid; }

    constexpr std::string_view name() const  { return std::string_view(fName, fNameLength); }
    constexpr SkSLType         type() const  { return static_cast<SkSLType>(fType);  }
    constexpr uint32_t         count() const { return static_cast<uint32_t>(fCount); }

private:
    uint32_t    fType    : 6;
    uint32_t    fCount   : 26;
    int32_t     fNameLength;  // like a std::string_view, but without wasting bytes to padding
    const char* fName;

    static_assert(kSkSLTypeCount <= (1 << 6));
};

#endif // SkUniform_DEFINED
