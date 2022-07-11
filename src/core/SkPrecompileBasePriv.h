/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPrecompileBasePriv_DEFINED
#define SkPrecompileBasePriv_DEFINED

#include "src/core/SkPrecompile.h"

/** Class that exposes methods to SkPrecompileBase that are only intended for use internal to Skia.
    This class is purely a privileged window into SkPrecompileBase. It should never have additional
    data members or virtual methods. */
class SkPrecompileBasePriv {
public:
    int numCombinations1(SkBackend backend) const {
        return fPrecompileBase->numCombinations1(backend);
    }

private:
    friend class SkPrecompileBase; // to construct/copy this type.

    explicit SkPrecompileBasePriv(SkPrecompileBase* precompileBase)
            : fPrecompileBase(precompileBase) {
    }

    SkPrecompileBasePriv& operator=(const SkPrecompileBasePriv&) = delete;

    // No taking addresses of this type.
    const SkPrecompileBasePriv* operator&() const;
    SkPrecompileBasePriv *operator&();

    SkPrecompileBase* fPrecompileBase;
};

inline SkPrecompileBasePriv SkPrecompileBase::priv() { return SkPrecompileBasePriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const SkPrecompileBasePriv SkPrecompileBase::priv() const {
    return SkPrecompileBasePriv(const_cast<SkPrecompileBase *>(this));
}

#endif // SkPrecompileBasePriv_DEFINED
