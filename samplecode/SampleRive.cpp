/*
 * Copyright 2021 Pentrek.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkParsePath.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"
#include "tools/timer/TimeUtils.h"

#include "src/core/SkGeometry.h"

#include <stdlib.h>

#include "rive/artboard.hpp"
#include "rive/file.hpp"
#include "rive/layout.hpp"
#include "rive/math/aabb.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/animation/state_machine_input_instance.hpp"
#include "skia_renderer.hpp"

#include "include/core/SkStream.h"
#include "tools/Resources.h"

enum RiveInputType {
    kBool,
    kFloat,
    kTrigger,
};
static RiveInputType find_type(rive::StateMachineInstance* smi, rive::SMIInput* input) {
    if (smi->getBool(input->name())) {
        return RiveInputType::kBool;
    }
    if (smi->getNumber(input->name())) {
        return RiveInputType::kFloat;
    }
    if (smi->getTrigger(input->name())) {
        return RiveInputType::kTrigger;
    }
    return (RiveInputType)(-1);
}

static rive::File* loadfile() {
    auto data = GetResourceAsData("rives/877-1781-football-time.riv");
//    SkFILEStream stream("/Users/mike/Downloads/877-1781-football-time.riv");

    auto reader = rive::BinaryReader((uint8_t*)data->data(), data->size());
    rive::File* file = nullptr;
    auto result = rive::File::import(reader, &file);
    if (result != rive::ImportResult::success) {
        fprintf(stderr, "failed to import file\n");
        return nullptr;
    }
    return file;
}

class SampleRive : public Sample {
    double fTimeBase = 0, fTime = 0, fPrevTime = 0;

public:
    rive::File* fFile = nullptr;
    rive::Artboard* fArtboard = nullptr;
    rive::StateMachineInstance* fStateMachineInstance = nullptr;
    rive::LinearAnimationInstance* fAnimationInstance = nullptr;
    // hack
    rive::SMITrigger* fJump;

    SampleRive() {}

    void init() {
        fFile = loadfile();

        auto sourceArtboard = fFile->artboard();

        fArtboard = sourceArtboard->instance();
        fArtboard->advance(0.0f);

        size_t index = 0;

        auto animation = index < fArtboard->animationCount()
                             ? fArtboard->animation(index)
                             : nullptr;
        if (animation != nullptr)
        {
            fAnimationInstance = new rive::LinearAnimationInstance(animation);
        }

        auto stateMachine = index < fArtboard->stateMachineCount()
                                ? fArtboard->stateMachine(index)
                                : nullptr;
        if (stateMachine != nullptr)
        {
            fStateMachineInstance = new rive::StateMachineInstance(stateMachine);
        }

        for (size_t i = 0; i < fArtboard->animationCount(); ++i) {
            SkDebugf("animation[%zu] %s\n", i, fArtboard->animation(i)->name().c_str());
        }
        for (size_t i = 0; i < fArtboard->stateMachineCount(); ++i) {
            auto sm = fArtboard->stateMachine(index);
            SkDebugf("statemachine[%zu] %s\n", i, sm->name().c_str());

            rive::StateMachineInstance smi(sm);
            for (size_t j = 0; j < smi.inputCount(); ++j) {
                auto input = smi.input(j);
                RiveInputType rt = find_type(&smi, input);
                SkDebugf("    [%zu] %s %d %d\n", j, input->name().c_str(), input->inputCoreType(), rt);
            }
        }
    }

protected:
    SkString name() override { return SkString("rive"); }

    bool onChar(SkUnichar uni) override {
            switch (uni) {
                case ' ': {
                    auto trigger = fStateMachineInstance->getTrigger("Jump");
                    if (trigger) {
                        trigger->fire();
                        return true;
                    }
                } break;
                default: break;
            }
            return false;
    }

    bool onAnimate(double nanos) override {
        double secs = nanos * 1e-9;
        if (fTimeBase == 0) {
            fTimeBase = secs;
            fTime = 0;
        }
        fPrevTime = fTime;
        fTime = secs - fTimeBase;
        return true;
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (!fFile) {
            this->init();
        }

        double delta = fTime - fPrevTime;
        fAnimationInstance->advance(delta);
        fAnimationInstance->apply(fArtboard);

        fStateMachineInstance->advance(fArtboard, delta);

        fArtboard->advance(delta);
//        SkDebugf("%g\n", fDuration);

        rive::SkiaRenderer renderer(canvas);
        renderer.save();
        renderer.align(rive::Fit::contain,
                       rive::Alignment::center,
                       rive::AABB(0, 0, 512, 512),
                       fArtboard->bounds());
        fArtboard->draw(&renderer);
        renderer.restore();
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
#if 0
        const SkScalar tol = 8;
        const SkRect r = SkRect::MakeXYWH(x - tol, y - tol, tol * 2, tol * 2);
        for (int i = 0; i < N; ++i) {
            if (r.intersects(SkRect::MakeXYWH(fPts[i].fX, fPts[i].fY, 1, 1))) {
                return new Click([this, i](Click* c) {
                    fPts[i] = c->fCurr;
                    return true;
                });
            }
        }
#endif
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    using INHERITED = Sample;
};
DEF_SAMPLE( return new SampleRive; )

