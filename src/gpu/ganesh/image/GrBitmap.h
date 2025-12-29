/*
 * Copyright 2025 Google LLC
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

namespace skgpu::ganesh {

// A wrapper for SkBitmap that includes ganesh-specific things, such as mipmaps.
class Bitmap {
public:
    explicit Bitmap(SkBitmap b) : fBitmap(b) {}
    explicit Bitmap(SkBitmap b, sk_sp<const SkMipmap> m) : fBitmap(b), fMips(m) {}

    ~Bitmap() = default;
    Bitmap(const Bitmap&) = default;
    Bitmap(Bitmap&&) = default;
    Bitmap& operator=(const Bitmap&) = default;
    Bitmap& operator=(Bitmap&&) = default;

    SkAlphaType alphaType() const { return fBitmap.alphaType(); }
    SkBitmap bitmap() const { return fBitmap; }
    SkColorType colorType() const { return fBitmap.colorType(); }
    sk_sp<const SkMipmap> mips() const { return fMips; }

    using ReleaseProc = void(void* pixels, void* context);

    static std::optional<Bitmap> Make(
            SkImageInfo, const void* pixels, size_t rowBytes, ReleaseProc, void* context);
    static std::optional<Bitmap> Make(SkImageInfo ii, const void* pixels, size_t rowBytes) {
        return Make(ii, pixels, rowBytes, nullptr, nullptr);
    }
    static std::optional<Bitmap> Make(const SkPixmap& p) {
        return Make(p.info(), p.addr(), p.rowBytes());
    }

private:
    SkBitmap fBitmap;
    sk_sp<const SkMipmap> fMips;
};

}  // namespace skgpu::ganesh

#endif
