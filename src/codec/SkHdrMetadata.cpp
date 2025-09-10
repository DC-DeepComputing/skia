/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkHdrMetadata.h"
#include "src/core/SkStreamPriv.h"

namespace SkHdrMetadata {

SkString ContentLightLevelInformation::toString() const {
    return SkStringPrintf("{maxCLL:%f, maxFALL:%f}", fMaxCLL, fMaxFALL);
}

bool ContentLightLevelInformation::parse(const SkData* data) {
    if (data->size() != 4) {
        return false;
    }
    SkMemoryStream s(data->data(), data->size());

    uint16_t max_cll = 0;
    uint16_t max_fall = 0;
    if (!SkStreamReadU16BE(&s, &max_cll)) {
        return false;
    }
    if (!SkStreamReadU16BE(&s, &max_fall)) {
        return false;
    }

    fMaxCLL = max_cll;
    fMaxFALL = max_fall;
    return true;
}

sk_sp<SkData> ContentLightLevelInformation::serialize() const {
    SkDynamicMemoryWStream s;
    SkWStreamWriteU16BE(&s, std::llroundf(fMaxCLL));
    SkWStreamWriteU16BE(&s, std::llroundf(fMaxFALL));
    return s.detachAsData();
}

bool ContentLightLevelInformation::parsePngChunk(const SkData* data) {
    if (data->size() != 8) {
        return false;
    }
    SkMemoryStream s(data->data(), data->size());

    uint32_t max_cll_times_10000 = 0;
    uint32_t max_fall_times_10000 = 0;
    if (!SkStreamReadU32BE(&s, &max_cll_times_10000)) {
        return false;
    }
    if (!SkStreamReadU32BE(&s, &max_fall_times_10000)) {
        return false;
    }

    fMaxCLL = max_cll_times_10000 / 10000.f;
    fMaxFALL = max_fall_times_10000 / 10000.f;
    return true;
}

sk_sp<SkData> ContentLightLevelInformation::serializePngChunk() const {
    SkDynamicMemoryWStream s;
    SkWStreamWriteU32BE(&s, std::llroundf(fMaxCLL * 10000.f));
    SkWStreamWriteU32BE(&s, std::llroundf(fMaxFALL * 10000.f));
    return s.detachAsData();
}

bool ContentLightLevelInformation::operator==(const ContentLightLevelInformation& other) const {
    return fMaxCLL == other.fMaxCLL &&
           fMaxFALL == other.fMaxFALL;
}

SkString MasteringDisplayColorVolume::toString() const {
    return SkStringPrintf(
        "{red:[%1.8f,%1.8f], green:[%1.8f,%1.8f], blue:[%1.8f,%1.8f], white:[%1.8f,%1.8f], maxLum:%f, minLum:%f}",
        fDisplayPrimaries.fRX, fDisplayPrimaries.fRY,
        fDisplayPrimaries.fGX, fDisplayPrimaries.fGY,
        fDisplayPrimaries.fBX, fDisplayPrimaries.fBY,
        fDisplayPrimaries.fWX, fDisplayPrimaries.fWY,
        fMaximumDisplayMasteringLuminance,
        fMinimumDisplayMasteringLuminance);
}

bool MasteringDisplayColorVolume::parse(const SkData* data) {
    if (data->size() != 24) {
        return false;
    }
    SkMemoryStream s(data->data(), data->size());

    uint16_t chromaticities_times_50000[8];
    for (auto& chromaticity_times_50000 : chromaticities_times_50000) {
        if (!SkStreamReadU16BE(&s, &chromaticity_times_50000)) {
            return false;
        }
    }
    uint32_t max_luminance_times_10000 = 0;
    uint32_t min_luminance_times_10000 = 0;
    if (!SkStreamReadU32BE(&s, &max_luminance_times_10000)) {
        return false;
    }
    if (!SkStreamReadU32BE(&s, &min_luminance_times_10000)) {
        return false;
    }

    fDisplayPrimaries = SkColorSpacePrimaries({
        chromaticities_times_50000[0] / 50000.f,
        chromaticities_times_50000[1] / 50000.f,
        chromaticities_times_50000[2] / 50000.f,
        chromaticities_times_50000[3] / 50000.f,
        chromaticities_times_50000[4] / 50000.f,
        chromaticities_times_50000[5] / 50000.f,
        chromaticities_times_50000[6] / 50000.f,
        chromaticities_times_50000[7] / 50000.f,
    });
    fMaximumDisplayMasteringLuminance = max_luminance_times_10000 / 10000.f;
    fMinimumDisplayMasteringLuminance = min_luminance_times_10000 / 10000.f;
    return true;
}

sk_sp<SkData> MasteringDisplayColorVolume::serialize() const {
    SkDynamicMemoryWStream s;
    SkWStreamWriteU16BE(&s, std::llroundf(fDisplayPrimaries.fRX * 50000.f));
    SkWStreamWriteU16BE(&s, std::llroundf(fDisplayPrimaries.fRY * 50000.f));
    SkWStreamWriteU16BE(&s, std::llroundf(fDisplayPrimaries.fGX * 50000.f));
    SkWStreamWriteU16BE(&s, std::llroundf(fDisplayPrimaries.fGY * 50000.f));
    SkWStreamWriteU16BE(&s, std::llroundf(fDisplayPrimaries.fBX * 50000.f));
    SkWStreamWriteU16BE(&s, std::llroundf(fDisplayPrimaries.fBY * 50000.f));
    SkWStreamWriteU16BE(&s, std::llroundf(fDisplayPrimaries.fWX * 50000.f));
    SkWStreamWriteU16BE(&s, std::llroundf(fDisplayPrimaries.fWY * 50000.f));
    SkWStreamWriteU32BE(&s, std::llroundf(fMaximumDisplayMasteringLuminance * 10000.f));
    SkWStreamWriteU32BE(&s, std::llroundf(fMinimumDisplayMasteringLuminance * 10000.f));
    return s.detachAsData();
}

bool MasteringDisplayColorVolume::operator==(const MasteringDisplayColorVolume& other) const {
    return fDisplayPrimaries.fRX == other.fDisplayPrimaries.fRX &&
           fDisplayPrimaries.fRY == other.fDisplayPrimaries.fRY &&
           fDisplayPrimaries.fGX == other.fDisplayPrimaries.fGX &&
           fDisplayPrimaries.fGY == other.fDisplayPrimaries.fGY &&
           fDisplayPrimaries.fBX == other.fDisplayPrimaries.fBX &&
           fDisplayPrimaries.fBY == other.fDisplayPrimaries.fBY &&
           fDisplayPrimaries.fWX == other.fDisplayPrimaries.fWX &&
           fDisplayPrimaries.fWY == other.fDisplayPrimaries.fWY &&
           fMaximumDisplayMasteringLuminance == other.fMaximumDisplayMasteringLuminance &&
           fMinimumDisplayMasteringLuminance == other.fMinimumDisplayMasteringLuminance;
}

}  // namespace SkHdrMetadata

