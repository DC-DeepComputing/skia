/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkDngDecoderWrapper.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <mutex>
#include <new>

#include "dng_area_task.h"  // NO_G3_REWRITE
#include "dng_color_space.h"  // NO_G3_REWRITE
#include "dng_errors.h"  // NO_G3_REWRITE
#include "dng_exceptions.h"  // NO_G3_REWRITE
#include "dng_host.h"  // NO_G3_REWRITE
#include "dng_image.h"  // NO_G3_REWRITE
#include "dng_info.h"  // NO_G3_REWRITE
#include "dng_memory.h"  // NO_G3_REWRITE
#include "dng_mosaic_info.h"  // NO_G3_REWRITE
#include "dng_negative.h"  // NO_G3_REWRITE
#include "dng_pixel_buffer.h"  // NO_G3_REWRITE
#include "dng_point.h"  // NO_G3_REWRITE
#include "dng_rect.h"  // NO_G3_REWRITE
#include "dng_render.h"  // NO_G3_REWRITE
#include "dng_sdk_limits.h"  // NO_G3_REWRITE
#include "dng_stream.h"  // NO_G3_REWRITE
#include "dng_tag_types.h"  // NO_G3_REWRITE
#include "dng_types.h"  // NO_G3_REWRITE
#include "dng_utils.h"  // NO_G3_REWRITE

#if qDNGUseXMP
#include "dng_xmp_sdk.h"  // NO_G3_REWRITE
#endif

#include "src/piex.h"  // NO_G3_REWRITE
#include "src/piex_types.h"  // NO_G3_REWRITE

namespace {

class DngStreamWrapper : public dng_stream {
public:
    explicit DngStreamWrapper(const DngStreamOps* ops)
        : dng_stream(static_cast<dng_abort_sniffer*>(nullptr), dng_stream::kDefaultBufferSize, 0)
        , fOps(ops) {}

    uint64 DoGetLength() override {
        return fOps->get_length(fOps->user_data);
    }

    void DoRead(void* data, uint32 count, uint64 offset) override {
        size_t bytesRead = fOps->read(fOps->user_data, data, static_cast<size_t>(offset),
                                      static_cast<size_t>(count));
        if (bytesRead < count) {
            ThrowReadFile();
        }
    }

private:
    const DngStreamOps* fOps;
};

class PiexStreamWrapper : public ::piex::StreamInterface {
public:
    explicit PiexStreamWrapper(const DngStreamOps* ops) : fOps(ops) {}

    ::piex::Error GetData(const size_t offset, const size_t length, uint8* data) override {
        size_t bytesRead = fOps->read(fOps->user_data, data, offset, length);
        return (bytesRead == length) ? ::piex::Error::kOk : ::piex::Error::kFail;
    }

private:
    const DngStreamOps* fOps;
};

bool IsTiffHeaderValid(const DngStreamOps* ops) {
    uint8_t header[4];
    if (ops->read(ops->user_data, header, 0, sizeof(header)) != sizeof(header)) {
        return false;
    }
    // Check little-endian ('II') or big-endian ('MM') and magic number 42 (0x2A)
    if (header[0] == 'I' && header[1] == 'I') {
        return header[2] == 0x2A && header[3] == 0x00;
    } else if (header[0] == 'M' && header[1] == 'M') {
        return header[2] == 0x00 && header[3] == 0x2A;
    }
    return false;
}

}  // namespace

struct DngImageOpaque {
    DngStreamOps ops;
    dng_memory_allocator allocator;
    std::unique_ptr<dng_host> host;
    std::unique_ptr<dng_info> info;
    std::unique_ptr<dng_negative> negative;
    std::unique_ptr<DngStreamWrapper> stream;

    int width = 0;
    int height = 0;
    bool is_scalable = false;
    bool is_xtrans = false;

    bool init(int w, int h, const dng_point& cfaPatternSize) {
        width = w;
        height = h;
        is_scalable = (cfaPatternSize.v != 0 && cfaPatternSize.h != 0);
        is_xtrans = is_scalable ? (cfaPatternSize.v == 6 && cfaPatternSize.h == 6) : false;
        return width > 0 && height > 0;
    }

    bool readDng() {
        try {
#if qDNGUseXMP
            static std::once_flag fInitializeDngSDK;
            std::call_once(fInitializeDngSDK, []() {
                dng_xmp_sdk::InitializeSDK();
            });
#endif
            host = std::make_unique<dng_host>(&allocator);
            info = std::make_unique<dng_info>();
            stream = std::make_unique<DngStreamWrapper>(&ops);

            host->ValidateSizes();
            info->Parse(*host, *stream);
            info->PostParse(*host);
            if (!info->IsValidDNG()) {
                return false;
            }

            negative.reset(host->Make_dng_negative());
            negative->Parse(*host, *stream, *info);
            negative->PostParse(*host, *stream, *info);
            negative->SynchronizeMetadata();

            dng_point cfaPatternSize(0, 0);
            if (negative->GetMosaicInfo() != nullptr) {
                cfaPatternSize = negative->GetMosaicInfo()->fCFAPatternSize;
            }
            return init(static_cast<int>(negative->DefaultCropSizeH().As_real64()),
                        static_cast<int>(negative->DefaultCropSizeV().As_real64()),
                        cfaPatternSize);
        } catch (...) {
            return false;
        }
    }
};

struct DngRenderOpaque {
    std::unique_ptr<dng_image> image;
    int actual_width = 0;
    int actual_height = 0;
};

DngImageHandle DngDecoder_Create(const DngStreamOps* ops, DngImageInfo* out_info) {
    if (!ops || !ops->read || !ops->get_length || !out_info) {
        return nullptr;
    }

    *out_info = {};

    try {
        PiexStreamWrapper piexStream(ops);
        ::piex::PreviewImageData imageData;
        if (::piex::IsRaw(&piexStream)) {
            ::piex::Error error = ::piex::GetPreviewImageData(&piexStream, &imageData);
            if (error == ::piex::Error::kFail) {
                return nullptr;
            }

            if (error == ::piex::Error::kOk && imageData.preview.length > 0 &&
                imageData.preview.format == ::piex::Image::kJpegCompressed) {
                out_info->preview.has_jpeg_preview = true;
                out_info->preview.preview_offset = imageData.preview.offset;
                out_info->preview.preview_length = imageData.preview.length;
                out_info->preview.is_adobe_rgb =
                    (imageData.color_space == ::piex::PreviewImageData::kAdobeRgb);
                out_info->width = static_cast<int>(imageData.full_width);
                out_info->height = static_cast<int>(imageData.full_height);
                return nullptr;
            }

            if (error == ::piex::Error::kOk) {
                auto image = std::make_unique<DngImageOpaque>();
                image->ops = *ops;
                dng_point cfaPatternSize(imageData.cfa_pattern_dim[1],
                                         imageData.cfa_pattern_dim[0]);
                if (image->init(static_cast<int>(imageData.full_width),
                                static_cast<int>(imageData.full_height),
                                cfaPatternSize)) {
                    out_info->width = image->width;
                    out_info->height = image->height;
                    out_info->is_scalable = image->is_scalable;
                    out_info->is_xtrans = image->is_xtrans;
                    return image.release();
                }
            }
        }

        if (!IsTiffHeaderValid(ops)) {
            return nullptr;
        }

        auto image = std::make_unique<DngImageOpaque>();
        image->ops = *ops;
        if (!image->readDng()) {
            return nullptr;
        }

        out_info->width = image->width;
        out_info->height = image->height;
        out_info->is_scalable = image->is_scalable;
        out_info->is_xtrans = image->is_xtrans;
        return image.release();
    } catch (...) {
        return nullptr;
    }
}

DngRenderHandle DngDecoder_RenderInit(DngImageHandle handle, int target_width, int target_height,
                                      int* actual_width, int* actual_height) {
    if (!handle || !actual_width || !actual_height) {
        return nullptr;
    }

    try {
        if (!handle->host || !handle->info || !handle->negative || !handle->stream) {
            if (!handle->readDng()) {
                return nullptr;
            }
        }

        const int preferredSize = std::max(target_width, target_height);
        std::unique_ptr<dng_host> host = std::move(handle->host);
        std::unique_ptr<dng_info> info = std::move(handle->info);
        std::unique_ptr<dng_negative> negative = std::move(handle->negative);
        std::unique_ptr<DngStreamWrapper> stream = std::move(handle->stream);

        host->SetPreferredSize(preferredSize);
        host->ValidateSizes();

        negative->ReadStage1Image(*host, *stream, *info);

        if (info->fMaskIndex != -1) {
            negative->ReadTransparencyMask(*host, *stream, *info);
        }

        negative->ValidateRawImageDigest(*host);
        if (negative->IsDamaged()) {
            return nullptr;
        }

        const int32 kMosaicPlane = -1;
        negative->BuildStage2Image(*host);
        negative->BuildStage3Image(*host, kMosaicPlane);

        dng_render render(*host, *negative);
        render.SetFinalSpace(dng_space_sRGB::Get());
        render.SetFinalPixelType(ttByte);

        dng_point stage3_size = negative->Stage3Image()->Size();
        render.SetMaximumSize(std::max(stage3_size.h, stage3_size.v));

        std::unique_ptr<dng_image> rendered(render.Render());
        if (!rendered) {
            return nullptr;
        }

        auto renderHandle = std::make_unique<DngRenderOpaque>();
        renderHandle->actual_width = rendered->Width();
        renderHandle->actual_height = rendered->Height();
        renderHandle->image = std::move(rendered);

        *actual_width = renderHandle->actual_width;
        *actual_height = renderHandle->actual_height;
        return renderHandle.release();
    } catch (...) {
        return nullptr;
    }
}

bool DngDecoder_RenderGetRow(DngRenderHandle render_handle, int row_index, void* dst_rgb888) {
    if (!render_handle || !render_handle->image || !dst_rgb888 ||
        row_index < 0 || row_index >= render_handle->actual_height) {
        return false;
    }

    try {
        dng_pixel_buffer buffer;
        buffer.fData = dst_rgb888;
        buffer.fPlane = 0;
        buffer.fPlanes = 3;
        buffer.fColStep = 3;
        buffer.fPlaneStep = 1;
        buffer.fPixelType = ttByte;
        buffer.fPixelSize = sizeof(uint8_t);
        buffer.fRowStep = render_handle->actual_width * 3;
        buffer.fArea = dng_rect(row_index, 0, row_index + 1, render_handle->actual_width);

        render_handle->image->Get(buffer, dng_image::edge_zero);
        return true;
    } catch (...) {
        return false;
    }
}

void DngDecoder_RenderDestroy(DngRenderHandle render_handle) {
    delete render_handle;
}

void DngDecoder_Destroy(DngImageHandle handle) {
    delete handle;
}
