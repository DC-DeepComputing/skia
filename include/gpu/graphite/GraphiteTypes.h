/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_GraphiteTypes_DEFINED
#define skgpu_graphite_GraphiteTypes_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"

#include <memory>

class SkSurface;

namespace skgpu {
class MutableTextureState;
}

namespace skgpu::graphite {

class BackendSemaphore;
class Recording;
class Task;

using GpuFinishedContext = void*;
using GpuFinishedProc = void (*)(GpuFinishedContext finishedContext, CallbackResult);

/**
 * The fFinishedProc is called when the Recording has been submitted and finished on the GPU, or
 * when there is a failure that caused it not to be submitted. The callback will always be called
 * and the caller can use the callback to know it is safe to free any resources associated with
 * the Recording that they may be holding onto. If the Recording is successfully submitted to the
 * GPU the callback will be called with CallbackResult::kSuccess once the GPU has finished. All
 * other cases where some failure occured it will be called with CallbackResult::kFailed.
 *
 * The fTargetSurface, if provided, is used as a target for any draws recorded onto a deferred
 * canvas returned from Recorder::makeDeferredCanvas. This target surface must be provided iff
 * the Recording contains any such draws. It must be Graphite-backed and its backing texture's
 * TextureInfo must match the info provided to the Recorder when making the deferred canvas.
 *
 * fTargetTranslation is an additional translation applied to draws targeting fTargetSurface.
 *
 * The client may pass in two arrays of initialized BackendSemaphores to be included in the
 * command stream. At some time before issuing commands in the Recording, the fWaitSemaphores will
 * be waited on by the gpu. We only guarantee these wait semaphores block transfer and fragment
 * shader work. Similarly, at some time after issuing the Recording's commands, the
 * fSignalSemaphores will be signaled by the gpu. Depending on the platform, the timing of the wait
 * and signal operations will either be immediately before or after the given Recording's command
 * stream, respectively, or before and after the entire CommandBuffer's command stream. The
 * semaphores are not sent to the GPU until the next Context::submit call is made.
 *
 * The client will own and be responsible for deleting the underlying semaphore objects after the
 * submission completes, however the BackendSemaphore objects themselves can be deleted as soon
 * as this function returns.
 */
struct InsertRecordingInfo {
    Recording* fRecording = nullptr;

    SkSurface* fTargetSurface = nullptr;
    SkIVector fTargetTranslation = {0, 0};
    MutableTextureState* fTargetTextureState = nullptr;

    size_t fNumWaitSemaphores = 0;
    BackendSemaphore* fWaitSemaphores = nullptr;
    size_t fNumSignalSemaphores = 0;
    BackendSemaphore* fSignalSemaphores = nullptr;

    GpuFinishedContext fFinishedContext = nullptr;
    GpuFinishedProc fFinishedProc = nullptr;
};

/**
 * The fFinishedProc is called when the Recording has been submitted and finished on the GPU, or
 * when there is a failure that caused it not to be submitted. The callback will always be called
 * and the caller can use the callback to know it is safe to free any resources associated with
 * the Recording that they may be holding onto. If the Recording is successfully submitted to the
 * GPU the callback will be called with CallbackResult::kSuccess once the GPU has finished. All
 * other cases where some failure occured it will be called with CallbackResult::kFailed.
 */
struct InsertFinishInfo {
    GpuFinishedContext fFinishedContext = nullptr;
    GpuFinishedProc fFinishedProc = nullptr;
};

/**
 * Actually submit work to the GPU and track its completion
 */
enum class SyncToCpu : bool {
    kYes = true,
    kNo = false
};

/*
 * For Promise Images - should the Promise Image be fulfilled every time a Recording that references
 * it is inserted into the Context.
 */
enum class Volatile : bool {
    kNo = false,              // only fulfilled once
    kYes = true               // fulfilled on every insertion call
};

/*
 * Graphite's different rendering methods each only apply to certain types of draws. This
 * enum supports decision-making regarding the different renderers and what is being drawn.
 */
enum DrawTypeFlags : uint16_t {

    kNone                = 0b000000000,

    // All the following k*Text* flags cover the calls:
    //     SkCanvas:: drawSimpleText, drawString, drawGlyphs, drawTextBlob, drawSlug
    // but each is used under specific conditions (detailed below).

    // Bitmap-based text
    kBitmapText_Mask     = 0b000000001,   // plain vanilla font
    kBitmapText_LCD      = 0b000000010,   // SkSurface w/ a known pixel geometry, font has both
                                          // sub-pixel positioning and sub-pixel-AA edging enabled
    kBitmapText_Color    = 0b000000100,   // font has color glyphs (these are usually emojis)
    kBitmapText_All      = kBitmapText_Mask | kBitmapText_LCD | kBitmapText_Color,

    // SDF-based text
    // The criteria for SDF being used is pretty strict (cf SubRunControl::isSDFT):
    //   The GPU must support shader derivatives
    //   The glyph being drawn is larger (in device space) than
    //           ContextOptions::fMinDistanceFieldFontSize if the SkSurface's surfaceProps has
    //           kUseDeviceIndependentFonts_Flag set
    //        or 162 if that surfaceProps flag is not set
    //        or the canvas matrix has perspective (i.e., perspective obviates the minimum limit).
    //   The glyph being drawn is smaller than ContextOptions::fGlyphsAsPathsFontSize
    //   The SkPaint has no mask filter
    //   The SkPaint's style must be kFill or non-hairline stroke
    kSDFText             = 0b000001000,
    kSDFText_LCD         = 0b000010000,    // Same additional requirements askBitmapText_LCD
    kSDFText_All         = kSDFText | kSDFText_LCD,

    // Path-based text
    kPathBasedText       = 0b000100000,    // The glyph is >= ContextOptions::fGlyphsAsPathsFontSize

    kText                = kBitmapText_All | kSDFText_All | kPathBasedText,

    // SkCanvas::drawVertices
    kDrawVertices        = 0b001000000,

    // SkCanvas::experimental_DrawEdgeAAQuad, experimental_DrawEdgeAAImageSet
    // SkCanvas:: drawRect, drawRRect, drawLine for:
    //    regular filled and hairline [r]rects,
    //    stroked rects,
    //    stroked and hairline lines,
    //    stroked circular rrects
    // Note: clipping can bump a draw out of the simple shape path
    kSimpleShape         = 0b010000000,
    // All other shapes (e.g., any strokeAndFill shape, non-circular-stroked RRects, SkPaths, ...)
    kNonSimpleShape      = 0b100000000,

    kShape               = kSimpleShape | kNonSimpleShape,

    kMostCommon          = kBitmapText_Mask | kSimpleShape,
    kAll                 = kText | kDrawVertices | kShape
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_GraphiteTypes_DEFINED
