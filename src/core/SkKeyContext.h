/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyContext_DEFINED
#define SkKeyContext_DEFINED

#include "include/gpu/GrTypes.h"

#ifdef SK_GRAPHITE_ENABLED
namespace skgpu { class Recorder; }
#endif

#if SK_SUPPORT_GPU
class GrRecordingContext;
#endif

class SkShaderCodeDictionary;

// The key context must always be able to provide a valid SkDictionary. Depending on the calling
// context it can also supply a backend-specific resource providing object (e.g., a Recorder).
class SkKeyContext {
public:
#ifdef SK_GRAPHITE_ENABLED
    SkKeyContext(skgpu::Recorder*);
#endif

#if SK_SUPPORT_GPU
    SkKeyContext(GrRecordingContext*);                                // Ganesh
#endif

    // Constructor for the pre-compile code path
    SkKeyContext(SkShaderCodeDictionary* dict) : fDictionary(dict) {}

    SkShaderCodeDictionary* dict() const { return fDictionary; }

#ifdef SK_GRAPHITE_ENABLED
    skgpu::Recorder* fRecorder = nullptr;
#endif

#if SK_SUPPORT_GPU
    GrRecordingContext* fRecordingContext = nullptr;
#endif

    SkShaderCodeDictionary* fDictionary;
};

#endif // SkKeyContext_DEFINED
