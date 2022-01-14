/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlender_DEFINED
#define SkBlender_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkFlattenable.h"

enum class SkBackend : uint8_t;
class SkPaintParamsKey;
class SkShaderCodeDictionary;

/**
 * SkBlender represents a custom blend function in the Skia pipeline. When an SkBlender is
 * present in a paint, the SkBlendMode is ignored. A blender combines a source color (the
 * result of our paint) and destination color (from the canvas) into a final color.
 */
class SK_API SkBlender : public SkFlattenable {
public:
    /**
     * Create a blender that implements the specified BlendMode.
     */
    static sk_sp<SkBlender> Mode(SkBlendMode mode);

    /**     (experimental - do not use)
        Add implementation details, for the specified backend, of this SkBlender to the
        provided key.

        @param dictionary dictionary of code fragments available to be used in the key
        @param backend    the backend that would be carrying out the drawing
        @param key        destination for implementation details of this SkBlender
    */
    virtual void addToKey_experimental(SkShaderCodeDictionary* dictionary,
                                       SkBackend backend,
                                       SkPaintParamsKey* end) const;

private:
    SkBlender() = default;
    friend class SkBlenderBase;

    using INHERITED = SkFlattenable;
};

#endif
