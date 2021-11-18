/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"

namespace skottie::internal {

#ifdef SK_ENABLE_SKSL

namespace  {
class SkSLColorFilterAdapter final : public DiscardableAdapterBase<SkSLColorFilterAdapter,
                                                             sksg::ExternalColorFilter> {
public:
    SkSLColorFilterAdapter(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      sk_sp<sksg::ExternalColorFilter> node)
        : INHERITED(std::move(node))
    {
        enum : size_t {
            kSkSL_index = 0,
            kFirstUniform_index = 1,
        };
        if (jprops.size() < 1) {
            return;
        }
        const skjson::ObjectValue* jSkSL = jprops[kSkSL_index];
        if (!jSkSL) {
            return;
        }
        const skjson::StringValue* jShader = (*jSkSL)["sh"];
        if (!jShader) {
            return;
        }
        SkString skSL = SkString(jShader->begin(), jShader->size());
        auto result = SkRuntimeEffect::MakeForColorFilter(skSL, {});
        if (!result.effect) {
            abuilder.log(Logger::Level::kError, nullptr, "Failed to parse SkSL color filter: %s",
               result.errorText.c_str());
            return;
        }
        fEffect = std::move(result.effect);

        // construct dynamic uniform list from jprops, skip SkSL property
        for (size_t i = kFirstUniform_index; i < jprops.size(); i++) {
            const skjson::ObjectValue* jprop = jprops[i];
            if (!jprop) { continue; }
            const skjson::StringValue* uniformName = (*jprop)["nm"];
            if (!uniformName) { continue; }
            auto uniformTuple = std::make_tuple(SkString(uniformName->begin(),
                                                         uniformName->size()),
                                                std::make_unique<VectorValue>());
            fUniforms.push_back(std::move(uniformTuple));
            this->bind(abuilder, (*jprop)["v"], std::get<1>(fUniforms.back()).get());
        }
    }

private:
    void onSync() override {
        sk_sp<SkData> uniformData = buildUniformData();
        if (nullptr == uniformData) {
            return;
        }
        auto cf = fEffect->makeColorFilter(uniformData);
        this->node()->setColorFilter(cf);
    }

    sk_sp<SkData> buildUniformData() const {
        if (!fEffect) {
            return nullptr;
        }
        sk_sp<SkData> uniformData = SkData::MakeFromMalloc(sk_malloc_canfail(fEffect->uniformSize()),
                                                           fEffect->uniformSize());
        if (nullptr == uniformData->data()) {
            return nullptr;
        }
        for (const auto& uniform : fUniforms) {
            const auto& name = std::get<0>(uniform);
            const auto& data = std::get<1>(uniform);
            auto metadata = fEffect->findUniform(name.c_str());
            auto src = reinterpret_cast<uint8_t*>(uniformData->writable_data()) + metadata->offset;
            memcpy(reinterpret_cast<void*>(src), &data->at(0), data->size() * sizeof(float));
        }
        return uniformData;
    }
    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::tuple<SkString, std::unique_ptr<VectorValue>>> fUniforms;
    using INHERITED = DiscardableAdapterBase<SkSLColorFilterAdapter, sksg::ExternalColorFilter>;
};

} // namespace

#endif  // SK_ENABLE_SKSL

sk_sp<sksg::RenderNode> EffectBuilder::attachSkSLColorFilter(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
#ifdef SK_ENABLE_SKSL
    auto cfNode = sksg::ExternalColorFilter::Make(std::move(layer));
    return fBuilder->attachDiscardableAdapter<SkSLColorFilterAdapter>(jprops, *fBuilder, cfNode);
#else
    return layer;
#endif
}

} // namespace skottie::internal
