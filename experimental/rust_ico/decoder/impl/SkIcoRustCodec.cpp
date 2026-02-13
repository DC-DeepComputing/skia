/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_ico/decoder/impl/SkIcoRustCodec.h"

#include "experimental/rust_ico/decoder/impl/SkIcoEntryCodec.h"
#include "experimental/rust_ico/ffi/FFI.rs.h"
#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkTemplates.h"
#include "src/codec/SkCodecPriv.h"
#include "src/core/SkStreamPriv.h"

#include "modules/skcms/skcms.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

using namespace skia_private;

class SkSampler;

// Checks the start of the stream to see if the image is an ICO or CUR.
bool SkIcoRustCodec::IsIco(const void* buffer, size_t bytesRead) {
    const rust::Slice<const uint8_t> data_slice{
        static_cast<const uint8_t*>(buffer),
        bytesRead
    };
    return rust_ico::is_ico_data(data_slice);
}

std::unique_ptr<SkCodec> SkIcoRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                        Result* result) {
    // Handle nullptr result parameter by using local storage
    Result resultStorage;
    if (result == nullptr) {
        result = &resultStorage;
    }

    if (!stream) {
        SkCodecPrintf("Error: ICO stream is null.\n");
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }

    // It is helpful to have the entire stream in a contiguous buffer.
    sk_sp<const SkData> data = nullptr;
    if (stream->getMemoryBase()) {
        // It is safe to make without copy because we'll hold onto the stream.
        data = SkData::MakeWithoutCopy(stream->getMemoryBase(), stream->getLength());
    } else {
        data = SkStreamPriv::CopyStreamToData(stream.get());
        // Note: Unlike the original SkIcoCodec, we do NOT reset the stream here.
        // Blink's SkiaImageDecoderBase expects the codec to keep the stream alive.
    }

    if (!data) {
        SkCodecPrintf("Error: CopyStreamToData returned null.\n");
        *result = kIncompleteInput;
        return nullptr;
    }

    if (data->size() == 0) {
        SkCodecPrintf("Error: CopyStreamToData returned empty data.\n");
        *result = kIncompleteInput;
        return nullptr;
    }

    // Parse ICO directory using Rust
    const rust::Slice<const uint8_t> data_slice{data->bytes(), data->size()};
    rust::Box<rust_ico::DirectoryResult> directoryResult = rust_ico::parse_directory(data_slice);

    // Map Rust parse result to SkCodec::Result
    switch (directoryResult->status()) {
        case rust_ico::DirectoryParseResult::Success:
            break;
        case rust_ico::DirectoryParseResult::InsufficientData:
        case rust_ico::DirectoryParseResult::TruncatedDirectory:
            SkCodecPrintf("Error: ICO data truncated.\n");
            *result = kIncompleteInput;
            return nullptr;
        case rust_ico::DirectoryParseResult::InvalidSignature:
            SkCodecPrintf("Error: Invalid ICO signature.\n");
            *result = kInvalidInput;
            return nullptr;
        case rust_ico::DirectoryParseResult::NoImages:
            SkCodecPrintf("Error: No images embedded in ICO.\n");
            *result = kInvalidInput;
            return nullptr;
    }

    const uint32_t numImages = directoryResult->image_count();
    const bool isCursor = (directoryResult->file_type() == rust_ico::IcoFileType::Cursor);

    // Default Result, if no valid embedded codecs are found.
    *result = kInvalidInput;

    // Structure to hold codec and its metadata for sorting
    struct CodecEntry {
        std::unique_ptr<SkCodec> codec;
        HotSpot hotSpot;
        uint16_t bitCount;
    };
    std::vector<CodecEntry> entries;
    entries.reserve(numImages);

    // Construct a candidate codec for each of the embedded images
    // Entries are already sorted by offset in Rust for proper size calculation
    uint32_t bytesRead = 0;

    for (uint32_t i = 0; i < numImages; i++) {
        rust_ico::IcoEntry entry = directoryResult->get_entry(i);
        uint32_t offset = entry.offset;
        uint32_t dirSize = entry.size;  // Size from directory (may be incorrect)

        // Calculate the actual available size for this entry.
        // The directory size field is sometimes incorrect (especially for PNG).
        // Use either the space to the next entry, or to EOF for the last entry.
        uint32_t actualSize;
        if (i + 1 < numImages) {
            // Not the last entry - use space to next entry's offset
            rust_ico::IcoEntry nextEntry = directoryResult->get_entry(i + 1);
            actualSize = nextEntry.offset - offset;
        } else {
            // Last entry - use all remaining data
            actualSize = data->size() - offset;
        }

        // Use the larger of directory size and calculated size
        // (some ICOs have correct sizes, some don't)
        uint32_t size = std::max(dirSize, actualSize);

        // Ensure that the offset is valid
        if (offset < bytesRead) {
            SkCodecPrintf("Warning: invalid ico offset.\n");
            continue;
        }

        // If we cannot skip, assume we have reached the end of the stream
        if (offset >= data->size()) {
            SkCodecPrintf("Warning: could not skip to ico offset.\n");
            break;
        }
        bytesRead = offset;

        if (offset + size > data->size()) {
            // Clamp size to available data
            size = data->size() - offset;
        }

        sk_sp<const SkData> embeddedData(SkData::MakeSubset(data.get(), offset, size));
        bytesRead += size;

        // Create codec using Rust ICO entry decoder for both PNG and BMP
        Result entryResult;
        std::unique_ptr<SkCodec> codec = SkIcoEntryCodec::Make(
                embeddedData->bytes(),
                embeddedData->size(),
                entry.format,
                size,
                &entryResult);

        if (nullptr != codec) {
            // Store codec with its hotspot and bit count for sorting
            entries.push_back({
                std::move(codec),
                {static_cast<int16_t>(entry.hotspot_x),
                 static_cast<int16_t>(entry.hotspot_y)},
                entry.bit_count
            });
        }
    }

    if (entries.empty()) {
        SkCodecPrintf("Error: could not find any valid embedded ico codecs.\n");
        return nullptr;
    }

    // Sort entries by decreasing quality (area first, then bit depth)
    // This matches the behavior of Blink's ICOImageDecoder
    std::sort(entries.begin(), entries.end(), [](const CodecEntry& a, const CodecEntry& b) {
        SkImageInfo infoA = a.codec->getInfo();
        SkImageInfo infoB = b.codec->getInfo();
        int areaA = infoA.width() * infoA.height();
        int areaB = infoB.width() * infoB.height();
        if (areaA != areaB) {
            return areaA > areaB;  // Larger area first
        }
        return a.bitCount > b.bitCount;  // Higher bit depth as tiebreaker
    });

    // Extract sorted codecs and hotspots into separate containers
    auto codecs = std::make_unique<TArray<std::unique_ptr<SkCodec>>>(entries.size());
    std::vector<HotSpot> hotSpots;
    hotSpots.reserve(entries.size());
    for (auto& entry : entries) {
        codecs->push_back(std::move(entry.codec));
        hotSpots.push_back(entry.hotSpot);
    }

    // Use the first (largest) codec's info
    auto maxInfo = codecs->at(0)->getEncodedInfo().copy();

    *result = kSuccess;
    return std::unique_ptr<SkCodec>(
            new SkIcoRustCodec(std::move(maxInfo), std::move(stream), std::move(codecs),
                               isCursor, std::move(hotSpots)));
}

SkIcoRustCodec::SkIcoRustCodec(SkEncodedInfo&& info,
                               std::unique_ptr<SkStream> stream,
                               std::unique_ptr<TArray<std::unique_ptr<SkCodec>>> codecs,
                               bool isCursor,
                               std::vector<HotSpot> hotSpots)
        // The source skcms_PixelFormat will not be used. The embedded
        // codec's will be used instead.
        : INHERITED(std::move(info), skcms_PixelFormat(), std::move(stream))
        , fEmbeddedCodecs(std::move(codecs))
        , fIsCursor(isCursor)
        , fHotSpots(std::move(hotSpots))
        , fCurrCodec(nullptr) {
    // Initialize frame holder with info from all embedded codecs
    // Use the largest image dimensions as the screen size
    int maxWidth = 0, maxHeight = 0;
    for (int i = 0; i < fEmbeddedCodecs->size(); i++) {
        SkImageInfo imgInfo = fEmbeddedCodecs->at(i)->getInfo();
        if (imgInfo.width() > maxWidth) maxWidth = imgInfo.width();
        if (imgInfo.height() > maxHeight) maxHeight = imgInfo.height();
    }
    fFrameHolder.setScreenSize(maxWidth, maxHeight);

    // Add a frame entry for each embedded codec
    for (int i = 0; i < fEmbeddedCodecs->size(); i++) {
        SkEncodedInfo encodedInfo = fEmbeddedCodecs->at(i)->getEncodedInfo().copy();
        SkImageInfo imgInfo = fEmbeddedCodecs->at(i)->getInfo();
        fFrameHolder.appendFrame(i, imgInfo.width(), imgInfo.height(), encodedInfo.alpha());
    }
}

/*
 * Returns the number of embedded images (frames) in this ICO.
 * Each embedded image is exposed as a separate frame.
 */
int SkIcoRustCodec::onGetFrameCount() {
    return fEmbeddedCodecs->size();
}

/*
 * Returns frame info for the embedded image at the given index.
 */
bool SkIcoRustCodec::onGetFrameInfo(int index, FrameInfo* info) const {
    if (index < 0 || index >= fEmbeddedCodecs->size()) {
        return false;
    }

    if (info) {
        const SkCodec* embeddedCodec = fEmbeddedCodecs->at(index).get();
        SkImageInfo embeddedInfo = embeddedCodec->getInfo();

        // ICO frames are independent - they don't depend on previous frames
        info->fRequiredFrame = SkCodec::kNoFrame;
        info->fDuration = 0;  // ICO frames are static
        info->fFullyReceived = true;
        info->fAlphaType = embeddedInfo.alphaType();
        info->fHasAlphaWithinBounds = (embeddedInfo.alphaType() != kOpaque_SkAlphaType);
        info->fDisposalMethod = SkCodecAnimation::DisposalMethod::kKeep;
        info->fBlend = SkCodecAnimation::Blend::kSrc;
        info->fFrameRect = SkIRect::MakeSize(embeddedInfo.dimensions());
    }

    return true;
}

/*
 * Chooses the best dimensions given the desired scale.
 */
SkISize SkIcoRustCodec::onGetScaledDimensions(float desiredScale) const {
    int origWidth = this->dimensions().width();
    int origHeight = this->dimensions().height();
    float desiredSize = desiredScale * origWidth * origHeight;
    float minError = ((float) (origWidth * origHeight)) - desiredSize + 1.0f;
    int32_t minIndex = -1;
    for (int32_t i = 0; i < fEmbeddedCodecs->size(); i++) {
        auto dimensions = fEmbeddedCodecs->at(i)->dimensions();
        int width = dimensions.width();
        int height = dimensions.height();
        float error = SkTAbs(((float) (width * height)) - desiredSize);
        if (error < minError) {
            minError = error;
            minIndex = i;
        }
    }
    SkASSERT(minIndex >= 0);

    return fEmbeddedCodecs->at(minIndex)->dimensions();
}

int SkIcoRustCodec::chooseCodec(const SkISize& requestedSize, int startIndex) {
    SkASSERT(startIndex >= 0);

    for (int i = startIndex; i < fEmbeddedCodecs->size(); i++) {
        if (fEmbeddedCodecs->at(i)->dimensions() == requestedSize) {
            return i;
        }
    }

    return -1;
}

bool SkIcoRustCodec::onDimensionsSupported(const SkISize& dim) {
    return this->chooseCodec(dim, 0) >= 0;
}

/*
 * Initiates the ICO decode.
 * If opts.fFrameIndex is specified, decode that specific embedded image.
 * Otherwise, find an embedded codec matching the requested dimensions.
 */
SkCodec::Result SkIcoRustCodec::onGetPixels(const SkImageInfo& dstInfo,
                                            void* dst, size_t dstRowBytes,
                                            const Options& opts,
                                            int* rowsDecoded) {
    if (opts.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }

    // If a specific frame index is requested, use that embedded codec directly.
    if (opts.fFrameIndex >= 0 && opts.fFrameIndex < fEmbeddedCodecs->size()) {
        SkCodec* embeddedCodec = fEmbeddedCodecs->at(opts.fFrameIndex).get();
        // Verify dimensions match
        if (embeddedCodec->dimensions() == dstInfo.dimensions()) {
            // Create a copy of options with fFrameIndex reset to 0 for the embedded codec.
            // Each embedded image is a separate codec with its own single frame (index 0).
            Options embeddedOpts = opts;
            embeddedOpts.fFrameIndex = 0;

            SkCodec::Result result = embeddedCodec->getPixels(dstInfo, dst, dstRowBytes, &embeddedOpts);
            if (result == kSuccess || result == kIncompleteInput) {
                *rowsDecoded = dstInfo.height();
            }
            return result;
        }
    }

    // Fall back to dimension-based codec selection (legacy behavior).
    int index = 0;
    SkCodec::Result result = kInvalidScale;
    while (true) {
        index = this->chooseCodec(dstInfo.dimensions(), index);
        if (index < 0) {
            break;
        }

        SkCodec* embeddedCodec = fEmbeddedCodecs->at(index).get();
        // Reset frame index for embedded codec
        Options embeddedOpts = opts;
        embeddedOpts.fFrameIndex = 0;

        result = embeddedCodec->getPixels(dstInfo, dst, dstRowBytes, &embeddedOpts);
        switch (result) {
            case kSuccess:
            case kIncompleteInput:
                *rowsDecoded = dstInfo.height();
                return result;
            default:
                break;
        }

        index++;
    }

    SkCodecPrintf("Error: No matching candidate image in ico.\n");
    return result;
}

SkCodec::Result SkIcoRustCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
        const SkCodec::Options& options) {
    // If a specific frame index is requested, use that embedded codec directly.
    if (options.fFrameIndex >= 0 && options.fFrameIndex < fEmbeddedCodecs->size()) {
        SkCodec* embeddedCodec = fEmbeddedCodecs->at(options.fFrameIndex).get();
        // Verify dimensions match
        if (embeddedCodec->dimensions() == dstInfo.dimensions()) {
            // Create a copy of options with fFrameIndex reset to 0 for the embedded codec.
            SkCodec::Options embeddedOptions = options;
            embeddedOptions.fFrameIndex = 0;

            SkCodec::Result result = embeddedCodec->startScanlineDecode(dstInfo, &embeddedOptions);
            if (result == kSuccess) {
                fCurrCodec = embeddedCodec;
            }
            return result;
        }
    }

    // Fall back to dimension-based codec selection (legacy behavior).
    int index = 0;
    SkCodec::Result result = kInvalidScale;
    while (true) {
        index = this->chooseCodec(dstInfo.dimensions(), index);
        if (index < 0) {
            break;
        }

        SkCodec* embeddedCodec = fEmbeddedCodecs->at(index).get();
        // Reset frame index for embedded codec
        SkCodec::Options embeddedOptions = options;
        embeddedOptions.fFrameIndex = 0;

        result = embeddedCodec->startScanlineDecode(dstInfo, &embeddedOptions);
        if (kSuccess == result) {
            fCurrCodec = embeddedCodec;
            return result;
        }

        index++;
    }

    SkCodecPrintf("Error: No matching candidate image in ico.\n");
    return result;
}

int SkIcoRustCodec::onGetScanlines(void* dst, int count, size_t rowBytes) {
    SkASSERT(fCurrCodec);
    return fCurrCodec->getScanlines(dst, count, rowBytes);
}

bool SkIcoRustCodec::onSkipScanlines(int count) {
    SkASSERT(fCurrCodec);
    return fCurrCodec->skipScanlines(count);
}

SkCodec::Result SkIcoRustCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
        void* pixels, size_t rowBytes, const SkCodec::Options& options) {
    // If a specific frame index is requested, use that embedded codec directly.
    if (options.fFrameIndex >= 0 && options.fFrameIndex < fEmbeddedCodecs->size()) {
        SkCodec* embeddedCodec = fEmbeddedCodecs->at(options.fFrameIndex).get();
        // Verify dimensions match
        if (embeddedCodec->dimensions() == dstInfo.dimensions()) {
            // Create a copy of options with fFrameIndex reset to 0 for the embedded codec.
            // Each embedded image is a separate codec with its own single frame (index 0).
            SkCodec::Options embeddedOptions = options;
            embeddedOptions.fFrameIndex = 0;

            SkCodec::Result result = embeddedCodec->startIncrementalDecode(
                    dstInfo, pixels, rowBytes, &embeddedOptions);
            if (result == kSuccess) {
                fCurrCodec = embeddedCodec;
                return kSuccess;
            }
            if (result == kUnimplemented) {
                // BMP doesn't support incremental decode. Try scanline decode instead.
                if (embeddedCodec->startScanlineDecode(dstInfo) == kSuccess) {
                    fCurrCodec = embeddedCodec;
                    return kUnimplemented;
                }
            }
            return result;
        }
    }

    // Fall back to dimension-based codec selection (legacy behavior).
    int index = 0;
    while (true) {
        index = this->chooseCodec(dstInfo.dimensions(), index);
        if (index < 0) {
            break;
        }

        SkCodec* embeddedCodec = fEmbeddedCodecs->at(index).get();
        switch (embeddedCodec->startIncrementalDecode(dstInfo, pixels, rowBytes, &options)) {
            case kSuccess:
                fCurrCodec = embeddedCodec;
                return kSuccess;
            case kUnimplemented:
                // BMP doesn't support incremental decode. Try scanline decode instead.
                if (embeddedCodec->startScanlineDecode(dstInfo) == kSuccess) {
                    return kUnimplemented;
                }
                break;
            default:
                break;
        }

        index++;
    }

    SkCodecPrintf("Error: No matching candidate image in ico.\n");
    return kInvalidScale;
}

SkCodec::Result SkIcoRustCodec::onIncrementalDecode(int* rowsDecoded) {
    SkASSERT(fCurrCodec);
    return fCurrCodec->incrementalDecode(rowsDecoded);
}

SkCodec::SkScanlineOrder SkIcoRustCodec::onGetScanlineOrder() const {
    if (fCurrCodec) {
        return fCurrCodec->getScanlineOrder();
    }

    return INHERITED::onGetScanlineOrder();
}

SkSampler* SkIcoRustCodec::getSampler(bool createIfNecessary) {
    if (fCurrCodec) {
        return fCurrCodec->getSampler(createIfNecessary);
    }

    return nullptr;
}
