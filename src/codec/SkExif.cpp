/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkExif.h"

#include "include/core/SkData.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkTiffUtility.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <utility>

constexpr uint16_t kSubIFDOffsetTag = 0x8769;
constexpr uint16_t kMarkerNoteTag = 0x927c;

static bool get_maker_note_hdr_headroom(sk_sp<SkData> data, std::optional<float>& hdrHeadroom) {
    // No little endian images that specify this data have been observed. Do not add speculative
    // support.
    const bool kLittleEndian = false;
    const uint8_t kSig[] = {
            'A', 'p', 'p', 'l', 'e', ' ', 'i', 'O', 'S', 0, 0, 1, 'M', 'M',  //
    };
    if (!data || data->size() < sizeof(kSig)) {
        return false;
    }
    if (memcmp(data->data(), kSig, sizeof(kSig)) != 0) {
        return false;
    }
    auto ifd =
            SkTiffImageFileDirectory::MakeFromOffset(std::move(data), kLittleEndian, sizeof(kSig));
    if (!ifd) {
        return false;
    }

    // See documentation at:
    // https://developer.apple.com/documentation/appkit/images_and_pdf/applying_apple_hdr_effect_to_your_photos
    bool hasMaker33 = false;
    bool hasMaker48 = false;
    float maker33 = 0.f;
    float maker48 = 0.f;
    for (uint32_t i = 0; i < ifd->getNumEntries(); ++i) {
        switch (ifd->getEntryTag(i)) {
            case 33:
                if (!hasMaker33) {
                    hasMaker33 = ifd->getEntrySignedRational(i, 1, &maker33);
                }
                break;
            case 48:
                if (!hasMaker48) {
                    hasMaker48 = ifd->getEntrySignedRational(i, 1, &maker48);
                }
                break;
            default:
                break;
        }
    }
    // Many images have a maker33 but not a maker48. Treat them as having maker48 of 0.
    if (!hasMaker33) {
        return false;
    }
    float stops = 0.f;
    if (maker33 < 1.0f) {
        if (maker48 <= 0.01f) {
            stops = -20.0f * maker48 + 1.8f;
        } else {
            stops = -0.101f * maker48 + 1.601f;
        }
    } else {
        if (maker48 <= 0.01f) {
            stops = -70.0f * maker48 + 3.0f;
        } else {
            stops = -0.303f * maker48 + 2.303f;
        }
    }
    hdrHeadroom = std::pow(2.f, std::max(stops, 0.f));
    return true;
}

static void parse_ifd(SkExifMetadata& exif,
                      sk_sp<SkData> data,
                      std::unique_ptr<SkTiffImageFileDirectory> ifd,
                      bool littleEndian,
                      bool isRoot) {
    if (!ifd) {
        return;
    }
    for (uint32_t i = 0; i < ifd->getNumEntries(); ++i) {
        switch (ifd->getEntryTag(i)) {
            case SkExifMetadata::kOriginTag: {
                uint16_t value = 0;
                if (!exif.fOrigin.has_value() && ifd->getEntryUnsignedShort(i, 1, &value)) {
                    if (0 < value && value <= kLast_SkEncodedOrigin) {
                        exif.fOrigin = static_cast<SkEncodedOrigin>(value);
                    }
                }
                break;
            }
            case kMarkerNoteTag:
                if (!exif.fHdrHeadroom.has_value()) {
                    if (auto makerNoteData = ifd->getEntryUndefinedData(i)) {
                        get_maker_note_hdr_headroom(std::move(makerNoteData), exif.fHdrHeadroom);
                    }
                }
                break;
            case kSubIFDOffsetTag: {
                uint32_t subIfdOffset = 0;
                if (isRoot && ifd->getEntryUnsignedLong(i, 1, &subIfdOffset)) {
                    auto subIfd = SkTiffImageFileDirectory::MakeFromOffset(
                            std::move(data), littleEndian, subIfdOffset, /*allowTruncated=*/true);
                    parse_ifd(exif,
                              data,
                              std::move(subIfd),
                              littleEndian,
                              /*isRoot=*/false);
                }
                break;
            }
            case SkExifMetadata::kXResolutionTag: {
                float value = 0.f;
                if (!exif.fXResolution.has_value() && ifd->getEntryUnsignedRational(i, 1, &value)) {
                    exif.fXResolution = value;
                }
                break;
            }
            case SkExifMetadata::kYResolutionTag: {
                float value = 0.f;
                if (!exif.fYResolution.has_value() && ifd->getEntryUnsignedRational(i, 1, &value)) {
                    exif.fYResolution = value;
                }
                break;
            }
            case SkExifMetadata::kResolutionUnitTag: {
                uint16_t value = 0;
                if (!exif.fResolutionUnit.has_value() && ifd->getEntryUnsignedShort(i, 1, &value)) {
                    exif.fResolutionUnit = value;
                }
                break;
            }
            case SkExifMetadata::kPixelXDimensionTag: {
                // The type for this tag can be unsigned short or unsigned long (as per the Exif 2.3
                // spec, aka CIPA DC-008-2012). Support for unsigned long was added in
                // https://crrev.com/817600.
                uint16_t value16 = 0;
                if (!exif.fPixelXDimension.has_value() &&
                    ifd->getEntryUnsignedShort(i, 1, &value16)) {
                    exif.fPixelXDimension = value16;
                }
                uint32_t value32 = 0;
                if (!exif.fPixelXDimension.has_value() &&
                    ifd->getEntryUnsignedLong(i, 1, &value32)) {
                    exif.fPixelXDimension = value32;
                }
                break;
            }
            case SkExifMetadata::kPixelYDimensionTag: {
                uint16_t value16 = 0;
                if (!exif.fPixelYDimension.has_value() &&
                    ifd->getEntryUnsignedShort(i, 1, &value16)) {
                    exif.fPixelYDimension = value16;
                }
                uint32_t value32 = 0;
                if (!exif.fPixelYDimension.has_value() &&
                    ifd->getEntryUnsignedLong(i, 1, &value32)) {
                    exif.fPixelYDimension = value32;
                }
                break;
            }
            default:
                break;
        }
    }
}

void SkExifMetadata::parse(sk_sp<SkData> data) {
    bool littleEndian = false;
    uint32_t ifdOffset = 0;
    if (SkTiffImageFileDirectory::ParseHeader(data.get(), &littleEndian, &ifdOffset)) {
        auto ifd = SkTiffImageFileDirectory::MakeFromOffset(
                data, littleEndian, ifdOffset, /*allowTruncated=*/true);
        parse_ifd(*this, std::move(data), std::move(ifd), littleEndian, /*isRoot=*/true);
    }
}
