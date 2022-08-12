/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/AnimatedFont.h"

#include "include/core/SkCanvas.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/sksg/include/SkSGRenderNode.h"

namespace skottie::internal {

sk_sp<AnimatedGlyph> AnimatedGlyph::Make(const skjson::ObjectValue& jdata,
                                         const AnimationBuilder* abuilder) {
    AnimationBuilder::LayerInfo linfo{
        {0,0},
        ParseDefault<float>(jdata["ip"], 0.0f),
        ParseDefault<float>(jdata["op"], 0.0f)
    };

    auto node = abuilder->attachPrecompLayer(jdata, &linfo);

    return node
            ? sk_sp<AnimatedGlyph>(new AnimatedGlyph(std::move(node)))
            : nullptr;
}

AnimatedGlyph::AnimatedGlyph(sk_sp<sksg::RenderNode> node)
    : fRenderNode(std::move(node)) {}

AnimatedGlyph::~AnimatedGlyph() = default;

SkRect AnimatedGlyph::onGetBounds() {
    return fRenderNode->revalidate(nullptr, SkMatrix::I());
}

void AnimatedGlyph::onDraw(SkCanvas* canvas) {
    fRenderNode->revalidate(nullptr, SkMatrix::I());

    canvas->scale(0.01f, 0.01f);
    fRenderNode->render(canvas);
}

}  // namesapce skottie::internal
