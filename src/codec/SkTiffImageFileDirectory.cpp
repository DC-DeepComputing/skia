/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkTiffImageFileDirectory.h"

#include "include/core/SkData.h"
#include "src/codec/SkCodecPriv.h"

#undef SkCodecPrintf
#define SkCodecPrintf printf

constexpr uint16_t kTypeUnsignedByte = 1;
constexpr uint16_t kTypeAsciiString = 2;
constexpr uint16_t kTypeUnsignedShort = 3;
constexpr uint16_t kTypeUnsignedLong = 4;
constexpr uint16_t kTypeUnsignedRational = 5;
constexpr uint16_t kTypeSignedByte = 6;
constexpr uint16_t kTypeUndefined = 7;
constexpr uint16_t kTypeSignedShort = 8;
constexpr uint16_t kTypeSignedLong = 9;
constexpr uint16_t kTypeSignedRational = 10;
constexpr uint16_t kTypeSingleFloat = 11;
constexpr uint16_t kTypeDoubleFloat = 12;

constexpr size_t kSizeEntry = 12;
constexpr size_t kSizeShort = 2;
constexpr size_t kSizeLong = 4;

bool is_valid_type(uint16_t type) { return type >= 1 && type <= 12; }

size_t bytes_for_type(uint16_t type) {
    switch (type) {
        case kTypeUnsignedByte:
            return 1;
        case kTypeAsciiString:
            return 1;
        case kTypeUnsignedShort:
            return kSizeShort;
        case kTypeUnsignedLong:
            return kSizeLong;
        case kTypeUnsignedRational:
            return 8;
        case kTypeSignedByte:
            return 1;
        case kTypeUndefined:
            return 1;
        case kTypeSignedShort:
            return kSizeShort;
        case kTypeSignedLong:
            return kSizeLong;
        case kTypeSignedRational:
            return 8;
        case kTypeSingleFloat:
            return 4;
        case kTypeDoubleFloat:
            return 8;
    }
    return 0;
}

// Helper function for computing the address of an entry.
static const uint8_t* get_entry_address(const SkData* data,
                                        uint32_t ifdOffset,
                                        uint16_t entryIndex) {
    return data->bytes() +           // Base address
           ifdOffset +               // IFD offset
           kSizeShort +              // IFD number of entries
           kSizeEntry * entryIndex;  // Entries
}

// Return true if the IFD starting at |ifdOffset| contains valid number of entries (that doesn't
// overrun |data|).
static bool validate_ifd(const SkData* data,
                         bool littleEndian,
                         uint32_t ifdOffset,
                         uint16_t* outNumEntries,
                         uint32_t* outNextIfdOffset) {
    const uint8_t* dataCurrent = data->bytes();
    size_t dataSize = data->size();

    // Seek to the IFD offset.
    if (dataSize < ifdOffset) {
        SkCodecPrintf("IFD offset is too large.\n");
        return false;
    }
    dataCurrent += ifdOffset;
    dataSize -= ifdOffset;

    // Read the number of entries.
    if (dataSize < kSizeShort) {
        SkCodecPrintf("Insufficient space to store number of entries.\n");
        return false;
    }
    uint16_t numEntries = get_endian_short(dataCurrent, littleEndian);
    dataCurrent += kSizeShort;
    dataSize -= kSizeShort;

    // Check that there is enough space for all entries.
    if (dataSize < kSizeEntry * numEntries) {
        SkCodecPrintf("Insufficient space (%u) to store all %u entries.\n",
                      static_cast<uint32_t>(data->size()),
                      numEntries);
        return false;
    }

    // Save the number of entries.
    *outNumEntries = numEntries;

    // Seek past the entries.
    dataCurrent += kSizeEntry * numEntries;
    dataSize -= kSizeEntry * numEntries;

    // Read the next IFD offset.
    if (dataSize < kSizeLong) {
        SkCodecPrintf("Insufficient space to store next IFD offset.\n");
        return false;
    }
    uint32_t nextIfdOffset = get_endian_int(dataCurrent, littleEndian);

    // Save the next IFD offset.
    *outNextIfdOffset = nextIfdOffset;
    return true;
}

std::unique_ptr<SkTiffImageFileDirectory> SkTiffImageFileDirectory::Make(const sk_sp<SkData> data,
                                                                         bool littleEndian,
                                                                         uint32_t ifdOffset) {
    uint16_t numEntries = 0;
    uint32_t nextOffset = 0;
    if (!validate_ifd(data.get(), littleEndian, ifdOffset, &numEntries, &nextOffset)) {
        SkCodecPrintf("Failed to validate IFD.\n");
        return nullptr;
    }
    return std::unique_ptr<SkTiffImageFileDirectory>(
            new SkTiffImageFileDirectory(data, littleEndian, ifdOffset, numEntries, nextOffset));
}

std::unique_ptr<SkTiffImageFileDirectory> SkTiffImageFileDirectory::MakeFromIndirectOffset(
        const sk_sp<SkData> data, bool littleEndian, uint32_t ifdOffsetOffset) {
    if (data->size() < kSizeLong || data->size() - kSizeLong < ifdOffsetOffset) {
        SkCodecPrintf("Data too small to read IFD offset.\n");
    }
    uint32_t ifdOffset = get_endian_int(data->bytes() + ifdOffsetOffset, littleEndian);
    return Make(data, littleEndian, ifdOffset);
}

SkTiffImageFileDirectory::SkTiffImageFileDirectory(const sk_sp<SkData> data,
                                                   bool littleEndian,
                                                   uint32_t offset,
                                                   uint16_t numEntries,
                                                   uint32_t nextIfdOffset)
        : fData(std::move(data))
        , fLittleEndian(littleEndian)
        , fOffset(offset)
        , fNumEntries(numEntries)
        , fNextIfdOffset(nextIfdOffset) {}

uint16_t SkTiffImageFileDirectory::getEntryTag(uint16_t entryIndex) const {
    const uint8_t* entry = get_entry_address(fData.get(), fOffset, entryIndex);
    return get_endian_short(entry, fLittleEndian);
}

bool SkTiffImageFileDirectory::getEntryRawData(uint16_t entryIndex,
                                               uint16_t* outTag,
                                               uint16_t* outType,
                                               uint32_t* outCount,
                                               sk_sp<SkData>* outData) const {
    const uint8_t* entry = get_entry_address(fData.get(), fOffset, entryIndex);

    // Read the tag
    uint16_t tag = get_endian_short(entry, fLittleEndian);
    entry += kSizeShort;

    // Read the type.
    uint16_t type = get_endian_short(entry, fLittleEndian);
    entry += kSizeShort;
    if (!is_valid_type(type)) {
        return false;
    }

    // Read the count.
    uint32_t count = get_endian_int(entry, fLittleEndian);
    entry += kSizeLong;

    // If the entry fits in the remaining 4 bytes, use that.
    size_t entryDataBytes = bytes_for_type(type) * count;
    if (entryDataBytes <= kSizeLong) {
        *outData = SkData::MakeSubset(fData.get(), entry - fData->bytes(), entryDataBytes);
    } else {
        // Otherwise, the next 4 bytes specify an offset where the data can be found.
        uint32_t entryDataOffset = get_endian_int(entry, fLittleEndian);
        *outData = SkData::MakeSubset(fData.get(), entryDataOffset, entryDataBytes);
        if (*outData == nullptr) {
            return false;
        }
    }

    if (outTag) {
        *outTag = tag;
    }
    if (outType) {
        *outType = type;
    }
    if (outCount) {
        *outCount = count;
    }
    return true;
}

bool SkTiffImageFileDirectory::getEntryUnsignedShort(uint16_t entryIndex,
                                                     uint32_t count,
                                                     uint16_t* values) const {
    return getEntryValuesGeneric(entryIndex, kTypeUnsignedShort, count, values);
}

bool SkTiffImageFileDirectory::getEntryUnsignedLong(uint16_t entryIndex,
                                                    uint32_t count,
                                                    uint32_t* values) const {
    return getEntryValuesGeneric(entryIndex, kTypeUnsignedLong, count, values);
}

bool SkTiffImageFileDirectory::getEntrySignedRational(uint16_t entryIndex,
                                                      uint32_t count,
                                                      float* values) const {
    return getEntryValuesGeneric(entryIndex, kTypeSignedRational, count, values);
}

sk_sp<SkData> SkTiffImageFileDirectory::getEntryUndefinedData(uint16_t entryIndex) const {
    uint16_t type = 0;
    uint32_t count = 0;
    sk_sp<SkData> data;
    if (!getEntryRawData(entryIndex, nullptr, &type, &count, &data)) {
        return nullptr;
    }
    if (type != kTypeUndefined) {
        return nullptr;
    }
    return data;
}

bool SkTiffImageFileDirectory::getEntryValuesGeneric(uint16_t entryIndex,
                                                     uint16_t type,
                                                     uint32_t count,
                                                     void* values) const {
    uint16_t entryType = 0;
    uint32_t entryCount = 0;
    sk_sp<SkData> entryData;
    if (!getEntryRawData(entryIndex, nullptr, &entryType, &entryCount, &entryData)) {
        return false;
    }
    if (type != entryType) {
        return false;
    }
    if (count != entryCount) {
        return false;
    }
    for (uint32_t i = 0; i < count; ++i) {
        const uint8_t* data = entryData->bytes() + i * bytes_for_type(kTypeUnsignedLong);
        switch (type) {
            case kTypeUnsignedShort:
                reinterpret_cast<uint16_t*>(values)[i] = get_endian_short(data, fLittleEndian);
                break;
            case kTypeUnsignedLong:
                reinterpret_cast<uint32_t*>(values)[i] = get_endian_int(data, fLittleEndian);
                break;
            case kTypeSignedRational: {
                uint32_t numerator = get_endian_int(data, fLittleEndian);
                uint32_t denominator = get_endian_int(data + kSizeLong, fLittleEndian);
                reinterpret_cast<float*>(values)[i] = numerator / static_cast<float>(denominator);
                break;
            }
            default:
                SkCodecPrintf("Unsupported type %u\n", type);
                return false;
                break;
        }
    }
    return true;
}
