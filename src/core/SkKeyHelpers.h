/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyHelpers_DEFINED
#define SkKeyHelpers_DEFINED

#ifdef SK_GRAPHITE_ENABLED
#include "experimental/graphite/include/Context.h"
#endif

#include "include/core/SkBlendMode.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"

enum class SkBackend : uint8_t;
class SkPaintParamsKey;

// The KeyHelpers can be used to manually construct an SkPaintParamsKey

namespace DepthStencilOnlyBlock {

    void AddToKey(SkPaintParamsKey*);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace DepthStencilOnlyBlock

namespace SolidColorShaderBlock {

    void AddToKey(SkPaintParamsKey*);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace SolidColorShaderBlock

// TODO: move this functionality to the SkLinearGradient, SkRadialGradient, etc classes
namespace GradientShaderBlocks {

    void AddToKey(SkPaintParamsKey*, SkShader::GradientType, SkTileMode);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace GradientShaderBlocks

namespace ImageShaderBlock {

    struct ImageData {
        bool operator==(const ImageData& rhs) const {
            return fTileModes[0] == rhs.fTileModes[0] &&
                   fTileModes[1] == rhs.fTileModes[1];
        }
        bool operator!=(const ImageData& rhs) const { return !(*this == rhs); }

        SkTileMode fTileModes[2];
    };

    void AddToKey(SkBackend, SkPaintParamsKey*, const ImageData&);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace ImageShaderBlock

namespace BlendModeBlock {

    void AddToKey(SkPaintParamsKey*, SkBlendMode);
#ifdef SK_DEBUG
    void Dump(const SkPaintParamsKey&, int headerOffset);
#endif

} // namespace BlendModeBlock

#ifdef SK_GRAPHITE_ENABLED
// Bridge between the combinations system and the SkPaintParamsKey
SkPaintParamsKey CreateKey(skgpu::ShaderCombo::ShaderType, SkTileMode, SkBlendMode);
#endif

#endif // SkKeyHelpers_DEFINED
