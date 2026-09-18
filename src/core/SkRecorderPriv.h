/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRecorderPriv_DEFINED
#define SkRecorderPriv_DEFINED

class SkCanvas;
class SkSurface;
class SkRecorder;
class SkImageInfo;
class SkSurfaceProps;

class SK_API SkRecorderPriv {
public:
    explicit SkRecorderPriv(SkRecorder* recorder) : fRecorder(recorder) {}

    sk_sp<SkSurface> makeSurface(SkImageInfo info,
                                 size_t rowBytes,
                                 const SkSurfaceProps* props = nullptr);

private:
    friend class SkRecorder;
    SkRecorder* fRecorder;
};

#endif
