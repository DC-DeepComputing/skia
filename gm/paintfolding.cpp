/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkArc.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"

namespace skiagm {

constexpr int kTileSize = 32;
const int kPad = 4;

// TODO: remove this
constexpr int kNumCases = 8;

namespace {

sk_sp<SkSurface> create_surface(SkCanvas* canvas, sk_sp<SkColorSpace> cs) {
    SkImageInfo ii = SkImageInfo::Make(kTileSize, kTileSize,
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType,
                                       std::move(cs));

    sk_sp<SkSurface> surface = canvas->makeSurface(ii);
    if (!surface) {
        // could be a recording canvas.
        surface = SkSurfaces::Raster(ii);
    }

    return surface;
}

void draw_paint(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawPaint(paint);
}

void draw_rect(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawRect(SkRect::MakeWH(kTileSize, kTileSize), paint);
}

void draw_rrect(SkCanvas* canvas, const SkPaint& paint) {
    canvas->drawRRect(SkRRect::MakeRectXY({ 0, 0, kTileSize, kTileSize }, 4, 4), paint);
}

void draw_arc(SkCanvas* canvas, const SkPaint& paint) {
    SkArc arc = SkArc::Make({ -kTileSize, -kTileSize, kTileSize, kTileSize},
                            0, 270, SkArc::Type::kWedge);

    canvas->drawArc(arc, paint);
}

SkPath create_dented_rect() {
    SkPathBuilder b;
    b.moveTo(0, 0);
    b.lineTo(kTileSize/2, 1);
    b.lineTo(kTileSize, 0);
    b.lineTo(kTileSize, kTileSize);
    b.lineTo(0, kTileSize);
    b.close();
    return b.detach();
}

void draw_path(SkCanvas* canvas, const SkPaint& paint) {
    SkPath p = create_dented_rect();

    canvas->drawPath(p, paint);
}

sk_sp<SkColorSpace> make_knockout(SkColorSpace* cs, int index) {
    skcms_Matrix3x3 knockoutMat = {{
                                    { 1, 0, 0 },
                                    { 0, 1, 0 },
                                    { 0, 0, 1 },
                            }};

    knockoutMat.vals[index][0] = knockoutMat.vals[index][1] = knockoutMat.vals[index][2] = 0;

    skcms_Matrix3x3 tmpXYZD50;
    cs->toXYZD50(&tmpXYZD50);
    skcms_TransferFunction tmpTransferFn;
    cs->transferFn(&tmpTransferFn);

    skcms_Matrix3x3 spun = skcms_Matrix3x3_concat(&tmpXYZD50, &knockoutMat);

    return SkColorSpace::MakeRGB(tmpTransferFn, spun);
}

#if 0
static sk_sp<SkColorFilter> make_color_filter() {
    float colorMatrix[20] = {
            1, 0, 0,   0,   0,
            0, 1, 0,   0,   0,
            0, 0, 0.5, 0.5, 0,
            0, 0, 0.5, 0.5, 0}; // mix G and A.
    return SkColorFilters::Matrix(colorMatrix);
}

static sk_sp<SkColorFilter> make_invert_filter() {
    static const float matrix[20] = {-1.f, 0.f, 0.f, 0.f, 1.f,
                                     0.f, -1.f, 0.f, 0.f, 1.f,
                                     0.f, 0.f, -1.f, 0.f, 1.f,
                                     0.f, 0.f, 0.f, 1.f, 0.f};
    return SkColorFilters::Matrix(matrix);
}
#endif

} // anonymous namespace

class PaintFoldingGM : public GM {
public:
    PaintFoldingGM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:
    typedef void (*PFDrawMth)(SkCanvas*, const SkPaint&);
    static constexpr PFDrawMth kDrawProcs[] = {
            draw_paint,
            draw_rect,
            draw_rrect,
            draw_arc,
            draw_path
    };

    SkString getName() const override { return SkString("paintfolding"); }

    SkISize getISize() override {
        return SkISize::Make(kPad + std::size(kDrawProcs) * (kTileSize+kPad),
                             kPad + kNumCases * (kTileSize+kPad));
    }

    void onDraw(SkCanvas* canvas) override {

        sk_sp<SkColorSpace> srgbCS = SkColorSpace::MakeSRGB();
        sk_sp<SkColorSpace> knockoutR = make_knockout(srgbCS.get(), 0);
        sk_sp<SkColorSpace> knockoutG = make_knockout(srgbCS.get(), 1);
        sk_sp<SkColorSpace> knockoutB = make_knockout(srgbCS.get(), 2);
        sk_sp<SkColorSpace> spinCS = srgbCS->makeColorSpin();

        // The same as the color spin ColorSpace: R->B, G->R, B->R
        static constexpr float kSpinMatrix[] = { 0, 1, 0, 0, 0,
                                                 0, 0, 1, 0, 0,
                                                 1, 0, 0, 0, 0,
                                                 0, 0, 0, 1, 0  };
        sk_sp<SkColorFilter> spinMatrixCF = SkColorFilters::Matrix(kSpinMatrix);

        constexpr SkColor4f kTransWhite = { 1, 1, 1, 0.5f };

#if 0
        {
            SkPaint p;
            p.setColor(SK_ColorRED);
            p.setColorFilter(spinMatrixCF);

            canvas->drawRect({100, 100, 110, 110}, p);
            p.setColor(SK_ColorGREEN);
            canvas->drawRect({110, 110, 120, 120}, p);
            p.setColor(SK_ColorBLUE);
            canvas->drawRect({120, 120, 130, 130}, p);
        }
#endif

        struct {
            SkColor4f fPaint;
            SkColor4f fShader;
            sk_sp<SkColorFilter> fColorFilter;
        } cases[kNumCases] = {
            { SkColors::kWhite, SkColors::kWhite, nullptr },
            { kTransWhite,      SkColors::kWhite, nullptr },
            { SkColors::kWhite, kTransWhite,      nullptr },
            { kTransWhite,      kTransWhite,      nullptr },

            { SkColors::kWhite, SkColors::kWhite, spinMatrixCF },
            { kTransWhite,      SkColors::kWhite, spinMatrixCF },
            { SkColors::kWhite, kTransWhite,      spinMatrixCF },
            { kTransWhite,      kTransWhite,      spinMatrixCF },
        };

        int yOff = kPad;
        for (auto c : cases) {
            SkPaint paint;
            paint.setColor(c.fPaint, knockoutB.get());               // yellow
            paint.setShader(SkShaders::Color(c.fShader, knockoutG)); // magenta
            paint.setColorFilter(c.fColorFilter);                    // magenta -> cyan

            int xOff = kPad;
            for (auto d : kDrawProcs) {
                // spins:
                //    cyan    -> yellow
                //    magenta -> cyan
                //    yellow  -> magenta
                sk_sp<SkSurface> s = create_surface(canvas, spinCS);
                d(s->getCanvas(), paint);
                canvas->drawImage(s->makeImageSnapshot(), xOff, yOff);
                xOff += kTileSize + kPad;
            }

            yOff += kTileSize + kPad;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new PaintFoldingGM;)
}  // namespace skiagm
