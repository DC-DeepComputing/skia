/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAnimatedFont_DEFINED
#define SkottieAnimatedFont_DEFINED

#include "include/core/SkDrawable.h"
#include "include/core/SkRRect.h"

class SkCanvas;

namespace sksg   { class RenderNode;  }
namespace skjson { class ObjectValue; }

namespace skottie::internal {

class AnimationBuilder;

class AnimatedGlyph final : public SkDrawable {
public:
    ~AnimatedGlyph() override;

    static sk_sp<AnimatedGlyph> Make(const skjson::ObjectValue&, const AnimationBuilder*);

protected:
    SkRect onGetBounds() override;

    void onDraw(SkCanvas*) override;

private:
    explicit AnimatedGlyph(sk_sp<sksg::RenderNode>);

    const sk_sp<sksg::RenderNode> fRenderNode;
};

}  // namespace skottie::internal

#endif  // SkottieAnimatedFont_DEFINED
