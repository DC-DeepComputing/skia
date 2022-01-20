/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUniform_DEFINED
#define SkUniform_DEFINED

#include "experimental/graphite/src/DrawTypes.h"

/**
 * Describes a uniform. Uniforms consist of:
 *  type:       The type of the uniform
 *  count:      Number of elements of 'type' in the array or kNonArray if not an array.
 */
class SkUniform {
public:
    static constexpr int kNonArray = 0;

    constexpr SkUniform(const char* name, skgpu::SLType type) : SkUniform(name, type, kNonArray) {}

    constexpr SkUniform(const char* name, skgpu::SLType type, int count)
            : fType      (static_cast<unsigned>(type))
            , fCount     (static_cast<unsigned>(count))
            , fName      (name) {
    }

    constexpr SkUniform(const SkUniform&) = default;
    SkUniform& operator=(const SkUniform&) = default;

    constexpr bool isInitialized() const { return this->type() != skgpu::SLType::kVoid; }

    constexpr const char* name() const  { return fName; }
    constexpr skgpu::SLType   type() const  { return static_cast<skgpu::SLType>(fType);  }
    constexpr uint32_t count() const { return static_cast<uint32_t>(fCount); }

private:
    uint32_t    fType    : 6;
    uint32_t    fCount   : 26;
    const char* fName;

    static_assert(skgpu::kSLTypeCount <= (1 << 6));
};

#endif // SkUniform_DEFINED
