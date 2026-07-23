/*
 * Copyright 2026 Apple Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRefCnt.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/SkTPin.h"
#include "modules/jsonreader/SkJSONReader.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <utility>

namespace skottie::internal {

namespace {

class ColorOverlayAdapter final
        : public DiscardableAdapterBase<ColorOverlayAdapter, sksg::ExternalImageFilter> {
public:
    ColorOverlayAdapter(const skjson::ObjectValue& jstyle, const AnimationBuilder& abuilder) {
        this->bind(abuilder, jstyle["c" ], fColor);
        this->bind(abuilder, jstyle["so"], fOpacity);
    }

private:
    void onSync() override {
        const auto opacity = SkTPin(fOpacity / 100, 0.0f, 1.0f);
        const auto  color  = static_cast<SkColor4f>(fColor);

        // Colorize the source using the overlay color and opacity, preserving source alpha shape.
        SkColorMatrix cm{0, 0, 0,                  0, color.fR,
                         0, 0, 0,                  0, color.fG,
                         0, 0, 0,                  0, color.fB,
                         0, 0, 0, opacity * color.fA,        0};

        auto f = SkImageFilters::ColorFilter(SkColorFilters::Matrix(cm), nullptr);

        sk_sp<SkImageFilter> source;
        this->node()->setImageFilter(SkImageFilters::Merge(std::move(source), std::move(f)));
    }

    ColorValue  fColor;
    ScalarValue fOpacity = 100; // percentage

    using INHERITED = DiscardableAdapterBase<ColorOverlayAdapter, sksg::ExternalImageFilter>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachColorOverlayStyle(const skjson::ObjectValue& jstyle,
                                                               sk_sp<sksg::RenderNode> layer) const {
    auto filter_node =
            fBuilder->attachDiscardableAdapter<ColorOverlayAdapter>(jstyle, *fBuilder);

    return sksg::ImageFilterEffect::Make(std::move(layer), std::move(filter_node));
}

} // namespace skottie::internal
