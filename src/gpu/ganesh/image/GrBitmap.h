/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrBitmap_DEFINED
#define GrBitmap_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkPixelRef.h"

#include <memory>

class SkMipmap;

namespace skgpu::ganesh {

// PixelView?
class Bitmap {
public:
    ~Bitmap();

    sk_sp<const SkMipmap> mips(); // const { return fMips; }
    SkAlphaType alphaType() const;
    SkColorType colorType() const;

    using ReleaseProc = void(void* pixels, void* context);

    static std::unique_ptr<Bitmap> Make(SkImageInfo, const void* pixels, size_t rowBytes, ReleaseProc, void* context);
    static std::unique_ptr<Bitmap> Make(SkImageInfo ii, const void* pixels, size_t rowBytes) {
        return Make(ii, pixels, rowBytes, nullptr, nullptr);
    }
    static std::unique_ptr<Bitmap> Make(const SkPixmap& p) {
        return Make(p.info(), p.addr(), p.rowBytes());
    }

    static std::unique_ptr<Bitmap> MakeAndOwn(SkImageInfo, sk_sp<const SkPixelRef>);
    static std::unique_ptr<Bitmap> MakeAndOwn(SkBitmap& b) {
        SkPixelRef* pixelRef = b.pixelRef();
        if (!pixelRef) {
            return Make(b.pixmap());
        }
        b.setImmutable();
        return MakeAndOwn(b.info(), sk_ref_sp<const SkPixelRef>(pixelRef));
    }

    // #include "src/gpu/ganesh/image/GrBitmap.h"
    // auto bitmap = skgpu::ganesh::Bitmap::Make();
    // static std::unique_ptr<Bitmap> TODO();

private:
    Bitmap();
    sk_sp<const SkMipmap> fMips;
};

} // namespace skgpu::ganesh

#endif