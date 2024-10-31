/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "imgui.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPathUtils.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkPathPriv.h"
#include "tools/viewer/ClickHandlerSlide.h"

#include <stack>
#include <vector>

static void extrude_quad(SkPoint start, SkPoint control, SkPoint end,
                           SkPathBuilder* inner, SkPathBuilder* outer, float radius) {
    SkVector normalUnitStart = control - start;
    normalUnitStart.normalize();
    SkPointPriv::RotateCCW(&normalUnitStart);

    SkVector normalUnitEnd = end - control;
    normalUnitEnd.normalize();
    SkPointPriv::RotateCCW(&normalUnitEnd);

    SkVector normalUnitMid = normalUnitStart + normalUnitEnd;
    normalUnitMid.normalize();

    if (outer->empty()) {
        outer->moveTo(start + normalUnitStart * radius);
    }
    else {
        // This is where join logic would go
        outer->lineTo(start + normalUnitStart * radius);
    }
    outer->quadTo(control + normalUnitMid * radius, end + normalUnitEnd * radius);


    if (inner->empty()) {
        inner->moveTo(start - normalUnitStart * radius);
    }
    else {
        // This is where join logic would go
        inner->lineTo(start - normalUnitStart * radius);
    }
    inner->quadTo(control - normalUnitMid * radius, end - normalUnitEnd * radius);
}

static SkPath extrude_path(const SkPath& input, float width) {
    static SkOnce once;
    once([input](){input.dump();});
    SkPathBuilder inner, outer;

    SkPath::Iter iter(input, false);
    float radius = width / 2;

    SkPoint pts[4];
    for (auto verb = iter.next(pts); verb != SkPath::kDone_Verb; verb = iter.next(pts)) {
        switch (verb) {
            case SkPath::kMove_Verb:
                break;
            case SkPath::kQuad_Verb:
                extrude_quad(pts[0], pts[1], pts[2], &inner, &outer, radius);
                break;
            default:
                SK_ABORT("not implemented\n");
        }
    }

    // existing algo
    // SkPaint p;
    // p.setAntiAlias(true);
    // p.setStyle(SkPaint::kStroke_Style);
    // p.setStrokeWidth(width);
    // SkStrokeRec rec(p);
    // SkPath fillPath;
    // rec.applyToPath(&fillPath, input);
    // return fillPath;
    outer.lineTo(inner.getLastPt());
    SkPathPriv::ReverseAddPath(&outer, inner.detach());
    outer.close();
    SkPath output = outer.detach();
    static SkOnce twice;
    twice([output](){output.dump();});
    return output;
}

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
        fStrokePaint.setColor(0x80FF0000);

        fNewFillPaint.setAntiAlias(true);
        fNewFillPaint.setColor(0x8000FF00);

        fHiddenPaint.setAntiAlias(true);
        fHiddenPaint.setStyle(SkPaint::kStroke_Style);
        fHiddenPaint.setColor(0xFF0000FF);

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
        canvas->drawColor(0xFFEEEEEE);

        SkPath path;
        this->makePath(&path);

        fStrokePaint.setStrokeWidth(fWidth);

        // The current result
        if (fShowSkiaStroke) {
            canvas->drawPath(path, fStrokePaint);
        }

        SkPath fillPath = extrude_path(path, fWidth);
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
