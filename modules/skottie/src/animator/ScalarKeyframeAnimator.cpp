/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/animator/KeyframeAnimator.h"
#include "third_party/skia/HEAD/include/core/SkString.h"
#include "third_party/skia/HEAD/modules/skottie/include/Skottie.h"
#include "third_party/skia/HEAD/modules/skottie/src/SkottiePriv.h"
#include "third_party/skia/HEAD/src/utils/SkJSON.h"

namespace skottie::internal {

class AnimationBuilder;

namespace  {

    // Scalar specialization: stores scalar values (floats) inline in keyframes.
class ScalarKeyframeAnimator final : public KeyframeAnimator {
public:
    ScalarKeyframeAnimator(std::vector<Keyframe> kfs,
                           std::vector<SkCubicMap> cms,
                           ScalarValue* target_value)
        : INHERITED(std::move(kfs), std::move(cms))
        , fTarget(target_value) {}

private:

    StateChanged onSeek(float t) override {
        const auto& lerp_info = this->getLERPInfo(t);
        const auto  old_value = *fTarget;

        *fTarget = Lerp(lerp_info.vrec0.flt, lerp_info.vrec1.flt, lerp_info.weight);

        return *fTarget != old_value;
    }

    ScalarValue* fTarget;

    using INHERITED = KeyframeAnimator;
};

    // Scalar specialization: stores scalar values (floats).
class ScalarExpressionAnimator final : public ExpressionAnimator {
public:
    ScalarExpressionAnimator(sk_sp<ScalarExpressionEvaluator> expression_evaluator, ScalarValue* target_value) :
        fExpressionEvaluator(std::move(expression_evaluator)), fTarget(target_value) {}

private:

    StateChanged onSeek(float t) override {
        auto old_value = *fTarget;

        *fTarget = fExpressionEvaluator->Evaluate();

        return *fTarget != old_value;
    }

    sk_sp<ScalarExpressionEvaluator> fExpressionEvaluator;
    ScalarValue* fTarget;
};

class ScalarDynamicBuilder final : public DynamicAnimatorBuilder {
    public:
        explicit ScalarDynamicBuilder(ScalarValue* target) : fTarget(target) {}

        sk_sp<DynamicAnimator> make(const AnimationBuilder& abuilder,
                                    const skjson::ArrayValue& jkfs,
                                    const skjson::Value& expression) override {
          SkString expression_string;
          if (parseExpression(expression, &expression_string)) {
            sk_sp<ScalarExpressionEvaluator> expression_evaluator =
                abuilder.createScalarExpressionEvaluator(expression_string);
            if (expression_evaluator != nullptr) {
              return sk_sp<ScalarExpressionAnimator>(
                  new ScalarExpressionAnimator(std::move(expression_evaluator),
                                               fTarget));
            }
          }
          SkASSERT(jkfs.size() > 0);
          if (!this->parseKeyframes(abuilder, jkfs)) {
            return nullptr;
          }

          return sk_sp<ScalarKeyframeAnimator>(new ScalarKeyframeAnimator(
              std::move(fKFs), std::move(fCMs), fTarget));
        }

        bool parseValue(const AnimationBuilder&, const skjson::Value& jv) const override {
            return Parse(jv, fTarget);
        }

        bool parseExpression(const skjson::Value& expression, SkString* result) {
            return Parse(expression, result);
        }

    private:
        bool parseKFValue(const AnimationBuilder&,
                          const skjson::ObjectValue&,
                          const skjson::Value& jv,
                          Keyframe::Value* v) override {
            return Parse(jv, &v->flt);
        }

        ScalarValue* fTarget;
    };

} // namespace

template <>
bool AnimatablePropertyContainer::bind<ScalarValue>(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop,
                                                    ScalarValue* v) {
    ScalarDynamicBuilder builder(v);

    return this->bindImpl(abuilder, jprop, builder);
}

} // namespace skottie::internal
