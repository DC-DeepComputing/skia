/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/RiveSlide.h"

#include <rive/span.hpp>
#include <skia_renderer.hpp>
#include <skia_factory.hpp>

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkTime.h"
#include "src/utils/SkOSPath.h"
#include "tools/Resources.h"
#include "tools/timer/TimeUtils.h"

#include <cmath>
#include <vector>

#include "imgui.h"

static rive::SkiaFactory gFactory;

RiveSlide::RiveSlide(const SkString& name, const SkString& path)
    : fPath(path)
{
    fName = name;
}

void RiveSlide::load(SkScalar w, SkScalar h) {
//    SkOSPath::Dirname(fPath.c_str()),

    FILE* f = fopen(fPath.c_str(), "rb");
    auto data = SkData::MakeFromFILE(f);
    auto span = rive::Span<const uint8_t>((const uint8_t*)data->data(), data->size());
    fFile = rive::File::import(span, &gFactory);
    fclose(f);

    if (fFile) {
        fArtboard = fFile->artboardAt(0);
        fScene = fArtboard->stateMachineAt(0);
        if (!fScene) {
            fScene = fArtboard->animationAt(0);
        }
    }
}

void RiveSlide::unload() {
    fScene = nullptr;
    fArtboard = nullptr;
    fFile = nullptr;
}

void RiveSlide::resize(SkScalar w, SkScalar h) {
    fWinSize = { w, h };
}

SkISize RiveSlide::getDimensions() const {
//    auto aabb = fArtboard->bounds();
//    return SkISize{ SkScalarRoundToInt(aabb.width()), SkScalarRoundToInt(aabb.height()) };
    // We always scale to fill the window.
    return fWinSize.toCeil();
}

void RiveSlide::draw(SkCanvas* canvas) {
    if (fScene) {
        rive::SkiaRenderer renderer(canvas);

        fScene->draw(&renderer);
    }
}

static double nanos_to_secs(double nanos) {
    return nanos * 1e-9;
}

bool RiveSlide::animate(double nanos) {
    const double secs = nanos_to_secs(nanos);

    if (fScene) {
        if (!fSecsBase) {
            // Reset the animation time.
            fSecsBase = secs;
        }
        fScene->advanceAndApply(secs - fSecsBase);
        fSecsBase = secs;
        return true;
    }
    return false;
}

bool RiveSlide::onChar(SkUnichar c) {
    return INHERITED::onChar(c);
}

bool RiveSlide::onMouse(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey) {
    if (fScene) {
        auto pos = rive::Vec2D(x, y);
        switch (state) {
            case skui::InputState::kUp:     fScene->pointerUp(pos); break;
            case skui::InputState::kDown:   fScene->pointerDown(pos); break;
            case skui::InputState::kMove:   fScene->pointerMove(pos); break;
            default: break;
        }
        return true;
    }
    return false;
}
