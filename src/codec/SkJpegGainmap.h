/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegGainmap_codec_DEFINED
#define SkJpegGainmap_codec_DEFINED

/*
 * Internal representation of gainmap parameter, to be plumbed to AndroidGainmapInfo.
 */
struct SkJpegGainmapInfoInternal {
    // The interval in log space that the gainmap is mapped into.
    float logRatioMin = -1.f;
    float logRatioMax = 1.f;
};

#endif
