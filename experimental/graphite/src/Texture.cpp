/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Texture.h"

#include "experimental/graphite/src/Caps.h"

namespace skgpu {

Texture::Texture(const Gpu* gpu, SkISize dimensions, const TextureInfo& info, Ownership ownership)
        : Resource(gpu), fDimensions(dimensions), fInfo(info), fOwnership(ownership) {}

Texture::~Texture() {}

void Texture::registerAsScratch(const Caps& caps) {
    caps.computeScratchKeyForSurface(this->dimensions(), this->textureInfo(), &fScratchKey);
}

} // namespace skgpu
