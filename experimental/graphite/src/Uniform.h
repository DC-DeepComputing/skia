/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Uniform_DEFINED
#define skgpu_Uniform_DEFINED

#include "include/private/GrTypesPriv.h"

namespace skgpu {

/**
 * Describes a uniform. Uniforms consist of:
 *  type:       The type of the values in the shader
 *  count:      Number of elements of 'type' in the array or GrShaderVar::kNonArray if not an
 *              array.
 *  offset:     byte offset of the data within the GrProcessor class (no relation to uniform
 *              buffer offset).
 *  ctype:      specifies the way the data at the 'offset' is represented. See CType enum
 *              comments.
 *  visibility: specifies in which shader stage(s) the uniform is declared.
 */
class Uniform {
public:
    static constexpr int kNonArray = 0;

#if 0
    enum class CType : unsigned {
        // Any float/half, vector of floats/half, or matrices of floats/halfs are a tightly
        // packed array of floats. Similarly, any bool/shorts/ints are a tightly packed array
        // of int32_t.
        kDefault,
        // Can be used with kFloat3x3 or kHalf3x3
        kSkMatrix,

        kLast = kSkMatrix
    };
    static constexpr int kCTypeCount = static_cast<int>(CType::kLast) + 1;
#endif

    constexpr Uniform()
            : fType      (static_cast<unsigned>(kVoid_GrSLType))
            , fCount     (static_cast<unsigned>(kNonArray))
//            , fVisibility(static_cast<unsigned>(GrShaderFlags::kNone_GrShaderFlags))
//            , fCType     (static_cast<unsigned>(CType::kDefault))
            , fOffset    (0) {}

    constexpr Uniform(GrSLType      type,
                      ptrdiff_t     offset)
//                      GrShaderFlags visibility = kFragment_GrShaderFlag,
//                      CType         ctype      = CType::kDefault)
            : Uniform(type, kNonArray, offset/*, visibility, ctype*/) {}

    constexpr Uniform(GrSLType      type,
                      int           arrayCount,
                      size_t        offset)
//                      GrShaderFlags visibility = kFragment_GrShaderFlag,
//                      CType         ctype      = CType::kDefault)
            : fType      (static_cast<unsigned>(type      ))
            , fCount     (static_cast<unsigned>(arrayCount))
//            , fVisibility(static_cast<unsigned>(visibility))
//            , fCType     (static_cast<unsigned>(ctype     ))
            , fOffset    (static_cast<unsigned>(offset    )) {
//        SkASSERT(CTypeCompatibleWithType(ctype, type));

        SkASSERT(this->type()       == type      );
        SkASSERT(this->count()      == arrayCount);
        SkASSERT(this->offset()     == offset    );
//        SkASSERT(this->visibility() == visibility);
//        SkASSERT(this->ctype()      == ctype     );
    }

    constexpr Uniform(const Uniform&) = default;

    Uniform& operator=(const Uniform&) = default;

    constexpr bool isInitialized() const { return this->type() != kVoid_GrSLType; }

    constexpr GrSLType      type      () const { return static_cast<GrSLType>     (fType);   }
    constexpr int           count     () const { return static_cast<int>          (fCount);  }
//    constexpr CType         ctype     () const { return static_cast<CType>        (fCType);  }
    constexpr size_t        offset    () const { return static_cast<GrShaderFlags>(fOffset); }
//    constexpr GrShaderFlags visibility() const {
//        return static_cast<GrShaderFlags>(fVisibility);
//    }

//    static constexpr bool CTypeCompatibleWithType(CType, GrSLType);

private:
    unsigned    fType       : 6;
    unsigned    fCount      : 8;
//    unsigned    fVisibility : 4;
//    unsigned    fCType      : 1;
    unsigned    fOffset     : 32 - (6 + 8 + 4 + 1);

    static_assert(kGrSLTypeCount <= (1 << 6));
//    static_assert(kCTypeCount    <= (1 << 1));
};

} // namespace skgpu

#endif // skgpu_Uniform_DEFINED
