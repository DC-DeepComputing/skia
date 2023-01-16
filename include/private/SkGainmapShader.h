/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGainmapShader_DEFINED
#define SkGainmapShader_DEFINED

#include "include/core/SkRefCnt.h"

class SkImage;
class SkColorSpace;
class SkShader;
struct SkGainmapInfo;
struct SkSamplingOptions;
struct SkRect;

/**
 *  Return an SkShader that will draw a gainmapped image.
 */
sk_sp<SkShader> SK_API SkDrawImageWithGainmapShader(sk_sp<SkImage> sdrImage,
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
