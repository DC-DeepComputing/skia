/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// GM to stress the GPU font cache
// It's not necessary to run this with CPU configs

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

using MaskFormat = skgpu::MaskFormat;

static SkScalar draw_string(SkCanvas* canvas, const SkString& text, SkScalar x,
                           SkScalar y, const SkFont& font) {
    SkPaint paint;
    canvas->drawString(text, x, y, font, paint);
    return x + font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8);
}

class FontCacheGM : public skiagm::GM {
public:
    FontCacheGM(GrContextOptions::Enable allowMultipleTextures)
        : fAllowMultipleTextures(allowMultipleTextures) {
        this->setBGColor(SK_ColorLTGRAY);
    }

    void modifyGrContextOptions(GrContextOptions* options) override {
        options->fGlyphCacheTextureMaximumBytes = 0;
        options->fAllowMultipleGlyphCacheTextures = fAllowMultipleTextures;
    }

protected:
    SkString getName() const override {
        SkString name("fontcache");
        if (GrContextOptions::Enable::kYes == fAllowMultipleTextures) {
            name.append("-mt");
        }
        return name;
    }

    SkISize getISize() override { return SkISize::Make(kSize, kSize); }

    void onOnceBeforeDraw() override {
        fTypefaces[0] = ToolUtils::CreateTypefaceFromResource("fonts/Nabla-Regular-VariableFont.ttf");

//        fTypefaces[0] = ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Italic());
    }

    void onDraw(SkCanvas* canvas) override {
        this->drawText(canvas);
        //  Debugging tool for GPU.
        static const bool kShowAtlas = false;
        if (kShowAtlas) {
            if (auto dContext = GrAsDirectContext(canvas->recordingContext())) {
                auto img = dContext->priv().testingOnly_getFontAtlasImage(MaskFormat::kA8);
                canvas->drawImage(img, 0, 0);
            }
        }
    }

private:
    void drawText(SkCanvas* inputCanvas) {
        static const int kSizes[] = {8, 9, 10, 11, 12, 13, 18, 20, 25, 30, 40, 60, 80};

        static const SkString kTexts[] = {SkString("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
                                          SkString("abcdefghijklmnopqrstuvwxyz"),
                                          SkString("0123456789"),
                                          SkString("!@#$%^&*()<>[]{}")};
        SkFont font = ToolUtils::DefaultPortableFont();
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setSubpixel(true);

        auto ctx = inputCanvas->recordingContext();
        SkISize isize = getISize();
        SkImageInfo info = SkImageInfo::MakeN32(isize.width(), isize.height(), kPremul_SkAlphaType,
                                                SkColorSpace::MakeSRGBLinear());
        SkSurfaceProps inputProps;
        inputCanvas->getProps(&inputProps);
        SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag | inputProps.flags(),
                             inputProps.pixelGeometry());
        auto surface(SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info, 0, &props));
        SkCanvas* canvas = surface ? surface->getCanvas() : inputCanvas;

        static const SkScalar kSubPixelInc = 1 / 2.f;
        SkScalar x = 0;
        SkScalar y = 10;
        SkScalar subpixelX = 0;
        SkScalar subpixelY = 0;
        bool offsetX = true;

        if (GrContextOptions::Enable::kYes == fAllowMultipleTextures) {
            canvas->scale(10, 10);
        }

            for (auto s : kSizes) {
                auto size = 2 * s;
                font.setSize(size);
                for (const auto& typeface : fTypefaces) {
                    font.setTypeface(typeface);
                    for (const auto& text : kTexts) {
                        x = size + draw_string(canvas, text, x + subpixelX, y + subpixelY, font);
                        x = SkScalarCeilToScalar(x);
                        if (x + 100 > kSize) {
                            x = 0;
                            y += SkScalarCeilToScalar(size + 3);
                            if (y > kSize) {
                                break;
                            }
                        }
                    }
                }
                (offsetX ? subpixelX : subpixelY) += kSubPixelInc;
                offsetX = !offsetX;
            }

            // render offscreen buffer
            if (surface) {
                SkAutoCanvasRestore acr(inputCanvas, true);
                // since we prepended this matrix already, we blit using identity
                inputCanvas->resetMatrix();
                inputCanvas->drawImage(surface->makeImageSnapshot().get(), 0, 0);
            }

    }

    inline static constexpr SkScalar kSize = 1280;

    GrContextOptions::Enable fAllowMultipleTextures;
    sk_sp<SkTypeface> fTypefaces[1];
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new FontCacheGM(GrContextOptions::Enable::kNo))
DEF_GM(return new FontCacheGM(GrContextOptions::Enable::kYes))
