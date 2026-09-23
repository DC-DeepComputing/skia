/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_DNG_DECODER_WRAPPER_H
#define SK_DNG_DECODER_WRAPPER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t (*DngReadFn)(void* user_data, void* dst, size_t offset, size_t count);
typedef uint64_t (*DngGetLengthFn)(void* user_data);

typedef struct {
    void* user_data;
    DngReadFn read;
    DngGetLengthFn get_length;
} DngStreamOps;

typedef struct {
    bool has_jpeg_preview;
    size_t preview_offset;
    size_t preview_length;
    bool is_adobe_rgb;
} DngPreviewInfo;

typedef struct {
    int width;
    int height;
    bool is_scalable;
    bool is_xtrans;
    DngPreviewInfo preview;
} DngImageInfo;

typedef struct DngImageOpaque* DngImageHandle;
typedef struct DngRenderOpaque* DngRenderHandle;

#ifndef DNG_DECODER_API
#define DNG_DECODER_API __attribute__((visibility("default")))
#endif

// Probes stream with PIEX and DNG SDK.
// Catches all exceptions internally; returns NULL on error.
DNG_DECODER_API DngImageHandle DngDecoder_Create(const DngStreamOps* stream, DngImageInfo* out_info);

// Initiates rendering to target_width x target_height.
// actual_width and actual_height receive the rendered dimensions.
// Catches all exceptions internally; returns NULL on error.
DNG_DECODER_API DngRenderHandle DngDecoder_RenderInit(DngImageHandle handle, int target_width, int target_height,
                                                      int* actual_width, int* actual_height);

// Reads a single row (0 .. actual_height-1) into dst_rgb888 (size >= actual_width * 3).
// Catches all exceptions internally; returns true on success, false on error.
DNG_DECODER_API bool DngDecoder_RenderGetRow(DngRenderHandle render_handle, int row_index, void* dst_rgb888);

// Destroys the render handle.
DNG_DECODER_API void DngDecoder_RenderDestroy(DngRenderHandle render_handle);

// Destroys the image handle.
DNG_DECODER_API void DngDecoder_Destroy(DngImageHandle handle);

#ifdef __cplusplus
}
#endif

#endif  // SK_DNG_DECODER_WRAPPER_H
