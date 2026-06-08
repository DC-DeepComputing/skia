/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_ico/decoder/impl/SkIcoRustCodec.h"

#include "experimental/rust_bmp/decoder/impl/SkBmpRustCodec.h"
#include "experimental/rust_ico/ffi/FFI.rs.h"
#include "src/codec/SkPngRustCodec.h"
#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkTemplates.h"
#include "rust/common/SkStreamAdapter.h"
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
    // Create a memory stream from the peeked buffer data
    auto data = SkData::MakeWithoutCopy(buffer, bytesRead);
    auto stream = std::make_unique<SkMemoryStream>(std::move(data));
    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(stream.get());
    return rust_ico::is_ico(std::move(inputAdapter));
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

    // ICO parsing requires random access to detect PNG vs BMP for each entry,
    // so we read everything into a memory stream that we can seek within.
    std::unique_ptr<SkMemoryStream> dataStream;
    if (stream->getMemoryBase()) {
        // Stream already has memory base - create a copy we can seek within
        dataStream = SkMemoryStream::MakeCopy(stream->getMemoryBase(), stream->getLength());
    } else {
        // Read all data from stream into memory
        sk_sp<SkData> data = SkStreamPriv::CopyStreamToData(stream.get());
        if (!data) {
            SkCodecPrintf("Error: CopyStreamToData returned null.\n");
            *result = kIncompleteInput;
            return nullptr;
        }
        dataStream = SkMemoryStream::Make(std::move(data));
    }

    if (!dataStream || dataStream->getLength() == 0) {
        SkCodecPrintf("Error: ICO stream is empty.\n");
        *result = kIncompleteInput;
        return nullptr;
    }

    const size_t totalSize = dataStream->getLength();

    // Parse ICO directory using Rust via stream adapter
    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(dataStream.get());
    rust::Box<rust_ico::DirectoryResult> directoryResult =
            rust_ico::parse_directory(std::move(inputAdapter));

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

    // Default Result, if no valid embedded codecs are found.
    *result = kInvalidInput;

    // Structure to hold codec and its metadata for sorting
    struct CodecEntry {
        std::unique_ptr<SkCodec> codec;
        uint16_t bitCount;
        sk_sp<SkData> bmpEntryData;  // Raw entry data for BMP (nullptr for PNG)
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
            actualSize = totalSize - offset;
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
        if (offset >= totalSize) {
            SkCodecPrintf("Warning: could not skip to ico offset.\n");
            break;
        }
        bytesRead = offset;

        if (offset + size > totalSize) {
            // Clamp size to available data
            size = totalSize - offset;
        }

        // Create a stream for the entry by copying the embedded data
        // Seek to the entry offset and read the entry data
        dataStream->seek(offset);
        auto entryStream = std::make_unique<SkMemoryStream>(
                SkData::MakeFromStream(dataStream.get(), size));
        bytesRead += size;

        // Create codec for the embedded image.
        // PNG entries delegate to SkPngRustCodec for full color profile and EXIF support.
        // BMP entries delegate to SkBmpRustCodec with ICO-aware metadata parsing
        // (no file header, halved height, alpha channel via image crate).
        Result entryResult;
        std::unique_ptr<SkCodec> codec;
        sk_sp<SkData> bmpData;
        if (entry.format == rust_ico::EmbeddedFormat::Png) {
            codec = SkPngRustCodec::MakeFromStream(std::move(entryStream), &entryResult);
        } else {
            // Keep a copy of the raw entry data for AND mask post-processing
            bmpData = SkData::MakeWithCopy(entryStream->getMemoryBase(), entryStream->getLength());
            codec = SkBmpRustCodec::MakeFromIcoStream(std::move(entryStream), &entryResult);
        }

        if (nullptr != codec) {
            // Store codec with its bit count for sorting
            entries.push_back({
                std::move(codec),
                entry.bit_count,
                std::move(bmpData)
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

    // Move the sorted entries into the single per-image container.  Bundling the
    // codec with its AND-mask payload keeps the two from drifting out of sync.
    std::vector<EmbeddedImage> embeddedImages;
    embeddedImages.reserve(entries.size());
    for (auto& entry : entries) {
        embeddedImages.push_back({std::move(entry.codec), std::move(entry.bmpEntryData)});
    }

    // Use the first (largest) codec's info
    auto maxInfo = embeddedImages.front().fCodec->getEncodedInfo().copy();

    // Signal incomplete input if some embedded images couldn't be decoded.
    // This lets callers (e.g. Blink) retry when more data arrives.
    *result = (embeddedImages.size() < numImages) ? kIncompleteInput : kSuccess;
    return std::unique_ptr<SkCodec>(
            new SkIcoRustCodec(std::move(maxInfo), std::move(stream),
                               std::move(embeddedImages)));
}

SkIcoRustCodec::SkIcoRustCodec(SkEncodedInfo&& info,
                               std::unique_ptr<SkStream> stream,
                               std::vector<EmbeddedImage> embeddedImages)
        // The source skcms_PixelFormat will not be used. The embedded
        // codec's will be used instead.
        : INHERITED(std::move(info), skcms_PixelFormat(), std::move(stream))
        , fEmbeddedImages(std::move(embeddedImages))
        , fCurrCodec(nullptr) {
    // Initialize frame holder with info from all embedded codecs
    // Use the largest image dimensions as the screen size
    int maxWidth = 0, maxHeight = 0;
    for (const EmbeddedImage& image : fEmbeddedImages) {
        SkImageInfo imgInfo = image.fCodec->getInfo();
        if (imgInfo.width() > maxWidth) maxWidth = imgInfo.width();
        if (imgInfo.height() > maxHeight) maxHeight = imgInfo.height();
    }
    fFrameHolder.setScreenSize(maxWidth, maxHeight);

    // Derive one frame entry per embedded image from the same container, so the
    // frame count can never disagree with the number of codecs.
    for (int i = 0; i < static_cast<int>(fEmbeddedImages.size()); i++) {
        SkImageInfo imgInfo = fEmbeddedImages[i].fCodec->getInfo();
        SkEncodedInfo::Alpha alpha = fEmbeddedImages[i].fCodec->getEncodedInfo().alpha();
        fFrameHolder.appendFrame(i, imgInfo.width(), imgInfo.height(), alpha);
    }
}

/*
 * Returns the number of embedded images (frames) in this ICO.
 * Each embedded image is exposed as a separate frame.
 */
int SkIcoRustCodec::onGetFrameCount() {
    return static_cast<int>(fEmbeddedImages.size());
}

/*
 * Returns frame info for the embedded image at the given index.
 */
bool SkIcoRustCodec::onGetFrameInfo(int index, FrameInfo* info) const {
    if (index < 0 || index >= static_cast<int>(fEmbeddedImages.size())) {
        return false;
    }

    if (info) {
        const SkCodec* embeddedCodec = fEmbeddedImages[index].fCodec.get();
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

int SkIcoRustCodec::chooseCodec(const SkISize& requestedSize, int startIndex) {
    SkASSERT(startIndex >= 0);

    for (int i = startIndex; i < static_cast<int>(fEmbeddedImages.size()); i++) {
        if (fEmbeddedImages[i].fCodec->dimensions() == requestedSize) {
            return i;
        }
    }

    return -1;
}

/*
 * Common codec selection logic shared by onGetPixels and onStartIncrementalDecode.
 * See selectAndDecode declaration in the header for details.
 */
template <typename Fn>
SkCodec::Result SkIcoRustCodec::selectAndDecode(
        const SkISize& dims, const Options& opts, Fn fn) {
    // Each embedded image is a separate codec with its own single frame (index 0),
    // so reset fFrameIndex when delegating.
    Options embeddedOpts = opts;
    embeddedOpts.fFrameIndex = 0;

    // If a specific frame index is requested, use that embedded codec directly.
    if (opts.fFrameIndex >= 0 && opts.fFrameIndex < static_cast<int>(fEmbeddedImages.size())) {
        SkCodec* codec = fEmbeddedImages[opts.fFrameIndex].fCodec.get();
        if (codec->dimensions() == dims) {
            return fn(codec, opts.fFrameIndex, embeddedOpts);
        }
    }

    // Fall back to dimension-based codec selection.
    int index = 0;
    Result lastResult = kInvalidScale;
    while (true) {
        index = this->chooseCodec(dims, index);
        if (index < 0) {
            break;
        }

        lastResult = fn(fEmbeddedImages[index].fCodec.get(), index, embeddedOpts);
        if (lastResult == kSuccess || lastResult == kIncompleteInput) {
            return lastResult;
        }

        index++;
    }

    SkCodecPrintf("Error: No matching candidate image in ico.\n");
    return lastResult;
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

    return selectAndDecode(dstInfo.dimensions(), opts,
        [&](SkCodec* codec, int codecIndex, const Options& embeddedOpts) -> Result {
            Result result = codec->getPixels(dstInfo, dst, dstRowBytes, &embeddedOpts);
            if (result == kSuccess || result == kIncompleteInput) {
                // Apply AND mask for BMP entries (non-32-bit BMPs have a transparency mask)
                if (codecIndex < static_cast<int>(fEmbeddedImages.size()) &&
                    fEmbeddedImages[codecIndex].fBmpEntryData) {
                    const auto& entryData = fEmbeddedImages[codecIndex].fBmpEntryData;
                    uint32_t bpp = dstInfo.bytesPerPixel();
                    rust::Slice<uint8_t> pixelSlice(
                            static_cast<uint8_t*>(dst),
                            static_cast<size_t>(dstInfo.height()) * dstRowBytes);
                    rust::Slice<const uint8_t> dataSlice(
                            static_cast<const uint8_t*>(entryData->data()),
                            entryData->size());
                    rust_ico::apply_and_mask(pixelSlice, dataSlice,
                                            dstInfo.width(), dstInfo.height(), bpp);
                }
                *rowsDecoded = dstInfo.height();
            }
            return result;
        });
}

SkCodec::Result SkIcoRustCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
        void* pixels, size_t rowBytes, const SkCodec::Options& options) {
    return selectAndDecode(dstInfo.dimensions(), options,
        [&](SkCodec* codec, int codecIndex, const Options& embeddedOpts) -> Result {
            Result r = codec->startIncrementalDecode(
                    dstInfo, pixels, rowBytes, &embeddedOpts);
            if (r == kSuccess) {
                fCurrCodec = codec;
                fCurrCodecIndex = codecIndex;
                fIncrementalDst = pixels;
                fIncrementalRowBytes = rowBytes;
                fIncrementalDstInfo = dstInfo;
            }
            return r;
        });
}

SkCodec::Result SkIcoRustCodec::onIncrementalDecode(int* rowsDecoded) {
    SkASSERT(fCurrCodec);
    Result result = fCurrCodec->incrementalDecode(rowsDecoded);
    if (result == kSuccess || result == kIncompleteInput) {
        // Apply AND mask for BMP entries after incremental decode completes
        if (fCurrCodecIndex >= 0 &&
            fCurrCodecIndex < static_cast<int>(fEmbeddedImages.size()) &&
            fEmbeddedImages[fCurrCodecIndex].fBmpEntryData) {
            const auto& entryData = fEmbeddedImages[fCurrCodecIndex].fBmpEntryData;
            uint32_t bpp = fIncrementalDstInfo.bytesPerPixel();
            rust::Slice<uint8_t> pixelSlice(
                    static_cast<uint8_t*>(fIncrementalDst),
                    static_cast<size_t>(fIncrementalDstInfo.height()) * fIncrementalRowBytes);
            rust::Slice<const uint8_t> dataSlice(
                    static_cast<const uint8_t*>(entryData->data()),
                    entryData->size());
            rust_ico::apply_and_mask(pixelSlice, dataSlice,
                                    fIncrementalDstInfo.width(),
                                    fIncrementalDstInfo.height(), bpp);
        }
    }
    return result;
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
