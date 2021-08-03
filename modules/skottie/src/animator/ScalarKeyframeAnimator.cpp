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

namespace skottie::internal {

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
class ScalarExpressionAnimator final : public Animator {
public:
    ScalarExpressionAnimator(sk_sp<ExpressionEvaluator<ScalarValue>> expression_evaluator, sk_sp<Logger> logger, ScalarValue* target_value) :
        fExpressionEvaluator(std::move(expression_evaluator)), fLogger(std::move(logger)), fTarget(target_value) {}

private:

    StateChanged onSeek(float t) override {
        auto old_value = *fTarget;

        bool success = fExpressionEvaluator->Evaluate(t, fTarget);
        if (!success && fLogger && !hasLogged) {
            // Only log once to avoid spam.
            hasLogged = true;
            fLogger->log(Logger::Level::kError, "Unable to evaluate the expression.");
        }
        return *fTarget != old_value;
    }

    sk_sp<ExpressionEvaluator<ScalarValue>> fExpressionEvaluator;
    sk_sp<Logger> fLogger;
    ScalarValue* fTarget;
    bool hasLogged = false;
};

class ScalarAnimatorBuilder final : public AnimatorBuilder {
    public:
        explicit ScalarAnimatorBuilder(ScalarValue* target) : fTarget(target) {}

        sk_sp<KeyframeAnimator> makeFromKeyframes(const AnimationBuilder& abuilder,
                                     const skjson::ArrayValue& jkfs) override {
            SkASSERT(jkfs.size() > 0);
            if (!this->parseKeyframes(abuilder, jkfs)) {
                return nullptr;
            }

            return sk_sp<ScalarKeyframeAnimator>(
                        new ScalarKeyframeAnimator(std::move(fKFs), std::move(fCMs), fTarget));
        }

        sk_sp<Animator> makeFromExpression(sk_sp<ExpressionManager> em, sk_sp<Logger> logger, const SkString& expr) override {
            if (!em) {
                if (logger) {
                    logger->log(Logger::Level::kWarning, "Expression manager not supplied.");
                }
                return nullptr;
            }
            sk_sp<ExpressionEvaluator<ScalarValue>> expression_evaluator = em->CreateNumberExpressionEvaluator(expr.c_str());
            if (!expression_evaluator) {
                if (logger) {
                    logger->log(Logger::Level::kWarning, "Expression manager not supplied.");
                }
                return nullptr;
            }
            return sk_sp<ScalarExpressionAnimator>(new ScalarExpressionAnimator(expression_evaluator, std::move(logger), fTarget));
        }


        bool parseValue(const AnimationBuilder&, const skjson::Value& jv) const override {
            return Parse(jv, fTarget);
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
    ScalarAnimatorBuilder builder(v);

    return this->bindImpl(abuilder, jprop, builder);
}

} // namespace skottie::internal
