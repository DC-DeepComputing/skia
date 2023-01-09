/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegGainmap_codec_DEFINED
#define SkJpegGainmap_codec_DEFINED

struct SkJpegGainmapInfoInternal {
    float logratioMinValue = -1.f;
    float logratioMaxValue = 1.f;
};

#endif
