/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkExif.h"

#include "include/core/SkData.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkTiffImageFileDirectory.h"

SkExifMetadata::SkExifMetadata(sk_sp<SkData> data) : fData(std::move(data)) {
    // Read the endianness (4 bytes) and IFD offset (4 bytes).
    if (fData->size() < 8) {
        SkCodecPrintf("Exif data must be at least 8 bytes.\n");
        return;
    }
    bool littleEndian = false;
    if (!is_valid_endian_marker(fData->bytes(), &littleEndian)) {
        SkCodecPrintf("Exif data had invalid endian marker 0x%x,0x%x,0x%x,0x%x.\n",
                      fData->bytes[0],
                      fData->bytes[1],
                      fData->bytes[2],
                      fData->bytes[3]);
        return;
    }
    uint32_t ifdOffset = get_endian_int(fData->bytes() + 4, littleEndian);
    parseIfd(ifdOffset, littleEndian, /*isRoot=*/true);
}

void SkExifMetadata::parseIfd(uint32_t ifdOffset, bool littleEndian, bool isRoot) {
    auto ifd = SkTiffImageFileDirectory::Make(fData, littleEndian, ifdOffset);
    if (!ifd) {
        SkCodecPrintf("Failed to make IFD\n");
        return;
    }
    for (uint32_t i = 0; i < ifd->getNumEntries(); ++i) {
        switch (ifd->getEntryTag(i)) {
            case kOriginTag: {
                uint16_t value = 0;
                if (!fOriginPresent && ifd->getEntryUnsignedShort(i, 1, &value)) {
                    if (0 < value && value <= kLast_SkEncodedOrigin) {
                        fOriginValue = static_cast<SkEncodedOrigin>(value);
                        fOriginPresent = true;
                    }
                }
                break;
            }
            case kSubIFDOffsetTag: {
                uint32_t subIfdOffset = 0;
                if (isRoot && ifd->getEntryUnsignedLong(i, 1, &subIfdOffset)) {
                    parseIfd(subIfdOffset, littleEndian, /*isRoot=*/false);
                }
                break;
            }
            default:
                break;
        }
    }
}
