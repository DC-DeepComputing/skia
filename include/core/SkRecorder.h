/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRecorder_DEFINED
#define SkRecorder_DEFINED

#include "include/core/SkPicture.h"
#include "include/private/SkAPI.h"
#include "include/private/SkTArray.h"

class SkCanvas;
class SkSurface;
class SkRecorderPriv;

class SK_API SkRecorder : public SkRefCnt {
public:
    SkRecorder() = default;
    virtual ~SkRecorder() = default;
    SkRecorder(const SkRecorder&) = delete;
    SkRecorder(SkRecorder&&) = delete;
    SkRecorder& operator=(const SkRecorder&) = delete;

    enum class Type {
        kCPU,
        kGanesh,
        kGraphite,
    };

    Type type() const { return fType; }

    SkRecorderPriv priv();

protected:
    skia_private::TArray<sk_sp<SkPicture>> fCapturedPictures;

private:
    sk_sp<SkSurface> makeSurface(SkImageInfo info,
                                 size_t rowBytes,
                                 const SkSurfaceProps* props = nullptr);

    Type fType;

    friend class SkRecorderPriv;
    friend class SkSurface_Base; // for capture functionality
};

#endif
