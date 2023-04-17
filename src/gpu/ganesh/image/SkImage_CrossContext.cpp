/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/image/SkImage_CrossContext.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/gpu/ganesh/SkImageGanesh.h"

#include <utility>

class GrDirectContext;
struct SkIRect;

sk_sp<SkImage> SkImage_CrossContext::onMakeSubset(GrDirectContext* direct,
                                                  const SkIRect& subset) const {
    if (!direct) {
        return nullptr;
    }
    auto pixels = SkImages::TextureFromImage(direct, this);
    return pixels ? pixels->makeSubset(direct, subset) : nullptr;
}

sk_sp<SkImage> MakeCrossContextImage(std::unique_ptr<SkImageGenerator> generator) {
    SkImage_Lazy::Validator validator(
            SharedGenerator::Make(std::move(generator)), nullptr, nullptr);

    return validator ? sk_make_sp<SkImage_CrossContext>(&validator) : nullptr;
}
