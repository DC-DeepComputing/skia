/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrBitmap_DEFINED
#define GrBitmap_DEFINED

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkMipmap.h"

#include <optional>

class SkMipmap;

// A wrapper for an immutable SkBitmap that includes ganesh-specific things, such as mipmaps.
class GrBitmap {
public:
    explicit GrBitmap(SkBitmap b) : fBitmap(b) {}
    explicit GrBitmap(SkBitmap b, sk_sp<const SkMipmap> m) : fBitmap(b), fMips(m) {}

    ~GrBitmap() = default;
    GrBitmap(const GrBitmap&) = default;
    GrBitmap(GrBitmap&&) = default;
    GrBitmap& operator=(const GrBitmap&) = default;
    GrBitmap& operator=(GrBitmap&&) = default;

    SkAlphaType alphaType() const { return fBitmap.alphaType(); }
    SkBitmap bitmap() const { return fBitmap; }
    SkColorType colorType() const { return fBitmap.colorType(); }
    sk_sp<const SkMipmap> mips() const { return fMips; }

    using ReleaseProc = void(void* pixels, void* context);

    static std::optional<GrBitmap> Make(
            SkImageInfo, const void* pixels, size_t rowBytes, ReleaseProc, void* context);
    static std::optional<GrBitmap> Make(SkImageInfo ii, const void* pixels, size_t rowBytes) {
        return Make(ii, pixels, rowBytes, nullptr, nullptr);
    }
    static std::optional<GrBitmap> Make(const SkPixmap& p) {
        return Make(p.info(), p.addr(), p.rowBytes());
    }

private:
    SkBitmap fBitmap;
    sk_sp<const SkMipmap> fMips;
};

#endif
