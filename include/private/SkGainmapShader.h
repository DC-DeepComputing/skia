/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGainmapShader_DEFINED
#define SkGainmapShader_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"

class GrRecordingContext;
class SkImage;
class SkColorSpace;
class SkShader;
struct SkGainmapInfo;
struct SkSamplingOptions;
struct SkRect;

/**
 *  Return the SkImage that should be used to render the specified gainmap-based image, given a
 *  desired HDR ratio for the resulting image.
 *
 *  If the gainmap should not be applied at all, return the SDR image directly.
 *
 *  If resultColorInfo is specified, use that to create the resulting SkImage. If not specified,
 *  then select an efficient color space for the resulting image.
 */
sk_sp<SkImage> SK_API SkGainmapGetHdrImage(GrRecordingContext*,
                                           sk_sp<SkImage> sdrImage,
                                           sk_sp<SkImage> gainmapImage,
                                           const SkGainmapInfo& gainmapInfo,
                                           float resultHdrRatio,
                                           SkColorInfo resultColorInfo = SkColorInfo());

/**
 *  Return an SkShader that will draw a gainmapped image.
 */
sk_sp<SkShader> SK_API SkGainmapGetShader(sk_sp<SkImage> sdrImage,
                                          const SkRect& sdrRect,
                                          const SkSamplingOptions& sdrSamplingOptions,
                                          sk_sp<SkImage> gainmapImage,
                                          const SkRect& gainmapRect,
                                          const SkSamplingOptions& gainmapSamplingOptions,
                                          const SkGainmapInfo& gainmapInfo,
                                          const SkRect& dstRect,
                                          sk_sp<SkColorSpace> dstColorSpace,
                                          float dstHdrRatio);

#endif
