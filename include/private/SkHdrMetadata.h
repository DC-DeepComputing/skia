/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHdrMetadata_DEFINED
#define SkHdrMetadata_DEFINED

#include <optional>

#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"
class SkData;
class SkString;

namespace SkHdrMetadata {

/**
 * Content light level metadata.
 * The semantics of this metadata is defined in:
 * ANSI/CTA-861-H A DTV Profile for Uncompressed High Speed Digital Interfaces
 * Annex P Calculation of MaxCLL and MaxFALL
 */
struct ContentLightLevelInformation {
    float fMaxCLL = 0.f;
    float fMaxFALL = 0.f;

    /**
     * Decode from the binary encoding listed at:
     *   AV1 Bitstream & Decoding Process Specification Version 1.0.0 Errata 1
     *   https://aomediacodec.github.io/av1-spec/av1-spec.pdf
     *   5.8.3 Metadata high dynamic range content light level syntax
     * This encoding is equivalent to:
     *   ITU-T H.265 (V10) (07/2024)
     *   D.2.35 Content light level information SEI message syntax
     * Return false if parsing fails.
     */
    bool parse(const SkData* data);

    /**
     * Serialize to the encoding used by parse().
     */
    sk_sp<SkData> serialize() const;

    /**
     * Decode from the binary encoding listed at:
     *   Portable Network Graphics (PNG) Specification (Third Edition)
     *   11.3.2.8 cLLI Content Light Level Information
     * This encoding is not equivalent to the encoding used by parse().
     * Return false if parsing fails.
     */
    bool parsePngChunk(const SkData* data);

    /**
     * Serialize to the encoding used by parsePngChunk().
     */
    sk_sp<SkData> serializePngChunk() const;

    /**
     * Return a human-readable description.
     */
    SkString toString() const;

    bool operator==(const ContentLightLevelInformation& other) const;
    bool operator!=(const ContentLightLevelInformation& other) const {
        return !(*this == other);
    }
};

/**
 * Mastering display color volume metadata.
 * The semantics of this metadata is defined in:
 * SMPTE ST 2086:2018 Mastering Display Color Volume Metadata Supporting
 * High Luminance and Wide Color Gamut Images
 */
struct MasteringDisplayColorVolume {
    SkColorSpacePrimaries fDisplayPrimaries = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
    float fMaximumDisplayMasteringLuminance = 0.f;
    float fMinimumDisplayMasteringLuminance = 0.f;

    /**
     * The encoding as defined in:
     *   AV1 Bitstream & Decoding Process Specification Version 1.0.0 Errata 1
     *   https://aomediacodec.github.io/av1-spec/av1-spec.pdf
     *   5.8.4 Metadata high dynamic range mastering display color volume syntax
     * This encoding is equivalent to:
     *   ITU-T H.265 (V10) (07/2024)
     *   D.2.35 Content light level information SEI message syntax
     * This encoding is also equivalent to:
     *   Portable Network Graphics (PNG) Specification (Third Edition)
     *   11.3.2.7 mDCV Mastering Display Color Volume
     * Return false if parsing fails.
     */
    bool parse(const SkData* data);

    /**
     * Serialize to the encoding used by parse().
     */
    sk_sp<SkData> serialize() const;

    /**
     * Return a human-readable description.
     */
    SkString toString() const;

    bool operator==(const MasteringDisplayColorVolume& other) const;
    bool operator!=(const MasteringDisplayColorVolume& other) const {
        return !(*this == other);
    }
};

}  // namespace SkHdrMetadata

#endif
