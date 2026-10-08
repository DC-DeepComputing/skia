/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCaptureDrawId_DEFINED
#define SkCaptureDrawId_DEFINED

#include <cstdint>

/**
 * Identifies the SkCapture draw command that is currently being forwarded by an SkCaptureCanvas to
 * its base canvas. Used (when draw id tracking is enabled) by GPU backends to attribute the GPU work
 * they record back to the Skia draw call that caused it.
 *
 *  - fCanvasToken identifies one recording session of one SkCaptureCanvas, i.e. one captured
 *    SkPicture asset. The same token is stored in the asset as an annotation (kAnnotationKey) so a
 *    viewer can match GPU work to the asset. 0 means "not inside a captured draw".
 *  - fOrdinal is the 0-based index of the draw among the draw calls of that recording session.
 *    Only draw calls are counted (not save/restore/clip/matrix ops or annotations), so the viewer
 *    can recover it by counting draw commands while playing back the asset.
 *
 * The current id is thread-local: SkCaptureCanvas and the Recorder it draws through are used on
 * the same thread.
 */
struct SkCaptureDrawId {
    // Annotation key of the per-asset token annotation. Its value is the uint32_t fCanvasToken.
    static constexpr const char kAnnotationKey[] = "skcapture.drawid.token";

    uint32_t fCanvasToken = 0;
    uint32_t fOrdinal = 0;

    bool isValid() const { return fCanvasToken != 0; }

    static SkCaptureDrawId& Current() {
        static thread_local SkCaptureDrawId gCurrent;
        return gCurrent;
    }
};

// Sets SkCaptureDrawId::Current() for the lifetime of the scope, restoring the previous id after.
class SkAutoCaptureDrawId {
public:
    explicit SkAutoCaptureDrawId(SkCaptureDrawId id) : fPrev(SkCaptureDrawId::Current()) {
        SkCaptureDrawId::Current() = id;
    }
    ~SkAutoCaptureDrawId() { SkCaptureDrawId::Current() = fPrev; }

    SkAutoCaptureDrawId(const SkAutoCaptureDrawId&) = delete;
    SkAutoCaptureDrawId& operator=(const SkAutoCaptureDrawId&) = delete;

private:
    SkCaptureDrawId fPrev;
};

#endif  // SkCaptureDrawId_DEFINED
