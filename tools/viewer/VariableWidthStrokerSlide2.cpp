/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPointPriv.h"
#include "src/utils/SkDashPathPriv.h"
#include "tools/viewer/ClickHandlerSlide.h"

#include <numbers>
#include <stack>
#include <vector>

#include "imgui.h"

class SkStrokeEffect : public SkRefCnt {
public:
    // virtual ~SkStrokeEffect() {}
    virtual float strokeMultiplierAt(float t) = 0;
};

static void extrude_line(SkPoint start, SkPoint end,
                         SkPathBuilder* inner, SkPathBuilder* outer,
                         float startRadius, float endRadius) {
    SkVector normalUnit = end - start;
    normalUnit.normalize();
    SkPointPriv::RotateCCW(&normalUnit);

    if (outer->empty()) {
        outer->moveTo(start + normalUnit * startRadius);
    }
    else {
        // This is where join logic would go
        outer->lineTo(start + normalUnit * startRadius);
    }
    outer->lineTo(end + normalUnit * endRadius);

    if (inner->empty()) {
        inner->moveTo(start - normalUnit * startRadius);
    }
    else {
        // This is where join logic would go
        inner->lineTo(start - normalUnit * startRadius);
    }
    inner->lineTo(end - normalUnit * endRadius);
}

static void extrude_quad(SkPoint start, SkPoint control, SkPoint end,
                         SkPathBuilder* inner, SkPathBuilder* outer,
                         float startRadius, float controlRadius, float endRadius) {
    SkVector normalUnitStart = control - start;
    normalUnitStart.normalize();
    SkPointPriv::RotateCCW(&normalUnitStart);

    SkVector normalUnitEnd = end - control;
    normalUnitEnd.normalize();
    SkPointPriv::RotateCCW(&normalUnitEnd);

    SkVector normalUnitMid = normalUnitStart + normalUnitEnd;
    normalUnitMid.normalize();

    if (outer->empty()) {
        outer->moveTo(start + normalUnitStart * startRadius);
    }
    else {
        // This is where join logic would go
        outer->lineTo(start + normalUnitStart * startRadius);
    }
    outer->quadTo(control + normalUnitMid * controlRadius, end + normalUnitEnd * endRadius);


    if (inner->empty()) {
        inner->moveTo(start - normalUnitStart * startRadius);
    }
    else {
        // This is where join logic would go
        inner->lineTo(start - normalUnitStart * startRadius);
    }
    inner->quadTo(control - normalUnitMid * controlRadius, end - normalUnitEnd * endRadius);
}

static float line_length(SkPoint start, SkPoint end) {
  SkPath path;
  path.moveTo(start);
  path.lineTo(end);

  SkPathMeasure pathMeasure(path, false);
  return pathMeasure.getLength();
}

static float quad_length(SkPoint start, SkPoint control, SkPoint end) {
  SkPath path;
  path.moveTo(start);
  path.quadTo(control, end);

  SkPathMeasure pathMeasure(path, false);
  return pathMeasure.getLength();
}

static SkPath extrude_path(const SkPath& input, sk_sp<SkStrokeEffect> sEffect, float width) {
    SkPathBuilder inner, outer;

    SkPath::Iter iter(input, false);
    float radius = width / 2;

    float totalLength = 0;

    SkPathMeasure pMeas(input, false);
    do {
        float delta = pMeas.getLength();
        totalLength += delta;
    } while(pMeas.nextContour());

    float currLength = 0;
    SkPoint pts[4];
    for (auto verb = iter.next(pts); verb != SkPath::kDone_Verb; verb = iter.next(pts)) {
        float prevLength = currLength;
        switch (verb) {
            case SkPath::kMove_Verb:
                // TODO(kjlubick)?
                break;
            case SkPath::kLine_Verb:
                currLength += line_length(pts[0], pts[1]);
                extrude_line(pts[0], pts[1], &inner, &outer,
                             radius * sEffect->strokeMultiplierAt(prevLength / totalLength),
                             radius * sEffect->strokeMultiplierAt(currLength / totalLength));
                break;
            case SkPath::kQuad_Verb:
                currLength += quad_length(pts[0], pts[1], pts[2]);
                extrude_quad(pts[0], pts[1], pts[2], &inner, &outer,
                             radius * sEffect->strokeMultiplierAt(prevLength / totalLength),
                             radius * sEffect->strokeMultiplierAt((prevLength + currLength) / (totalLength * 2)),
                             radius * sEffect->strokeMultiplierAt(currLength / totalLength));
                break;
            default:
                SK_ABORT("not implemented\n");
        }
    }

    outer.lineTo(inner.getLastPt());
    SkPathPriv::ReversePathTo(&outer, inner);
    outer.close();
    return outer.detach();
}

namespace {
class DividePathEffect : public SkPathEffectBase {
public:
    DividePathEffect() {}

    bool onNeedsCTM() const final { return false; }

    bool onFilterPath(SkPath* dst,
                      const SkPath& src,
                      SkStrokeRec* rec,
                      const SkRect* cullR,
                      const SkMatrix& ctm) const final {
        SkScalar dashing[2] = {10,0};
        SkPathEffect::DashInfo splitting(dashing, 2, 0);
        return SkDashPath::FilterDashPath(dst, src, rec, cullR, splitting);
    }

protected:
    void flatten(SkWriteBuffer&) const final {}

private:
    SK_FLATTENABLE_HOOKS(DividePathEffect)

    bool computeFastBounds(SkRect* bounds) const final { return false; }
};

sk_sp<SkFlattenable> DividePathEffect::CreateProc(SkReadBuffer&) { return nullptr; }

class SwoopStrokeEffect: public SkStrokeEffect {
    float strokeMultiplierAt(float t) {
        SkASSERTF(t >= -0.001 && t <= 1.001, "%f is out of range", t);
        return std::sin(t * SK_FloatPI) * 1.8 + 0.2;
    }
};

}  // namespace

//////////////////////////////////////////////////////////////////////////////

class VariableWidthStrokerSlide : public ClickHandlerSlide {
public:
    VariableWidthStrokerSlide()
            : fShowHidden(true)
            , fShowSkeleton(true)
            , fShowSkiaStroke(true)
            , fShowStrokePoints(false)
            , fShowUI(false)
            , fDifferentInnerFunc(false)
            , fShowErrorCurve(false) {
        resetToDefaults();

        fPtsPaint.setAntiAlias(true);
        fPtsPaint.setStrokeWidth(10);
        fPtsPaint.setStrokeCap(SkPaint::kRound_Cap);

        fStrokePointsPaint.setAntiAlias(true);
        fStrokePointsPaint.setStrokeWidth(5);
        fStrokePointsPaint.setStrokeCap(SkPaint::kRound_Cap);

        fStrokePaint.setAntiAlias(true);
        fStrokePaint.setStyle(SkPaint::kStroke_Style);
        fStrokePaint.setColor(SkColorSetARGB(128, 0, 184, 255));

        fNewFillPaint.setAntiAlias(true);
        fNewFillPaint.setColor(SkColorSetARGB(128, 235, 83, 113));

        fHiddenPaint.setAntiAlias(true);
        fHiddenPaint.setStyle(SkPaint::kStroke_Style);
        fHiddenPaint.setColor(SkColorSetARGB(255, 0, 255, 0));

        fSkeletonPaint.setAntiAlias(true);
        fSkeletonPaint.setStyle(SkPaint::kStroke_Style);
        fSkeletonPaint.setColor(SK_ColorRED);

        fName = "VariableWidthStroker";
    }

    void load(SkScalar w, SkScalar h) override { fWinSize = {w, h}; }

    void resize(SkScalar w, SkScalar h) override { fWinSize = {w, h}; }

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case '0':
                this->toggle(fShowUI);
                return true;
            case '1':
                this->toggle(fShowSkeleton);
                return true;
            case '2':
                this->toggle(fShowHidden);
                return true;
            case '3':
                this->toggle(fShowStrokePoints);
                return true;
            case '4':
                this->toggle(fShowSkiaStroke);
                return true;
            case 'x':
                resetToDefaults();
                return true;
            case '-':
                fWidth -= 5;
                if (fWidth < 0) {
                    fWidth = 1;
                }
                return true;
            case '=':
                fWidth += 5;
                return true;
            default:
                break;
        }
        return false;
    }

    void draw(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);

        SkPath p;
        this->makePath(&p);

        sk_sp<SkPathEffect> pEffect = sk_make_sp<DividePathEffect>();
        sk_sp<SkStrokeEffect> sEffect = sk_make_sp<SwoopStrokeEffect>();

        SkStrokeRec strokeRec(fStrokePaint);

        fStrokePaint.setStrokeWidth(fWidth);

        // The current result
        if (fShowSkiaStroke) {
            canvas->drawPath(p, fStrokePaint);
        }

        SkPath path;
        SkAssertResult(pEffect->filterPath(&path, p, &strokeRec, nullptr));
        SkPath fillPath = extrude_path(path, sEffect, fWidth);
        canvas->drawPath(fillPath, fNewFillPaint);

        if (fShowHidden) {
            canvas->drawPath(fillPath, fHiddenPaint);
        }

        if (fShowSkeleton) {
            canvas->drawPath(path, fSkeletonPaint);
            fPtsPaint.setColor(SK_ColorBLACK);
            fPtsPaint.setStrokeWidth(10);
            for (int i = 0; i < (int)fPathPts.size(); i+=2) {
                canvas->drawPoint(fPathPts[i], fPtsPaint);
            }
            fPtsPaint.setColor(SK_ColorRED);
            fPtsPaint.setStrokeWidth(6);
            for (int i = 1; i < (int)fPathPts.size(); i+=2) {
                canvas->drawPoint(fPathPts[i], fPtsPaint);
            }
        }

        if (fShowStrokePoints) {
            if (fShowSkiaStroke) {
                SkStrokeRec rec(fStrokePaint);
                SkPath skFillPath;
                rec.applyToPath(&skFillPath, path);
                drawStrokePoints(canvas, skFillPath);
            } else {
                drawStrokePoints(canvas, fillPath);
            }
        }

        if (fShowUI) {
            drawUI();
        }
    }

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        const SkScalar tol = 4;
        const SkRect r = SkRect::MakeXYWH(x - tol, y - tol, tol * 2, tol * 2);
        for (size_t i = 0; i < fPathPts.size(); ++i) {
            if (r.intersects(SkRect::MakeXYWH(fPathPts[i].fX, fPathPts[i].fY, 1, 1))) {
                return new Click([this, i](Click* c) {
                    fPathPts[i] = c->fCurr;
                    return true;
                });
            }
        }
        return nullptr;
    }

    bool onClick(ClickHandlerSlide::Click *) override { return false; }

private:
    /** Selectable menu item for choosing distance functions */
    struct DistFncMenuItem {
        std::string fName;
        int fDegree;
        bool fSelected;
        std::vector<float> fWeights;

        DistFncMenuItem(const std::string& name, int degree, bool selected) {
            fName = name;
            fDegree = degree;
            fSelected = selected;
            fWeights.resize(degree + 1, 1.0f);
        }
    };

    void toggle(bool& value) { value = !value; }

    void resetToDefaults() {
        fPathPts[0] = {300, 400};
        fPathPts[1] = {500, 400};
        fPathPts[2] = {700, 400};
        fPathPts[3] = {900, 400};
        fPathPts[4] = {1100, 400};

        fWidth = 175;

        fDistFncs = fDefaultsDistFncs;
        fDistFncsInner = fDefaultsDistFncs;
    }

    void makePath(SkPath* path) {
        path->moveTo(fPathPts[0]);
        path->quadTo(fPathPts[1], fPathPts[2]);
        path->quadTo(fPathPts[3], fPathPts[4]);
    }

    void drawStrokePoints(SkCanvas* canvas, const SkPath& fillPath) {
        SkPath::Iter it(fillPath, false);
        SkPoint points[4];
        SkPath::Verb verb;
        std::vector<SkPoint> pointsVec, ctrlPts;
        while ((verb = it.next(&points[0])) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kLine_Verb:
                    pointsVec.push_back(points[1]);
                    break;
                case SkPath::kQuad_Verb:
                    ctrlPts.push_back(points[1]);
                    pointsVec.push_back(points[2]);
                    break;
                case SkPath::kMove_Verb:
                    pointsVec.push_back(points[0]);
                    break;
                case SkPath::kClose_Verb:
                    break;
                default:
                    SkDebugf("Unhandled path verb %d for stroke points\n", verb);
                    SkASSERT(false);
                    break;
            }
        }

        canvas->drawPoints(SkCanvas::kPoints_PointMode, pointsVec.size(), pointsVec.data(),
                           fStrokePointsPaint);
        fStrokePointsPaint.setColor(SK_ColorRED);
        fStrokePointsPaint.setStrokeWidth(3);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, ctrlPts.size(), ctrlPts.data(),
                           fStrokePointsPaint);
        fStrokePointsPaint.setColor(SK_ColorBLACK);
        fStrokePointsPaint.setStrokeWidth(5);
    }

    void drawUI() {
        static constexpr auto kUIOpacity = 0.35f;
        static constexpr float kUIWidth = 200.0f, kUIHeight = 400.0f;
        ImGui::SetNextWindowBgAlpha(kUIOpacity);
        if (ImGui::Begin("E-C Controls", nullptr,
                         ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
            const SkRect uiArea = SkRect::MakeXYWH(10, 10, kUIWidth, kUIHeight);
            ImGui::SetWindowPos(ImVec2(uiArea.x(), uiArea.y()));
            ImGui::SetWindowSize(ImVec2(uiArea.width(), uiArea.height()));

            const auto drawControls = [](std::vector<DistFncMenuItem>& distFncs,
                                         const std::string& menuPfx,
                                         const std::string& ptPfx) {
                std::string degreeMenuLabel = menuPfx + ": ";
                for (const auto& df : distFncs) {
                    if (df.fSelected) {
                        degreeMenuLabel += df.fName;
                        break;
                    }
                }
                if (ImGui::BeginMenu(degreeMenuLabel.c_str())) {
                    for (size_t i = 0; i < distFncs.size(); i++) {
                        if (ImGui::MenuItem(distFncs[i].fName.c_str(), nullptr,
                                            distFncs[i].fSelected)) {
                            for (size_t j = 0; j < distFncs.size(); j++) {
                                distFncs[j].fSelected = j == i;
                            }
                        }
                    }
                    ImGui::EndMenu();
                }

                for (auto& df : distFncs) {
                    if (df.fSelected) {
                        for (int i = 0; i <= df.fDegree; i++) {
                            const std::string label = ptPfx + std::to_string(i);
                            ImGui::SliderFloat(label.c_str(), &(df.fWeights[i]), 0, 1);
                        }
                    }
                }
            };

            drawControls(fDistFncs, "Degree", "P");

            if (ImGui::CollapsingHeader("Inner stroke", true)) {
                fDifferentInnerFunc = true;
                drawControls(fDistFncsInner, "Degree (inner)", "Q");
            } else {
                fDifferentInnerFunc = false;
            }
        }
        ImGui::End();
    }

    bool fShowHidden, fShowSkeleton, fShowSkiaStroke, fShowStrokePoints, fShowUI, fDifferentInnerFunc,
            fShowErrorCurve;
    float fWidth = 175;
    SkPaint fPtsPaint, fStrokePaint, fNewFillPaint, fHiddenPaint, fSkeletonPaint,
            fStrokePointsPaint;
    inline static constexpr int kNPts = 5;
    std::array<SkPoint, kNPts> fPathPts;
    SkSize fWinSize;

    const std::vector<DistFncMenuItem> fDefaultsDistFncs = {
            DistFncMenuItem("Linear", 1, true), DistFncMenuItem("Quadratic", 2, false),
            DistFncMenuItem("Cubic", 3, false), DistFncMenuItem("One Louder (11)", 11, false),
            DistFncMenuItem("30?!", 30, false)};
    std::vector<DistFncMenuItem> fDistFncs = fDefaultsDistFncs;
    std::vector<DistFncMenuItem> fDistFncsInner = fDefaultsDistFncs;
};

DEF_SLIDE(return new VariableWidthStrokerSlide;)
