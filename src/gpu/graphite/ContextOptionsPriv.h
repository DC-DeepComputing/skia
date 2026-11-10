/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ContextOptionsPriv_DEFINED
#define skgpu_graphite_ContextOptionsPriv_DEFINED

#include "include/private/SkMath.h"

#include <optional>

namespace skgpu::graphite {

enum class PathRendererStrategy;

/**
 * Private options that are only meant for testing within Skia's tools.
 */
struct ContextOptionsPriv {

    int  fMaxTextureSizeOverride = SK_MaxS32;

    /**
     * If true, will store a pointer in Recorder that points back to the Context
     * that created it. Used by readPixels() and other methods that normally require a Context.
     */
    bool fStoreContextRefInRecorder = false;

    /**
     * If true, enables layer based draw ordering.
     */
    bool fDrawListLayer = false;

    /**
     * Override Caps' default strategy heuristics to prioritize this one if set *and* is supported.
     */
    std::optional<PathRendererStrategy> fPathRendererStrategy;

    /**
     * Force DstReadStrategy::kTextureCopy even on backends that support framebuffer fetch or input
     * attachments, allowing DstCopy behavior to be unit-tested on all hardware.
     */
    bool fForceDstReadTextureCopy = false;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ContextOptionsPriv_DEFINED
