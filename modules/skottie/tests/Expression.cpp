/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "tests/Test.h"

using namespace skottie;

namespace {

class ScalarExpressionEvaluator : public ExpressionEvaluator<float> {
public:
    float evaluate(float t) override {
        return 7.0f;
    }
};

class FakeExpressionManager : public ExpressionManager {
public:
    sk_sp<ExpressionEvaluator<float>> createNumberExpressionEvaluator(
        const char expression[]) override {
        return sk_make_sp<ScalarExpressionEvaluator>();
    }  

    sk_sp<ExpressionEvaluator<SkString>> createStringExpressionEvaluator(
        const char expression[]) override {
        return nullptr;
    }

    sk_sp<ExpressionEvaluator<std::vector<float>>> createArrayExpressionEvaluator(
        const char expression[]) {
        return nullptr;
    }
};

class FakePropertyObserver : public PropertyObserver {
    public:
        void onOpacityProperty(const char node_name[],
                                     const LazyHandle<OpacityPropertyHandle>& opacity_handle) {
            fNodeName = node_name;
            opacity = opacity_handle()->get();
        }

    SkString fNodeName;
    float opacity = 0;
};
} // namespace

DEF_TEST(SimpleScalarExpression, r) {
    static constexpr char json[] =
        R"({
             "v": "5.2.1",
             "w": 100,
             "h": 100,
             "fr": 10,
             "ip": 0,
             "op": 100,
             "layers": [
               {
                 "ip": 0,
                 "op": 100,
                 "ty": 4,
                 "nm": "Shape Layer",
                 "ks": {
                   "o": {
                     "a": 0,
                     "k": 100,
                     "ix": 11,
                     "x": "var $bm_rt;\n$bm_rt = 70;\n}"
                   },
                 }
               }
             ]
           })";

    SkMemoryStream stream(json, strlen(json));

    auto em = sk_make_sp<FakeExpressionManager>();
    auto observer = sk_make_sp<FakePropertyObserver>();

    auto anim = Animation::Builder()
        .SetExpressionManager(em)
        .SetPropertyObserver(observer)
        .make(&stream);

    REPORTER_ASSERT(r, anim);

    anim->seekFrameTime(0);

    REPORTER_ASSERT(r, SkScalarNearlyEqual(observer->opacity, 70));
}
