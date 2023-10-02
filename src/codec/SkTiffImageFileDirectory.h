/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTiffImageFileDirectory_codec_DEFINED
#define SkTiffImageFileDirectory_codec_DEFINED

#include "include/core/SkRefCnt.h"

#include <cstdint>
#include <memory>
#include <vector>

class SkData;

/*
 * Helper function for parsing a Tiff Image File Directory (IFD) structure. This structure is used
 * by EXIF tags, multi-picture, and maker note metadata.
 */
class SkTiffImageFileDirectory {
public:
    /*
     * Create an object for parsing an IFD at offset |ifdOffset| inside |data| which has endianness
     * indicated by |littleEndian|.
     */
    static std::unique_ptr<SkTiffImageFileDirectory> Make(const sk_sp<SkData> data,
                                                          bool littleEndian,
                                                          uint32_t ifdOffset);
    /*
     * Read the IFD offset into |data| from |ifdOffsetOffset| and parse the resulting IFD.
     */
    static std::unique_ptr<SkTiffImageFileDirectory> MakeFromIndirectOffset(
            const sk_sp<SkData> data, bool littleEndian, uint32_t ifdOffsetOffset);

    /*
     * Return the number of entries.
     */
    uint16_t getNumEntries() const { return fNumEntries; }

    /*
     * Return the offset (within the specified SkData) of the next IFD in the list of IFDs.
     */
    uint32_t nextIfdOffset() const { return fNextIfdOffset; }

    /*
     * Return the tag, of a specific entry.
     */
    uint16_t getEntryTag(uint16_t entryIndex) const;

    /*
     * If |entryIndex| has type unsigned short (3), unsigned long (4), or signed rational (10), and
     * count |count|, then populate |values| with the data for the tag and return true. Otherwise
     * return false.
     */
    bool getEntryUnsignedShort(uint16_t entryIndex, uint32_t count, uint16_t* values) const;
    bool getEntryUnsignedLong(uint16_t entryIndex, uint32_t count, uint32_t* values) const;
    bool getEntrySignedRational(uint16_t entryIndex, uint32_t count, float* values) const;

    /*
     * If |entryIndex| has type undefined (7), then return the bytes specified by the count field
     * and the offset (read from the value field as an unsigned long).
     */
    sk_sp<SkData> getEntryUndefinedData(uint16_t entryIndex) const;

private:
    SkTiffImageFileDirectory(const sk_sp<SkData> data,
                             bool littleEndian,
                             uint32_t offset,
                             uint16_t ifdNumEntries,
                             uint32_t ifdNextOffset);

    /*
     * Return the tag, type, count, and data for the specified entry. Return false if the type
     * is invalid, or if the data in the IFD is out of bounds.
     */
    bool getEntryRawData(uint16_t entryIndex,
                         uint16_t* outTag,
                         uint16_t* outType,
                         uint32_t* outCount,
                         sk_sp<SkData>* outData) const;

    /*
     * Helper function for assorted getTag functions.
     */
    bool getEntryValuesGeneric(uint16_t entryIndex,
                               uint16_t type,
                               uint32_t count,
                               void* values) const;

    // The data that the IFD indexes into.
    const sk_sp<SkData> fData;

    // True if the data is little endian.
    const bool fLittleEndian;

    // The offset where the IFD starts.
    const uint32_t fOffset;

    // The number of entries of the IFD (read from the first 2 bytes at the IFD offset).
    const uint16_t fNumEntries;

    // The offset of the next IFD (read from the next 4 bytes after the IFD entries).
    const uint32_t fNextIfdOffset;
};

#endif
