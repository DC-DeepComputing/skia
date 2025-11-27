/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkEncodedInfo.h"

#include "modules/skcms/skcms.h"

std::unique_ptr<SkEncodedInfo::ICCProfile> SkEncodedInfo::ICCProfile::Make(
        sk_sp<const SkData> data) {
    if (data) {
        skcms_ICCProfile profile;
        if (skcms_Parse(data->data(), data->size(), &profile)) {
            return sk_sp<ColorProfile>(new ColorProfile(profile, std::move(data)));
        }
    }
    return nullptr;
}

sk_sp<SkEncodedInfo::ColorProfile> SkEncodedInfo::ColorProfile::Make(sk_sp<SkColorSpace> cs) {
    SkASSERT_RELEASE(cs);
    skcms_ICCProfile csProfile;
    cs->toProfile(&csProfile);
    return sk_sp<ColorProfile>(new ColorProfile(csProfile));
}

sk_sp<SkEncodedInfo::ColorProfile> SkEncodedInfo::ColorProfile::MakeCICP(
        uint8_t color_primaries,
        uint8_t transfer_characteristics,
        uint8_t matrix_coefficients,
        uint8_t full_range_flag) {
    skcms_ICCProfile skcmsProfile;
    skcms_Init(&skcmsProfile);
    skcmsProfile.has_CICP = true;
    skcmsProfile.CICP.color_primaries = color_primaries;
    skcmsProfile.CICP.transfer_characteristics = transfer_characteristics;
    skcmsProfile.CICP.matrix_coefficients = matrix_coefficients;
    skcmsProfile.CICP.video_full_range_flag = full_range_flag;
    return sk_sp<ColorProfile>(new ColorProfile(skcmsProfile));
}

uint32_t SkEncodedInfo::ColorProfile::getDataColorSpace() const {
    return fProfile.data_color_space;
}

bool SkEncodedInfo::ColorProfile::needsColorConversionTo(SkColorSpace* cs) const {
    SkASSERT_RELEASE(cs);
    skcms_ICCProfile csProfile;
    cs->toProfile(&csProfile);
    return !skcms_ApproximatelyEqualProfiles(&fProfile, &csProfile);
}

SkEncodedInfo::ColorProfile::ColorProfile(const skcms_ICCProfile& profile, sk_sp<const SkData> data)
    : fProfile(profile)
    , fData(std::move(data))
{}
