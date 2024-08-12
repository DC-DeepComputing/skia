/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngCodecBase_DEFINED
#define SkPngCodecBase_DEFINED

#include <cstdint>
#include <memory>
#include <optional>

#include "include/codec/SkCodec.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTemplates.h"

class SkColorPalette;
class SkSampler;
class SkStream;
class SkSwizzler;
enum class SkEncodedImageFormat;
struct SkEncodedInfo;
struct SkImageInfo;
template <typename T> class SkSpan;

// This class implements functionality shared between `SkPngCodec` and
// `SkPngRustCodec` (the latter is from `experimental/rust_png`).
class SkPngCodecBase : public SkCodec {
public:
    ~SkPngCodecBase() override;

protected:
    SkPngCodecBase(SkEncodedInfo&&, std::unique_ptr<SkStream>);

    Result initializeXforms(const SkImageInfo& dstInfo, const Options& options);
    void applyXformRow(void* dst, const void* src);
    const SkSwizzler* swizzler() const { return fSwizzler.get(); }

    struct PaletteColorEntry {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };
    virtual std::optional<SkSpan<const PaletteColorEntry>> onTryGetPlteChunk() = 0;
    virtual std::optional<SkSpan<const uint8_t>> onTryGetTrnsChunk() = 0;

private:
    // SkCodec overrides:
    SkEncodedImageFormat onGetEncodedFormat() const final;
    SkSampler* getSampler(bool createIfNecessary) override;

    void allocateStorage(const SkImageInfo& dstInfo);
    void initializeXformParams();
    void initializeSwizzler(const SkImageInfo& dstInfo,
                            const Options& options,
                            bool skipFormatConversion);
    bool createColorTable(const SkImageInfo& dstInfo);

    enum XformMode {
        // Requires only a swizzle pass.
        kSwizzleOnly_XformMode,

        // Requires only a color xform pass.
        kColorOnly_XformMode,

        // Requires a swizzle and a color xform.
        kSwizzleColor_XformMode,
    };
    XformMode fXformMode;

    std::unique_ptr<SkSwizzler> fSwizzler;
    skia_private::AutoTMalloc<uint8_t> fStorage;
    int fXformWidth = -1;
    sk_sp<SkColorPalette> fColorTable;  // May be unpremul.
};

#endif  // SkPngCodecBase_DEFINED
