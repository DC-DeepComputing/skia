/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCapture_DEFINED
#define SkCapture_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"

#include <vector>

struct SkCaptureMetadata {
    uint32_t version;
    uint32_t numPictures;
};

class SkCanvas;
class SkCaptureCanvas;

class SkCapture : public SkRefCnt {
public:
    // Centralized serialization constants
    static const char kMagic[];
    static const size_t kMagicSize = 8;
    static const uint32_t kVersion = 0; // Until this version is 1 or greater, active development
                                        // will make this unstable.

    static sk_sp<SkCapture> MakeFromData(sk_sp<SkData>);

private:
SkCaptureMetadata fMetadata;
std::vector<sk_sp<SkPicture>> fPictures; //TODO(b/412351769): Replace pictures with SkCapturePicture structs that also include picture metadata
};

#endif //SkCapture_DEFINED
