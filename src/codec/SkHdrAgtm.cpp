/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkHdrMetadata.h"
#include "src/codec/SkHdrAgtmPriv.h"
#include "src/core/SkStreamPriv.h"

#include <numbers>

namespace skhdr {

SkColor4f Agtm::ComponentMixingFunction::evaluate(const SkColor4f& c) const {
    // Assert that the parameters satisfy the constraints in clause 5.2.2.
    SkASSERT(0.f <= fRed        && fRed       <= 1.f);
    SkASSERT(0.f <= fGreen      && fGreen     <= 1.f);
    SkASSERT(0.f <= fBlue       && fBlue      <= 1.f);
    SkASSERT(0.f <= fMax        && fMax       <= 1.f);
    SkASSERT(0.f <= fMin        && fMin       <= 1.f);
    SkASSERT(0.f <= fComponent  && fComponent <= 1.f);
    SkASSERT(0.99999f <= fRed + fGreen + fBlue + fMax + fMin + fComponent);
    SkASSERT(1.00001f >= fRed + fGreen + fBlue + fMax + fMin + fComponent);

    // This implements that math in Formula 3 of SMPTE ST 2094-50.
    float common = fRed * c.fR + fGreen * c.fG + fBlue * c.fB  +
                   fMax * std::max(std::max(c.fR, c.fG), c.fB) +
                   fMin * std::min(std::max(c.fR, c.fG), c.fB);

    // Optimization for when all components are the same.
    if (fComponent == 0.f) {
        return {common, common, common, c.fA};
    }

    // Formula 4 of SMPTE ST 2094-50.
    return {fComponent * c.fR + common, fComponent * c.fG + common, fComponent * c.fB + common, c.fA};
}

float Agtm::PiecewiseCubicFunction::evaluate(float x) const {
    // This implements that math in Formula 1 of SMPTE ST 2094-50.
    SkASSERT(fNumControlPoints > 0 && fNumControlPoints <= 32);

    // Handle points off of the left endpoint.
    size_t i = 0;
    if (x <= fX[i]) {
        return fY[i];
    }

    // Handle points off of the right endpoint.
    size_t j = fNumControlPoints - 1;
    if (x >= fX[j]) {
        return fY[j] + std::log2(fX[j] / x);
    }

    // Binary search for i, j bracket in which we find x.
    while (j - i > 1) {
        size_t m = (i + j) / 2;
        if (x < fX[m]) {
            j = m;
        } else {
            i = m;
        }
    }

    // Cache short names for the parameters for computing the cubic coefficients.
    const float x_i = fX[i];
    const float y_i = fY[i];
    const float x_j = fX[j];
    const float y_j = fY[j];
    const float h_i = x_j - x_i;
    const float mHat_i = fM[i] * h_i;
    const float mHat_j = fM[j] * h_i;

    // Handle intervals that are a point.
    if (h_i == 0.f) {
        return y_i;
    }

    // Compute the coefficients and evaluate the polynomial.
    const float c3 =  2.f * y_i + mHat_i - 2.f * y_j + mHat_j;
    const float c2 = -3.f * y_i + 3.f * y_j - 2.f * mHat_i - mHat_j;
    const float c1 = mHat_i;
    const float c0 = y_i;
    const float t = (x - x_i) / h_i;

    return ((c3*t + c2)*t + c1)*t + c0;
}

SkColor4f Agtm::GainFunction::evaluate(const SkColor4f& c) const {
    SkColor4f m = fComponentMixing.evaluate(c);
    SkColor4f result = {0.f, 0.f, 0.f, c.fA};
    result.fR = fPiecewiseCubic.evaluate(m.fR);
    if (m.fR == m.fG && m.fG == m.fB) {
      result.fG = result.fR;
      result.fB = result.fR;
    } else {
      result.fG = fPiecewiseCubic.evaluate(m.fG);
      result.fB = fPiecewiseCubic.evaluate(m.fB);
    }
    return result;
}

void Agtm::PiecewiseCubicFunction::populateSlopeFromPCHIP() {
    size_t N = fNumControlPoints;

    // Compute the interval width (h) and piecewise linear slope (s).
    float s[Agtm::PiecewiseCubicFunction::kMaxNumControlPoints];
    float h[Agtm::PiecewiseCubicFunction::kMaxNumControlPoints];
    for (size_t i = 0; i < N - 1; ++i) {
        h[i] = fX[i+1] - fX[i];
    }
    for (size_t i = 0; i < N - 1; ++i) {
        s[i] = (fY[i+1] - fY[i]) / h[i];
    }

    // Handle the left and right control points.
    if (N >= 3) {
        // From Formulas 3 and 4 of ST 2094-50 candidate draft 2.
        fM[0]   = ((2 * h[0]   + h[1]  ) * s[0]   - h[0]   * s[1]  ) / (h[0]   + h[1]  );
        fM[N-1] = ((2 * h[N-2] + h[N-3]) * s[N-2] - h[N-2] * s[N-3]) / (h[N-2] + h[N-3]);
    } else if (N == 2) {
        fM[0]   = s[0];
        fM[N-1] = s[0];
    } else {
        fM[0]   = 0.f;
        fM[N-1] = 0.f;
    }

    // Populate internal control points.
    for (size_t i = 1; i <= N - 2; ++i) {
        // From Formula 5 of ST 2094-50 candidate draft 2.
        if (s[i-1] * s[i] < 0.f) {
            fM[i] = 0.f;
        } else {
            float num = 3 * (h[i-1] + h[i]) * s[i-1] * s[i];
            float den = (2 * h[i-1] + h[i]) * s[i-1] + (h[i-1] + 2 * h[i]) * s[i];
            fM[i] = num / den;
        }
    }
}

void Agtm::populateUsingRwtmo() {
   fToneMapType = AdaptiveToneMap::kReferenceWhite;
   fGainApplicationSpacePrimaries = SkNamedPrimaries::kRec2020;

    if (fBaselineHdrHeadroom == 0.f) {
        fNumAlternateImages = 0;
        return;
    }

    // Set the two alternate image headrooms using Formula D.1 from ST 2094-50 candidate draft 2.
    fNumAlternateImages = 2;
    fAlternateHdrHeadroom[0] = 0.f;
    fAlternateHdrHeadroom[1] =
        std::log2(8.f / 3.f) * std::min(fBaselineHdrHeadroom / std::log2(1000/203.f), 1.f);

    for (size_t a = 0; a <fNumAlternateImages; ++a) {
        fGainFunction[a] = GainFunction();

        // Use maxRGB for applying the curve.
        fGainFunction[a].fComponentMixing.fMax = 1.f;

        // Compute the image of white under the tone mapping from Formula D.2 from ST 2094-50
        // candidate draft 2.
        const float yWhite =
            (a == 1) ? 1.f
                     : 1.f - 0.5f * std::min(fBaselineHdrHeadroom / std::log2(1000/203.f), 1.f);

        // Compute the Bezier control points using Formula D.5 from ST 2094-50 candidate draft 2.
        const float kappa = 0.65f;
        const float xKnee = 1.f;
        const float yKnee = yWhite;
        const float xMax = std::exp2(fBaselineHdrHeadroom);
        const float yMax = std::exp2(fAlternateHdrHeadroom[a]);
        const float xMid = (1.f - kappa) * xKnee + kappa * (xKnee * yMax / yKnee);
        const float yMid = (1.f - kappa) * yKnee + kappa * yMax;

        // Compute the cubic coefficients using Formula D.5 from ST 2094-50 candidate draft 2.
        const float xA = xKnee - 2.f * xMid + xMax;
        const float yA = yKnee - 2.f * yMid + yMax;
        const float xB = 2.f * xMid - 2.f * xKnee;
        const float yB = 2.f * yMid - 2.f * yKnee;
        const float xC = xKnee;
        const float yC = yKnee;

        auto& cubic = fGainFunction[a].fPiecewiseCubic;
        cubic.fNumControlPoints = 8;
        for (size_t c = 0; c < cubic.fNumControlPoints; ++c) {
            // Compute the linear domain curve values using Formula D.4 from ST 2094-50 candidate
            // draft 2.
            const float t = c / (cubic.fNumControlPoints - 1.f);
            const float x = xC + t * (xB + t * xA);
            const float y = yC + t * (yB + t * yA);
            const float m = (2.f * yA * t + yB) / (2.f * xA * t + xB);

            // Compute the log domain curve values using Formula D.3 from ST 2094-50 candidate
            // draft 2.
            cubic.fX[c] = x;
            cubic.fY[c] = std::log2(y / x);
            cubic.fM[c] = (x * m - y) / (std::log(2.f) * x * y);
        }
    }
}

Agtm::Weighting Agtm::ComputeWeighting(float targetedHdrHeadroom) const {
    Weighting result;

    // Create the list of HDR headrooms including the baseline image and all alternate images, as
    // described in SMPTE ST 2094-50, clause 5.4.5, Computation of the adaptive tone map.

    // Let N be the length of the combined list.
    size_t N = 0;

    // Let H be the sorted list of HDR headrooms.
    float H[kMaxNumAlternateImages + 1];

    // Let indices list the index of each entry of H in fAlternateHdrHeadroom. The index for
    // fBaselineHdrHeadroom is Weighting::kInvalidIndex.
    size_t indices[kMaxNumAlternateImages + 1];
    for (size_t i = 0; i < fNumAlternateImages; ++i) {
        if (N == i && fBaselineHdrHeadroom < fAlternateHdrHeadroom[i]) {
            // Insert the baseline HDR headroom before the indices as they are visited.
            indices[N] = Weighting::kInvalidIndex;
            H[N++] = fBaselineHdrHeadroom;
        }
        indices[N] = i;
        H[N++] = fAlternateHdrHeadroom[i];
    }
    if (N == fNumAlternateImages) {
        // Insert the baseline HDR headroom at the end if it has not yet been inserted.
        indices[N] = Weighting::kInvalidIndex;
        H[N++] = fBaselineHdrHeadroom;
    }

    // Find the indices for the contributing images.
    if (targetedHdrHeadroom <= H[0]) {
        // The case of Formula 10 in SMPTE ST 2094-50.
        result.fWeight[0] = 1.f;
        result.fAlternateImageIndex[0] = indices[0];
    } else if (targetedHdrHeadroom >= H[N-1]) {
        // The case of Formula 11 in SMPTE ST 2094-50.
        result.fWeight[0] = 1.f;
        result.fAlternateImageIndex[0] = indices[N-1];
    } else {
        // The case of Formula 12 in SMPTE ST 2094-50.
        size_t i = 0;
        for (i = 0; i < N - 1; ++i) {
            if (H[i] <= targetedHdrHeadroom && targetedHdrHeadroom <= H[i+1]) {
                break;
            }
        }
        result.fWeight[0] = (targetedHdrHeadroom - H[i+1]) / (H[i] - H[i+1]);
        result.fWeight[1] = 1.f - result.fWeight[0];
        result.fAlternateImageIndex[0] = indices[i];
        result.fAlternateImageIndex[1] = indices[i+1];
    }

    // The baseline image always has a gain of 0, so set the weight for the baseline image to 0.
    for (size_t i = 0; i < 2; ++i) {
        if (result.fAlternateImageIndex[i] == Weighting::kInvalidIndex) {
            result.fWeight[i] = 0;
        } else if (result.fWeight[i] == 0) {
            result.fAlternateImageIndex[i] = Weighting::kInvalidIndex;
        }
    }

    // Sort the weights so that the first weight is always greater.
    if (result.fWeight[1] > result.fWeight[0]) {
        std::swap(result.fWeight[0], result.fWeight[1]);
        std::swap(result.fAlternateImageIndex[0], result.fAlternateImageIndex[1]);
    }

    return result;
}

// Return x clamped to [a, b].
template<typename T, typename U>
static T clamp(T x, U a, U b) {
  return std::min(std::max(x, static_cast<T>(a)), static_cast<T>(b));
}

// Helper class to read individual fields of a uint8_t bitfield.
class BitfieldReader {
  public:
    bool readFromStream(SkMemoryStream& s) {
        return s.readU8(&fBits);
    }
    uint8_t readBits(size_t bits) {
        // Read the topmost `bits` bits, then shift them off.
        uint8_t result = fBits >> (8 - bits);
        fBits <<= bits;
        fBitsRead += bits;
        return result;
    }
  private:
    uint8_t fBits = 0;
    size_t fBitsRead = 0;
};

// Helper class to write individual fields of a uint8_t bitfield.
class BitfieldWriter {
  public:
    void writeBits(uint8_t value, size_t bits) {
        // Ensure the value fits in `bits`.
        SkASSERT(value <= (1 << bits) - 1);
        fBits <<= bits;
        fBits |= value;
        fBitsWritten += bits;
    }
    void padAndWriteToStream(SkDynamicMemoryWStream& s) {
        // Write the remaining bits as zero, then write the result.
        SkASSERT(fBitsWritten <= 8);
        fBits <<= (8 - fBitsWritten);
        fBitsWritten = 8;
        s.write8(fBits);
    }
  private:
    uint8_t fBits = 0;
    size_t fBitsWritten = 0;
};

// Helper function to write a float as a big-endian uint16.
static void write_float_as_uint16(SkDynamicMemoryWStream& s, float f,
                                  uint16_t clamp_min, uint16_t clamp_max,
                                  uint16_t offset, float factor) {
    int32_t v = static_cast<int32_t>(std::lround(f * factor)) + offset;
    v = clamp(v, clamp_min, clamp_max);
    SkWStreamWriteU16BE(&s, v);
}

// Helper function to read a float as a big-endian uint16.
static bool read_float_as_uint16(SkMemoryStream& s, float& out,
                                 uint16_t clamp_min, uint16_t clamp_max,
                                 uint16_t offset, float factor) {
    uint16_t value = 0;
    if (!SkStreamReadU16BE(&s, &value)) {
        return false;
    }
    out = (static_cast<int32_t>(clamp(value, clamp_min, clamp_max)) - offset) / factor;
    return true;
}

// Wrapper to return false if the specified call returns false.
#define RETURN_ON_FALSE(x) \
    do { \
        if (!(x)) { \
            return false; \
        } \
    } while (0)

bool Agtm::parse(const SkData* data) {
    RETURN_ON_FALSE(data != nullptr);
    SkMemoryStream s(data->data(), data->size());

    // smpte_st_2094_50_application_info_v0
    // The value application_minor_version will only have an effect in future versions. Read it
    // and discard it.
    uint8_t application_minor_version = 0;
    RETURN_ON_FALSE(s.readU8(&application_minor_version));

    // smpte_st_2094_50_color_volume_transform
    BitfieldReader cvtFlags;
    RETURN_ON_FALSE(cvtFlags.readFromStream(s));
    const uint8_t has_hdr_reference_white_flag = cvtFlags.readBits(1);
    const uint8_t has_adaptive_tone_map_flag = cvtFlags.readBits(1);
    RETURN_ON_FALSE(cvtFlags.readBits(6) == 0);

    if (has_hdr_reference_white_flag == 1) {
        RETURN_ON_FALSE(read_float_as_uint16(s, fHdrReferenceWhite, 1u, 50000u, 0, 5.f));
    } else {
        fHdrReferenceWhite = 203.f;
    }
    if (has_adaptive_tone_map_flag == 0) {
        fToneMapType = AdaptiveToneMap::kNone;
        return true;
    }

    // smpte_st_2094_50_adaptive_tone_map()
    RETURN_ON_FALSE(read_float_as_uint16(s, fBaselineHdrHeadroom, 0u, 60000u, 0, 10000.f));

    BitfieldReader atmFlags;
    RETURN_ON_FALSE(atmFlags.readFromStream(s));
    const uint8_t use_reference_white_tone_mapping_flag = atmFlags.readBits(1);
    if (use_reference_white_tone_mapping_flag == 1) {
        RETURN_ON_FALSE(atmFlags.readBits(7) == 0);
        populateUsingRwtmo();
        fToneMapType = AdaptiveToneMap::kReferenceWhite;
        return true;
    } else {
        fToneMapType = AdaptiveToneMap::kCustom;
    }
    const uint8_t num_alternate_images = atmFlags.readBits(3);
    const uint8_t gain_application_space_primaries = atmFlags.readBits(2);
    const uint8_t has_common_component_mix_params_flag = atmFlags.readBits(1);
    const uint8_t has_common_cubic_params_flag = atmFlags.readBits(1);

    fNumAlternateImages = clamp(num_alternate_images, 0, 4);
    switch (gain_application_space_primaries) {
        case 0:
            fGainApplicationSpacePrimaries = SkNamedPrimaries::kRec709;
            break;
        case 1:
            fGainApplicationSpacePrimaries = SkNamedPrimaries::kSMPTE_EG_432_1;
            break;
        case 2:
            fGainApplicationSpacePrimaries = SkNamedPrimaries::kRec2020;
            break;
        case 3: {
            auto& p = fGainApplicationSpacePrimaries;
            RETURN_ON_FALSE(read_float_as_uint16(s, p.fRX, 0u, 50000u, 0, 50000.f));
            RETURN_ON_FALSE(read_float_as_uint16(s, p.fRY, 0u, 50000u, 0, 50000.f));
            RETURN_ON_FALSE(read_float_as_uint16(s, p.fGX, 0u, 50000u, 0, 50000.f));
            RETURN_ON_FALSE(read_float_as_uint16(s, p.fGY, 0u, 50000u, 0, 50000.f));
            RETURN_ON_FALSE(read_float_as_uint16(s, p.fBX, 0u, 50000u, 0, 50000.f));
            RETURN_ON_FALSE(read_float_as_uint16(s, p.fBY, 0u, 50000u, 0, 50000.f));
            RETURN_ON_FALSE(read_float_as_uint16(s, p.fWX, 0u, 50000u, 0, 50000.f));
            RETURN_ON_FALSE(read_float_as_uint16(s, p.fWY, 0u, 50000u, 0, 50000.f));
            break;
        }
    }

    // If `has_common_cubic_params_flag` is true then these will be used to save the parameters from
    // the first alternate images.
    uint8_t gain_curve_use_pchip_flag = 0;

    for (size_t a = 0; a < fNumAlternateImages; ++a) {
        // alternate_hdr_headrooms[a]
        RETURN_ON_FALSE(read_float_as_uint16(s, fAlternateHdrHeadroom[a], 0u, 60000u, 0, 10000.f));

        auto& mix = fGainFunction[a].fComponentMixing;
        if (a == 0 || has_common_component_mix_params_flag == 0) {
            // smpte_st_2094_50_component_mixing(a)
            mix = ComponentMixingFunction();
            BitfieldReader mixFlags;
            RETURN_ON_FALSE(mixFlags.readFromStream(s));
            uint8_t component_mixing_type = mixFlags.readBits(2);
            switch (component_mixing_type) {
                case 0:
                    mix.fMax = 1.f;
                    RETURN_ON_FALSE(mixFlags.readBits(6) == 0);
                    break;
                case 1:
                    mix.fComponent = 1.f;
                    RETURN_ON_FALSE(mixFlags.readBits(6) == 0);
                    break;
                case 2:
                    mix.fMax = 1.f / 2.f;
                    mix.fRed = mix.fGreen = mix.fBlue = 1.f / 6.f;
                    RETURN_ON_FALSE(mixFlags.readBits(6) == 0);
                    break;
                case 3:
                    if (mixFlags.readBits(1)) {
                        RETURN_ON_FALSE(
                            read_float_as_uint16(s, mix.fRed, 0u, 50000u, 0, 50000.f));
                    }
                    if (mixFlags.readBits(1)) {
                        RETURN_ON_FALSE(
                            read_float_as_uint16(s, mix.fGreen, 0u, 50000u, 0, 50000.f));
                    }
                    if (mixFlags.readBits(1)) {
                        RETURN_ON_FALSE(
                            read_float_as_uint16(s, mix.fBlue, 0u, 50000u, 0, 50000.f));
                    }
                    if (mixFlags.readBits(1)) {
                        RETURN_ON_FALSE(
                            read_float_as_uint16(s, mix.fMax, 0u, 50000u, 0, 50000.f));
                    }
                    if (mixFlags.readBits(1)) {
                        RETURN_ON_FALSE(
                            read_float_as_uint16(s, mix.fMin, 0u, 50000u, 0, 50000.f));
                    }
                    if (mixFlags.readBits(1)) {
                        RETURN_ON_FALSE(
                            read_float_as_uint16(s, mix.fComponent, 0u, 50000u, 0, 50000.f));
                    }
                    break;
            }
        } else {
            mix = fGainFunction[0].fComponentMixing;
        }

        // smpte_st_2094_50_gain_curve(a)
        auto& cubic = fGainFunction[a].fPiecewiseCubic;
        if (a == 0 || has_common_cubic_params_flag == 0) {
            BitfieldReader cubicFlags;
            RETURN_ON_FALSE(cubicFlags.readFromStream(s));
            const uint8_t gain_cubic_num_control_points_minus_1 = cubicFlags.readBits(5);
            gain_curve_use_pchip_flag = cubicFlags.readBits(1);
            RETURN_ON_FALSE(cubicFlags.readBits(2) == 0);

            cubic.fNumControlPoints = gain_cubic_num_control_points_minus_1 + 1;
            for (size_t c = 0; c < cubic.fNumControlPoints; ++c) {
                RETURN_ON_FALSE(read_float_as_uint16(s, cubic.fX[c], 0u, 64000u, 0, 1000.f));
            }
        } else {
            cubic.fNumControlPoints = fGainFunction[0].fPiecewiseCubic.fNumControlPoints;
            for (size_t c = 0; c < cubic.fNumControlPoints; ++c) {
                cubic.fX[c] = fGainFunction[0].fPiecewiseCubic.fX[c];
            }
        }
        for (size_t c = 0; c < cubic.fNumControlPoints; ++c) {
            RETURN_ON_FALSE(read_float_as_uint16(s, cubic.fY[c], 0u, 48000u, 24000, 1000.f));
        }
        if (gain_curve_use_pchip_flag) {
            cubic.populateSlopeFromPCHIP();
        } else {
            for (size_t c = 0; c < cubic.fNumControlPoints; ++c) {
                float theta = 0.f;
                RETURN_ON_FALSE(
                    read_float_as_uint16(s, theta, 1u, 35999u, 18000, 36000.f / std::numbers::pi));
                cubic.fM[c] = std::tan(theta);
            }
        }
    }

    return true;
}

static bool operator==(const SkColorSpacePrimaries& a, const SkColorSpacePrimaries& b) {
    return memcmp(&a, &b, sizeof(a)) == 0;
}

sk_sp<SkData> Agtm::serialize() const {
    SkDynamicMemoryWStream s;

    // application_minor_version
    s.write8(0);

    // smpte_st_2094_50_color_volume_transform
    const uint8_t has_hdr_reference_white_flag = 1;
    const uint8_t has_adaptive_tone_map_flag = fToneMapType == AdaptiveToneMap::kNone ? 0 : 1;

    BitfieldWriter cvtFlags;
    cvtFlags.writeBits(has_hdr_reference_white_flag, 1);
    cvtFlags.writeBits(has_adaptive_tone_map_flag, 1);
    cvtFlags.padAndWriteToStream(s);

    if (has_hdr_reference_white_flag) {
        write_float_as_uint16(s, fHdrReferenceWhite, 1u, 50000u, 0, 5.f);
    }
    if (has_adaptive_tone_map_flag == 0) {
        return s.detachAsData();
    }

    // smpte_st_2094_50_adaptive_tone_map
    write_float_as_uint16(s, fBaselineHdrHeadroom, 0u, 60000u, 0, 10000.f);
    const uint8_t use_reference_white_tone_mapping_flag =
        fToneMapType == AdaptiveToneMap::kReferenceWhite ? 1 : 0;
    BitfieldWriter atmFlags;
    atmFlags.writeBits(use_reference_white_tone_mapping_flag, 1);
    if (use_reference_white_tone_mapping_flag == 1) {
        atmFlags.padAndWriteToStream(s);
        return s.detachAsData();
    }

    if (fNumAlternateImages > kMaxNumAlternateImages) {
        return nullptr;
    }
    atmFlags.writeBits(fNumAlternateImages, 3);

    uint8_t gain_application_space_primaries = 3;
    if (fGainApplicationSpacePrimaries == SkNamedPrimaries::kRec709) {
        gain_application_space_primaries = 0;
    } else if (fGainApplicationSpacePrimaries == SkNamedPrimaries::kSMPTE_EG_432_1) {
        gain_application_space_primaries = 1;
    } else if (fGainApplicationSpacePrimaries == SkNamedPrimaries::kRec2020) {
        gain_application_space_primaries = 2;
    } else {
        gain_application_space_primaries = 3;
    }
    atmFlags.writeBits(gain_application_space_primaries, 2);

    const uint8_t has_common_component_mix_params_flag = 0;
    atmFlags.writeBits(has_common_component_mix_params_flag, 1);

    const uint8_t has_common_cubic_params_flag = 0;
    atmFlags.writeBits(has_common_cubic_params_flag, 1);
    atmFlags.padAndWriteToStream(s);

    if (gain_application_space_primaries == 3) {
        write_float_as_uint16(s, fGainApplicationSpacePrimaries.fRX, 0u, 50000u, 0, 50000.f);
        write_float_as_uint16(s, fGainApplicationSpacePrimaries.fRY, 0u, 50000u, 0, 50000.f);
        write_float_as_uint16(s, fGainApplicationSpacePrimaries.fGX, 0u, 50000u, 0, 50000.f);
        write_float_as_uint16(s, fGainApplicationSpacePrimaries.fGY, 0u, 50000u, 0, 50000.f);
        write_float_as_uint16(s, fGainApplicationSpacePrimaries.fBX, 0u, 50000u, 0, 50000.f);
        write_float_as_uint16(s, fGainApplicationSpacePrimaries.fBY, 0u, 50000u, 0, 50000.f);
        write_float_as_uint16(s, fGainApplicationSpacePrimaries.fWX, 0u, 50000u, 0, 50000.f);
        write_float_as_uint16(s, fGainApplicationSpacePrimaries.fWY, 0u, 50000u, 0, 50000.f);
    }

    for (size_t a = 0; a < fNumAlternateImages; ++a) {
        // alternate_hdr_headrooms[a]
        write_float_as_uint16(s, fAlternateHdrHeadroom[a], 0u, 60000u, 0, 10000.f);

        // smpte_st_2094_50_component_mixing(a)
        auto& mix = fGainFunction[a].fComponentMixing;
        if (mix.fRed == 0.f && mix.fGreen == 0.f && mix.fBlue == 0.f &&
            mix.fMax == 1.f && mix.fMin == 0.f && mix.fComponent == 0.f) {
            BitfieldWriter mixFlags;
            mixFlags.writeBits(0, 2);
            mixFlags.padAndWriteToStream(s);
        } else if (mix.fRed == 0.f && mix.fGreen == 0.f && mix.fBlue == 0.f &&
                   mix.fMax == 0.f && mix.fMin == 0.f && mix.fComponent == 1.f) {
            BitfieldWriter mixFlags;
            mixFlags.writeBits(1, 2);
            mixFlags.padAndWriteToStream(s);
        } else if (mix.fRed == 1.f/6.f && mix.fGreen == 1.f/6.f && mix.fBlue == 1.f/6.f &&
                   mix.fMax == 1.f/2.f && mix.fMin == 0.f && mix.fComponent == 1.f) {
            BitfieldWriter mixFlags;
            mixFlags.writeBits(2, 2);
            mixFlags.padAndWriteToStream(s);
        } else {
            BitfieldWriter mixFlags;
            mixFlags.writeBits(3, 2);
            mixFlags.writeBits(mix.fRed != 0.f, 1);
            mixFlags.writeBits(mix.fGreen != 0.f, 1);
            mixFlags.writeBits(mix.fBlue != 0.f, 1);
            mixFlags.writeBits(mix.fMax != 0.f, 1);
            mixFlags.writeBits(mix.fMin != 0.f, 1);
            mixFlags.writeBits(mix.fComponent != 0.f, 1);
            mixFlags.padAndWriteToStream(s);
            if (mix.fRed != 0.f) {
                write_float_as_uint16(s, mix.fRed, 0u, 50000u, 0, 50000.f);
            }
            if (mix.fGreen != 0.f) {
                write_float_as_uint16(s, mix.fGreen, 0u, 50000u, 0, 50000.f);
            }
            if (mix.fBlue != 0.f) {
                write_float_as_uint16(s, mix.fBlue, 0u, 50000u, 0, 50000.f);
            }
            if (mix.fMax != 0.f) {
                write_float_as_uint16(s, mix.fMax, 0u, 50000u, 0, 50000.f);
            }
            if (mix.fMin != 0.f) {
                write_float_as_uint16(s, mix.fMin, 0u, 50000u, 0, 50000.f);
            }
            if (mix.fComponent != 0.f) {
                write_float_as_uint16(s, mix.fComponent, 0u, 50000u, 0, 50000.f);
            }
        }

        // smpte_st_2094_50_gain_curve(a)
        auto& cubic = fGainFunction[a].fPiecewiseCubic;
        const uint8_t gain_curve_use_pchip_slope_flag = 0;
        BitfieldWriter curveFlags;
        if (cubic.fNumControlPoints < PiecewiseCubicFunction::kMinNumControlPoints ||
            cubic.fNumControlPoints > PiecewiseCubicFunction::kMaxNumControlPoints) {
            return nullptr;
        }
        curveFlags.writeBits(cubic.fNumControlPoints - 1, 5);
        curveFlags.writeBits(gain_curve_use_pchip_slope_flag, 1);
        curveFlags.padAndWriteToStream(s);
        for (size_t c = 0; c < cubic.fNumControlPoints; ++c) {
            write_float_as_uint16(s, cubic.fX[c], 0u, 64000u, 0, 1000.f);
        }
        for (size_t c = 0; c < cubic.fNumControlPoints; ++c) {
            write_float_as_uint16(s, cubic.fY[c], 0u, 48000u, 24000, 1000.f);
        }
        for (size_t c = 0; c < cubic.fNumControlPoints; ++c) {
            float theta = std::atan(cubic.fM[c]);
            write_float_as_uint16(s, theta, 1u, 35999u, 18000, 36000.f / std::numbers::pi);
        }
    }

    return s.detachAsData();
}

}  // namespace skhdr
