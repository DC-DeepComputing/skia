/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlTextureInfo_DEFINED
#define skgpu_graphite_MtlTextureInfo_DEFINED

namespace skgpu::graphite::TextureInfos {
SK_API MakeMetal(const MtlTextureInfo& mtlInfo);

SK_API bool GetMtlTextureInfo(const TextureInfo&, MtlTextureInfo* info);
} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlBackendContext_DEFINED
