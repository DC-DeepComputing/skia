/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegMultiPicture.h"

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/base/SkEndian.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/codec/SkTiffUtility.h"
#include "src/core/SkStreamPriv.h"

#include <cstring>

constexpr size_t kMpEndianSize = 4;
constexpr uint8_t kMpBigEndian[kMpEndianSize] = {0x4D, 0x4D, 0x00, 0x2A};

constexpr uint16_t kTypeUnsignedLong = 0x4;
constexpr uint16_t kTypeUndefined = 0x7;

constexpr uint32_t kIfdEntrySize = 12;

constexpr uint16_t kVersionTag = 0xB000;
constexpr uint32_t kVersionCount = 4;
constexpr size_t kVersionSize = 4;
constexpr uint8_t kVersionExpected[kVersionSize] = {'0', '1', '0', '0'};

constexpr uint16_t kNumberOfImagesTag = 0xB001;
constexpr uint32_t kNumberOfImagesCount = 1;

constexpr uint16_t kMPEntryTag = 0xB002;
constexpr uint32_t kMPEntrySize = 16;

constexpr uint32_t kMPEntryAttributeFormatMask = 0x7000000;
constexpr uint32_t kMPEntryAttributeFormatJpeg = 0x0000000;

constexpr uint32_t kMPEntryAttributeTypeMask = 0xFFFFFF;
constexpr uint32_t kMPEntryAttributeTypePrimary = 0x030000;

constexpr uint16_t kIndividualImageUniqueIDTag = 0xB003;
constexpr uint32_t kIndividualImageUniqueIDSize = 33;

constexpr uint16_t kIndividualImageNumberTag = 0xB101;
constexpr uint32_t kIndividualImageNumberCount = 1;

constexpr uint16_t kTotalNumberCapturedFramesTag = 0xB004;

std::unique_ptr<SkJpegMultiPictureParameters> SkJpegMultiPictureParameters::Make(
        const sk_sp<const SkData>& segmentParameters) {
    // Read the MP Format identifier starting after the APP2 Field Length. See Figure 4 of CIPA
    // DC-x007-2009.
    if (segmentParameters->size() < sizeof(kMpfSig)) {
        return nullptr;
    }
    if (memcmp(segmentParameters->data(), kMpfSig, sizeof(kMpfSig)) != 0) {
        return nullptr;
    }
    auto ifdData = SkData::MakeSubset(
            segmentParameters.get(), sizeof(kMpfSig), segmentParameters->size() - sizeof(kMpfSig));
    SkASSERT(ifdData);

    // The rest of this function reads the structure described in Figure 6 of CIPA DC-x007-2009.
    // Create the Index Image File Directory (Index IFD).
    auto ifd = SkTiffImageFileDirectory::Make(ifdData);
    if (!ifd) {
        SkCodecPrintf("Failed to create MP Index IFD offset.\n");
        return nullptr;
    }

    // Read the number of tags in the Index IFD. See Table 3 (MP Index IFD Tags) for a description
    // of all possible tags.
    uint16_t tagCount = ifd->getNumEntries();

    // We will extract the number of images from the tags.
    uint32_t numberOfImages = 0;

    // The data for the MP entries.
    sk_sp<SkData> mpEntriesData;

    // The MP Index IFD tags shall be specified in the order of their tag IDs (text from
    // section 5.2.3), so keep track of the previous tag id read.
    uint16_t previousTag = 0;
    for (uint16_t idfEntryIndex = 0; idfEntryIndex < tagCount; ++idfEntryIndex) {
        uint16_t tag = ifd->getEntryTag(idfEntryIndex);
        if (previousTag >= tag) {
            SkCodecPrintf("MPF tags not in order.\n");
            return nullptr;
        }
        previousTag = tag;

        switch (tag) {
            case kVersionTag: {
                // See 5.2.3.1: MP Format Version.
                sk_sp<SkData> data = ifd->getEntryUndefinedData(idfEntryIndex);
                if (!data) {
                    SkCodecPrintf("Version must be undefined type.\n");
                    return nullptr;
                }
                if (data->size() != kVersionSize) {
                    SkCodecPrintf("Version must be 4 bytes.\n");
                    return nullptr;
                }
                if (memcmp(data->data(), kVersionExpected, kVersionSize) != 0) {
                    SkCodecPrintf("Version value is not 0100.\n");
                    return nullptr;
                }
                break;
            }
            case kNumberOfImagesTag:
                // See 5.2.3.2: Number of Images.
                if (!ifd->getEntryUnsignedLong(idfEntryIndex, 1, &numberOfImages)) {
                    SkCodecPrintf("Number of Images was not 1 unsigned long.\n");
                }
                if (numberOfImages < 1) {
                    SkCodecPrintf("Invalid number of images.\n");
                    return nullptr;
                }
                break;
            case kMPEntryTag: {
                // See 5.2.3.3: MP Entry.
                mpEntriesData = ifd->getEntryUndefinedData(idfEntryIndex);
                if (!mpEntriesData) {
                    SkCodecPrintf("MP entries data could not be extracted.\n");
                    return nullptr;
                }
                if (mpEntriesData->size() != kMPEntrySize * numberOfImages) {
                    SkCodecPrintf("MP entries data should be %ux%u bytes, was %u.\n",
                                  kMPEntrySize,
                                  numberOfImages,
                                  static_cast<uint32_t>(mpEntriesData->size()));
                    return nullptr;
                }
                break;
            }
            case kIndividualImageUniqueIDTag: {
                // See 5.2.3.4: Individual Image Unique ID List.
                // Validate that the count parameter is correct, but do not extract any other
                // information.
                sk_sp<SkData> data = ifd->getEntryUndefinedData(idfEntryIndex);
                if (!data) {
                    SkCodecPrintf("Image Unique ID must be undefined type.\n");
                    return nullptr;
                }
                if (data->size() != kIndividualImageUniqueIDSize * numberOfImages) {
                    SkCodecPrintf("Invalid Image Unique ID count.\n");
                    return nullptr;
                }
                break;
            }
            case kTotalNumberCapturedFramesTag: {
                // See 5.2.3.5: Total Number of Captured Frames.
                uint32_t totalNumCapturedFrames = 0;
                if (!ifd->getEntryUnsignedLong(idfEntryIndex, 1, &totalNumCapturedFrames)) {
                    SkCodecPrintf("Total Number of Captures Frames was not 1 unsigned long.\n");
                }
                break;
            }
            default:
                return nullptr;
        }
    }
    if (!numberOfImages) {
        SkCodecPrintf("Number of images must be greater than zero.\n");
        return nullptr;
    }
    if (!mpEntriesData) {
        SkCodecPrintf("MP Entry data was not present.\n");
        return nullptr;
    }

    // Start to prepare the result that we will return.
    auto result = std::make_unique<SkJpegMultiPictureParameters>(numberOfImages);

    // The next IFD is the Attribute IFD offset. We will not read or validate the Attribute IFD.

    // Parse the MP Entries data.
    for (uint32_t i = 0; i < numberOfImages; ++i) {
        const uint8_t* mpEntryData = mpEntriesData->bytes() + kMPEntrySize * i;
        const uint32_t attribute = get_endian_int(mpEntryData + 0, ifd->isLittleEndian());
        const uint32_t size = get_endian_int(mpEntryData + 4, ifd->isLittleEndian());
        const uint32_t dataOffset = get_endian_int(mpEntryData + 8, ifd->isLittleEndian());

        const bool isPrimary =
                (attribute & kMPEntryAttributeTypeMask) == kMPEntryAttributeTypePrimary;
        const bool isJpeg =
                (attribute & kMPEntryAttributeFormatMask) == kMPEntryAttributeFormatJpeg;

        if (isPrimary != (i == 0)) {
            SkCodecPrintf("Image must be primary iff it is the first image..\n");
            return nullptr;
        }
        if (!isJpeg) {
            SkCodecPrintf("Image format must be 0 (JPEG).\n");
            return nullptr;
        }

        if (i == 0 && dataOffset != 0) {
            SkCodecPrintf("First individual Image offset must be NULL.\n");
            return nullptr;
        }

        result->images[i].dataOffset = dataOffset;
        result->images[i].size = size;
    }

    return result;
}

sk_sp<SkData> SkJpegMultiPictureParameters::serialize(uint32_t individualImageNumber) const {
    SkDynamicMemoryWStream s;

    const uint32_t numberOfImages = static_cast<uint32_t>(images.size());

    // Write the MPF signature.
    s.write(kMpfSig, sizeof(kMpfSig));

    // We will always write as big-endian.
    s.write(kMpBigEndian, kMpEndianSize);

    // Set the first IFD offset be the position after the endianness value and this offset. This
    // will be the MP Index IFD for the first individual image and the MP Attribute IFD for all
    // other images.
    constexpr uint32_t firstIfdOffset = sizeof(kMpBigEndian) +  // Endian-ness
                                        sizeof(uint32_t);       // Index IFD offset
    SkWStreamWriteU32BE(&s, firstIfdOffset);
    SkASSERT(s.bytesWritten() - sizeof(kMpfSig) == firstIfdOffset);

    if (individualImageNumber == 0) {
        // The MP Index IFD will write 3 tags (version, number of images, and MP entries).
        const uint32_t mpIndexIfdNumberOfTags = 3;
        const uint32_t mpEntryOffset = sizeof(kMpBigEndian) +  // Endian-ness
                                       sizeof(uint32_t) +      // Index IFD offset
                                       sizeof(uint16_t) +      // Index IFD Number of tags
                                       kIfdEntrySize * mpIndexIfdNumberOfTags +  // Index IFD Tags
                                       sizeof(uint32_t);  // Attribute IFD offset.
        const uint32_t mpEntriesSize = kMPEntrySize * numberOfImages;
        const uint32_t mpAttributeIfdOffset = mpEntryOffset + mpEntriesSize;

        // The MP Attribute IFD will write 1 tag (individual image number).
        const uint16_t mpAttributeIfdNumberOfTags = 1;

        SkWStreamWriteU16BE(&s, mpIndexIfdNumberOfTags);

        // Write the version.
        SkWStreamWriteU16BE(&s, kVersionTag);
        SkWStreamWriteU16BE(&s, kTypeUndefined);
        SkWStreamWriteU32BE(&s, kVersionCount);
        s.write(kVersionExpected, kVersionSize);

        // Write the number of images.
        SkWStreamWriteU16BE(&s, kNumberOfImagesTag);
        SkWStreamWriteU16BE(&s, kTypeUnsignedLong);
        SkWStreamWriteU32BE(&s, kNumberOfImagesCount);
        SkWStreamWriteU32BE(&s, numberOfImages);

        // Write the MP entries.
        SkWStreamWriteU16BE(&s, kMPEntryTag);
        SkWStreamWriteU16BE(&s, kTypeUndefined);
        SkWStreamWriteU32BE(&s, mpEntriesSize);
        SkWStreamWriteU32BE(&s, mpEntryOffset);

        // Write the attribute IFD offset.
        SkWStreamWriteU32BE(&s, mpAttributeIfdOffset);

        // Write the MP entries.
        SkASSERT(s.bytesWritten() - sizeof(kMpfSig) == mpEntryOffset);
        for (size_t i = 0; i < images.size(); ++i) {
            const auto& image = images[i];

            uint32_t attribute = kMPEntryAttributeFormatJpeg;
            if (i == 0) {
                attribute |= kMPEntryAttributeTypePrimary;
            }

            SkWStreamWriteU32BE(&s, attribute);
            SkWStreamWriteU32BE(&s, image.size);
            SkWStreamWriteU32BE(&s, image.dataOffset);
            // Dependent image 1 and 2 entries are zero.
            SkWStreamWriteU16BE(&s, 0);
            SkWStreamWriteU16BE(&s, 0);
        }

        // Write MPF Attribute IFD. Make sure we are the offset that we wrote earlier.
        SkASSERT(s.bytesWritten() - sizeof(kMpfSig) == mpAttributeIfdOffset);

        // Write the MP Attribute IFD number of tags.
        SkWStreamWriteU16BE(&s, mpAttributeIfdNumberOfTags);

        // Write the individual image number.
        SkWStreamWriteU16BE(&s, kIndividualImageNumberTag);
        SkWStreamWriteU16BE(&s, kTypeUnsignedLong);
        SkWStreamWriteU32BE(&s, kIndividualImageNumberCount);
        SkWStreamWriteU32BE(&s, individualImageNumber);
    } else {
        // The MP Attribute IFD will write 2 tags: Both version version and individual image number.
        // See 5.2.4.1. MP Format Version: "Except for the First Individual Image, this tag shall
        // also be specified when specifying an MP Attribute IFD in any Individual Image".
        const uint16_t mpAttributeIfdNumberOfTags = 2;
        SkWStreamWriteU16BE(&s, mpAttributeIfdNumberOfTags);

        // Write the version tag.
        SkWStreamWriteU16BE(&s, kVersionTag);
        SkWStreamWriteU16BE(&s, kTypeUndefined);
        SkWStreamWriteU32BE(&s, kVersionCount);
        s.write(kVersionExpected, kVersionSize);
    }

    // Write the individual image number.
    SkWStreamWriteU16BE(&s, kIndividualImageNumberTag);
    SkWStreamWriteU16BE(&s, kTypeUnsignedLong);
    SkWStreamWriteU32BE(&s, kIndividualImageNumberCount);
    SkWStreamWriteU32BE(&s, individualImageNumber);

    // Write the next IFD offset (zero because there is none).
    SkWStreamWriteU32BE(&s, 0);

    return s.detachAsData();
}

size_t SkJpegMultiPictureParameters::GetAbsoluteOffset(uint32_t dataOffset,
                                                       size_t mpSegmentOffset) {
    // The value of zero is used by the primary image.
    if (dataOffset == 0) {
        return 0;
    }
    return mpSegmentOffset +                  // The offset to the marker
           kJpegMarkerCodeSize +              // The marker itself
           kJpegSegmentParameterLengthSize +  // The parameter length
           sizeof(kMpfSig) +                  // The signature
           dataOffset;
}

uint32_t SkJpegMultiPictureParameters::GetDataOffset(size_t absoluteOffset,
                                                     size_t mpSegmentOffset) {
    // The value of zero is used by the primary image.
    if (absoluteOffset == 0) {
        return 0;
    }
    return absoluteOffset -                   // The offset of the image itself
           mpSegmentOffset -                  // The offset to the marker
           kJpegMarkerCodeSize -              // The marker itself
           kJpegSegmentParameterLengthSize -  // The parameter length
           sizeof(kMpfSig);                   // The signature
}
