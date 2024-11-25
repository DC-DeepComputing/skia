/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPngEncoderImpl_DEFINED
#define SkPngEncoderImpl_DEFINED

#include <cstdint>

#include "src/encode/SkImageEncoderFns.h"
#include "src/encode/SkPngEncoderBase.h"

#include <memory>

struct SkEncodedInfo;
class SkPixmap;
class SkPngEncoderMgr;
template <typename T> class SkSpan;

class SkPngEncoderImpl final : public SkPngEncoderBase {
public:
    // public so it can be called from SkPngEncoder namespace. It should only be made
    // via SkPngEncoder::Make
    SkPngEncoderImpl(const SkEncodedInfo& dstInfo,
                     transform_scanline_proc,
                     std::unique_ptr<SkPngEncoderMgr>,
                     const SkPixmap& src);
    ~SkPngEncoderImpl() override;

protected:
    bool onEncodeRow(SkSpan<uint8_t> row) override;
    bool onFinishEncoding() override;

    std::unique_ptr<SkPngEncoderMgr> fEncoderMgr;
};
#endif
