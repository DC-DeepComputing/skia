/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWhatWhat_DEFINED
#define SkWhatWhat_DEFINED

namespace skgpu { class Recorder; }

class GrRecordingContext;
class SkShaderCodeDictionary;

class SkWhatWhat {
public:
    SkWhatWhat(skgpu::Recorder* recorder) : fRecorder(recorder) {}
    SkWhatWhat(GrRecordingContext* rContext) : fRecordingContext(rContext) {}

    SkShaderCodeDictionary* dict() const;

    skgpu::Recorder* fRecorder = nullptr;
    GrRecordingContext* fRecordingContext = nullptr;
};

#endif // SkWhatWhat_DEFINED
