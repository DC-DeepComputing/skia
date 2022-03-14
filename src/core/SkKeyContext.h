/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyContext_DEFINED
#define SkKeyContext_DEFINED

namespace skgpu { class Recorder; }

class GrRecordingContext;
class SkShaderCodeDictionary;

class SkKeyContext {
public:
    SkKeyContext(skgpu::Recorder*);                                   // Graphite
    SkKeyContext(GrRecordingContext*);                                // Ganesh
    SkKeyContext(SkShaderCodeDictionary* dict) : fDictionary(dict) {} // pre-compile

    SkShaderCodeDictionary* dict() const { return fDictionary; }

    skgpu::Recorder* fRecorder = nullptr;
    GrRecordingContext* fRecordingContext = nullptr;

    SkShaderCodeDictionary* fDictionary;
};

#endif // SkKeyContext_DEFINED
