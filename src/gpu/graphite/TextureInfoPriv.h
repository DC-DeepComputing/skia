/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_TextureInfoPriv_DEFINED
#define skgpu_graphite_TextureInfoPriv_DEFINED

#include "include/core/SkString.h"
#include "include/gpu/graphite/TextureInfo.h"

#include <cstdint>

namespace skgpu::graphite {

class TextureInfoPriv {
public:
    template <typename SomeTextureInfoData>
    static TextureInfo Make(const SomeTextureInfoData& data) {
        return TextureInfo(data);
    }

    template <typename SomeTextureInfoData>
    static const SomeTextureInfoData& Get(const TextureInfo& info) {
        SkASSERT(info.isValid() && info.backend() == SomeTextureInfoData::kBackendApi);
        return *(static_cast<const SomeTextureInfoData*>(info.fData.get()));
    }

    template <typename SomeTextureInfoData>
    static bool Copy(const TextureInfo& info, SomeTextureInfoData* out) {
        if (!info.isValid() || info.backend() != SomeTextureInfoData::kBackendApi) {
            return false;
        }

        SkASSERT(out);
        *out = Get<SomeTextureInfoData>(info);
        return true;
    }
};

}  // namespace skgpu::graphite

#endif
