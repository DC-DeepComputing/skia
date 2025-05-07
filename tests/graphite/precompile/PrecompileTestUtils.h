/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef tests_graphite_precompile_PrecompileTestUtils_DEFINED
#define tests_graphite_precompile_PrecompileTestUtils_DEFINED

#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"

struct PrecompileSettings {
    skgpu::graphite::PaintOptions fPaintOptions;
    skgpu::graphite::DrawTypeFlags fDrawTypeFlags = skgpu::graphite::DrawTypeFlags::kNone;
    skgpu::graphite::RenderPassProperties fRenderPassProps;

    bool isSubsetOf(const PrecompileSettings& superSet) const;
};

#if defined(SK_DEBUG)
void DeduceSettingsFromLabel(const char* testStr, PrecompileSettings* result);
#endif

#endif
