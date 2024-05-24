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
constexpr uint32_t kIfdSerializedEntryCount = 3;

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
    // Determine the endianness of the values in the structure. See Figure 5 (MP endian tag
    // structure), and read the Index IFD offset.
    bool littleEndian = false;
    uint32_t ifdOffset = 0;
    if (!SkTiffImageFileDirectory::ParseHeader(ifdData.get(), &littleEndian, &ifdOffset)) {
        SkCodecPrintf("Failed to parse endian-ness and index IFD offset.\n");
        return nullptr;
    }

    // Create the Index Image File Directory (Index IFD).
    auto ifd = SkTiffImageFileDirectory::MakeFromOffset(ifdData, littleEndian, ifdOffset);
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
        const uint32_t attribute = get_endian_int(mpEntryData + 0, littleEndian);
        const uint32_t size = get_endian_int(mpEntryData + 4, littleEndian);
        const uint32_t dataOffset = get_endian_int(mpEntryData + 8, littleEndian);

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

// Return the number of bytes that will be written by SkJpegMultiPictureParametersSerialize, for a
// given number of images.
static void write_mpf_header(SkDynamicMemoryWStream& s,
                             uint32_t numberOfImages,
                             uint32_t numberOfTags) {
    // Write the MPF signature.
    s.write(kMpfSig, sizeof(kMpfSig));

    // We will always write as big-endian.
    s.write(kMpBigEndian, kMpEndianSize);

    // Set the Index IFD offset be the position after the endianness value and this offset.
    constexpr uint32_t indexIfdOffset =
            static_cast<uint16_t>(sizeof(kMpBigEndian) + sizeof(uint32_t));
    SkWStreamWriteU32BE(&s, indexIfdOffset);

    // We will write 2 or 3 tags (version, number of images, and optionally MP entries).
    SkASSERT(numberOfTags == 2 || numberOfTags == 3);
    SkWStreamWriteU16BE(&s, numberOfTags);

    // Write the version tag.
    SkWStreamWriteU16BE(&s, kVersionTag);
    SkWStreamWriteU16BE(&s, kTypeUndefined);
    SkWStreamWriteU32BE(&s, kVersionCount);
    s.write(kVersionExpected, kVersionSize);

    // Write the number of images.
    SkWStreamWriteU16BE(&s, kNumberOfImagesTag);
    SkWStreamWriteU16BE(&s, kTypeUnsignedLong);
    SkWStreamWriteU32BE(&s, kNumberOfImagesCount);
    SkWStreamWriteU32BE(&s, numberOfImages);
}

sk_sp<SkData> SkJpegMultiPictureParameters::serializeFirstIndividualImage() const {
    SkDynamicMemoryWStream s;

    // We will write 3 tags (version, number of images, MP entries).
    const uint32_t numberOfImages = static_cast<uint32_t>(images.size());
    constexpr uint32_t numberOfTags = 3;

    // Write the header, version, and number of images tags.
    write_mpf_header(s, numberOfImages, numberOfTags);

    // Write the MP entries.
    SkWStreamWriteU16BE(&s, kMPEntryTag);
    SkWStreamWriteU16BE(&s, kTypeUndefined);
    SkWStreamWriteU32BE(&s, kMPEntrySize * numberOfImages);
    const uint32_t mpEntryOffset =
            static_cast<uint32_t>(s.bytesWritten() -  // The bytes written so far
                                  sizeof(kMpfSig) +   // Excluding the MPF signature
                                  sizeof(uint32_t) +  // The 4 bytes for this offset
                                  sizeof(uint32_t));  // The 4 bytes for the attribute IFD offset.
    SkWStreamWriteU32BE(&s, mpEntryOffset);

    // Write the attribute IFD offset (zero because we don't write it).
    SkWStreamWriteU32BE(&s, 0);

    // Write the MP entries.
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

    return s.detachAsData();
}

sk_sp<SkData> SkJpegMultiPictureParameters::serializeNonFirstIndividualImage() const {
    SkDynamicMemoryWStream s;

    // We will write 2 tags (version, number of images).
    const uint32_t numberOfImages = static_cast<uint32_t>(images.size());
    constexpr uint32_t numberOfTags = 2;

    // Write the header, version, and number of images tags.
    write_mpf_header(s, numberOfImages, numberOfTags);

    // Write the attribute IFD offset (zero because we don't write it).
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
