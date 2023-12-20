/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Uniform_DEFINED
#define skgpu_graphite_Uniform_DEFINED

#include "src/core/SkSLTypeShared.h"

namespace skgpu::graphite {

// TODO: can SkRuntimeEffect::Uniform be absorbed into this class!?
// TODO: should we have a subset of SkSLType that represents SkSL types that can be
//       used as uniforms or ssbo data?

/**
 * Describes a uniform. Uniforms consist of:
 *  name:       The constant string name to use in the generated SkSL
 *  type:       The type of the uniform
 *  count:      Number of elements of 'type' in the array or kNonArray if not an array.
 */
class Uniform {
public:
    static constexpr int kNonArray = 0;

    /*
     * The paint color uniform is treated special and will only be added to the uniform block
     * once. Its name will not be mangled.
     */
    enum class IsPaintColor : bool {
        kNo = false,
        kYes = true,
    };

    constexpr Uniform(const char* name, SkSLType type, int count = kNonArray,
                      IsPaintColor isPaintColor = IsPaintColor::kNo)
            : fName(name)
            , fType(static_cast<unsigned>(type))
            , fIsPaintColor(static_cast<unsigned>(isPaintColor == IsPaintColor::kYes))
            , fCount(static_cast<unsigned>(count)) {
        SkASSERT(SkSLTypeCanBeUniformValue(type));
        SkASSERT(count >= 0);
    }

    constexpr Uniform(const Uniform&) = default;
    Uniform& operator=(const Uniform&) = default;

    // constexpr bool isInitialized() const { return this->type() != SkSLType::kVoid; }

    constexpr const char* name()  const { return fName; }
    constexpr SkSLType    type()  const { return static_cast<SkSLType>(fType);  }
    constexpr int         count() const { return static_cast<int>(fCount); }

    constexpr bool isPaintColor() const { return static_cast<bool>(fIsPaintColor); }

private:
    const char* fName;

    uint32_t    fType         : 6;
    uint32_t    fIsPaintColor : 1;
    uint32_t    fCount        : 25;

    static_assert(kSkSLTypeCount <= (1 << 6));
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Uniform_DEFINED
