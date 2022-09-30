/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ResourceTypes.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"

using namespace skgpu;
using namespace skgpu::graphite;

namespace {
    const SkISize kSize = {16, 16};
    const uint32_t kNumLevels = 5;
}

DEF_GRAPHITE_TEST_FOR_CONTEXTS(UpdateImageBackendTextureTest, reporter, context) {
    // TODO: Right now this just tests very basic combinations of images. This should be expanded
    // to cover a much broader set of things once we add more support in Graphite for different
    // formats, color types, etc.

    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    for (bool withMips : { true, false }) {
        for (Renderable renderable : { Renderable::kYes, Renderable::kNo }) {

            TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                                  withMips ? kNumLevels : 1,
                                                                  Protected::kNo,
                                                                  renderable);

            BackendTexture texture = recorder->createBackendTexture(kSize, info);
            REPORTER_ASSERT(reporter, texture.isValid());

            sk_sp<SkImage> image = SkImage::MakeGraphiteFromTexture(recorder.get(),
                                                                    texture,
                                                                    kRGBA_8888_SkColorType,
                                                                    kPremul_SkAlphaType,
                                                                    /* colorSpace= */ nullptr);
            REPORTER_ASSERT(reporter, image);
            REPORTER_ASSERT(reporter, image->hasMipmaps() == withMips);

            image.reset();

            // We should fail when trying to wrap the same image in an image with a non-compatible
            // color type.
            image = SkImage::MakeGraphiteFromTexture(recorder.get(),
                                                     texture,
                                                     kAlpha_8_SkColorType,
                                                     kPremul_SkAlphaType,
                                                     /* colorSpace= */ nullptr);
            REPORTER_ASSERT(reporter, !image);

            recorder->deleteBackendTexture(texture);
        }
    }
}
