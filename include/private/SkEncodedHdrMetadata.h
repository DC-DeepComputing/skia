/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEncodedHdrMetadata_DEFINED
#define SkEncodedHdrMetadata_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"
class SkData;

/**
 * Container for all HDR metadata that can be attached to an image or video.
 * Metadata is stored encoded form.
 */
struct SK_API SkEncodedHdrMetadata {
  /**
   * Content light level metadata.
   * The semantics of this metadata is defined in:
   *   ANSI/CTA-861-H
   *   A DTV Profile for Uncompressed High Speed Digital Interfaces
   *   Annex P Calculation of MaxCLL and MaxFALL
   * This is encoded as defined in:
   *   AV1 Bitstream & Decoding Process Specification Version 1.0.0 Errata 1
   *   https://aomediacodec.github.io/av1-spec/av1-spec.pdf
   *   5.8.3 Metadata high dynamic range content light level syntax
   * This encoding is equivalent to:
   *   ITU-T H.265 (V10) (07/2024)
   *   D.2.35 Content light level information SEI message syntax
   * This encoding is *not* equivalent to
   *   Portable Network Graphics (PNG) Specification (Third Edition)
   *   11.3.2.8 cLLI Content Light Level Information
   */
  sk_sp<SkData> fContentLightLevelInformation;

  /**
   * Mastering display color volume metadata.
   * The semantics of this metadata is defined in:
   *   SMPTE ST 2086:2018, Mastering Display Color Volume Metadata Supporting
   *   High Luminance and Wide Color Gamut Images
   * The encoding as defined in:
   *   AV1 Bitstream & Decoding Process Specification Version 1.0.0 Errata 1
   *   https://aomediacodec.github.io/av1-spec/av1-spec.pdf
   *   5.8.4 Metadata high dynamic range mastering display color volume syntax
   * This encoding is equivalent to:
   *   ITU-T H.265 (V10) (07/2024)
   *   D.2.35 Content light level information SEI message syntax
   * This encoding is *not* equivalent to
   *   Portable Network Graphics (PNG) Specification (Third Edition)
   *   11.3.2.7 mDCV Mastering Display Color Volume
   */
  sk_sp<SkData> fMasteringDisplayColorVolume;
};

#endif
