/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkCubicMap.h"
#include "include/core/SkImage.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkVertices.h"
#include "tools/Resources.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/Slide.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

#include "imgui.h"

namespace {

float lerp(float a, float b, float t) {
    return a + (b - a)*t;
}

sk_sp<SkShader> make_image_shader(const char* resource) {
    sk_sp<SkImage> img = GetResourceAsImage(resource);

    // Normalize to 1x1 for UV sampling.
    const auto lm = SkMatrix::Scale(1.0f/img->width(), 1.0f/img->height());

    return img->makeShader(SkTileMode::kDecal,
                           SkTileMode::kDecal,
                           SkSamplingOptions(SkFilterMode::kLinear),
                           &lm);
}

struct ShaderFactory {
    const char*     fName;
    sk_sp<SkShader> (*fBuild)();
} gShaderFactories[] = {
    {
        "Img (Mandrill)",
        []() ->sk_sp<SkShader> { return make_image_shader("images/mandrill_512.png"); }
    },
    {
        "Img (Baby Tux)",
        []() ->sk_sp<SkShader> { return make_image_shader("images/baby_tux.png"); }
    },
};

struct VertexAnimator {
    const char*          fName;
    void (*fAanimate)(const std::vector<SkPoint>& uv, double ms, std::vector<SkPoint>& out);
} gVertexAnimators[] = {
    {
        "None",
        [](const std::vector<SkPoint>& uvs, double, std::vector<SkPoint>& out) { out = uvs; },
    },
    {
        "Squircilator",
        [](const std::vector<SkPoint>& uvs, double ms, std::vector<SkPoint>& out) {
            auto t = std::abs(std::fmod((ms / 1000), 2) - 1);

            SkCubicMap cm{{0.5f, 0}, {0.5f, 1}};
            t = cm.computeYFromX(t);

            for (size_t i = 0; i < uvs.size(); ++i) {
                const auto uv = (uvs[i] - SkPoint{0.5,0.5});

                const auto l = uv.length()*0.5f/std::max(std::abs(uv.fX), std::abs(uv.fY));

                out[i] = uv * lerp(1, (0.5f / l), t) + SkPoint{0.5, 0.5};
            }
        },
    }
};

class MeshSlide final : public Slide {
public:
    MeshSlide() { fName = "Mesh"; }

    void load(SkScalar w, SkScalar h) override {
        fSize = {w, h};

        this->initMesh();
        this->initShader(gShaderFactories[0]);
    }

    void resize(SkScalar w, SkScalar h) override { fSize = {w, h}; }

    void draw(SkCanvas* canvas) override {
        SkAutoCanvasRestore acr(canvas, true);

        SkPaint p;
        p.setAntiAlias(true);

        canvas->translate((fSize.fWidth  - kMeshSize.fWidth ) * 0.5f,
                          (fSize.fHeight - kMeshSize.fHeight) * 0.5f);
        canvas->scale(kMeshSize.fWidth, kMeshSize.fHeight);

        auto verts = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                          fVertexCount,
                                          fVertices.data(),
                                          fUVs.data(),
                                          nullptr,
                                          fIndices.size(),
                                          fIndices.data());
        p.setShader(fShader);
        canvas->drawVertices(verts, SkBlendMode::kSrcOver, p);

        if (fShowMesh) {
            p.setShader(nullptr);
            p.setColor(SK_ColorBLUE);
            p.setStroke(true);
            p.setStrokeWidth(0.5f/std::sqrt(kMeshSize.fWidth * kMeshSize.fHeight));

            SkASSERT(fIndices.size() % 6 == 0);
            for (auto i = fIndices.cbegin(); i < fIndices.cend(); i += 6) {
                canvas->drawLine(fVertices[i[0]], fVertices[i[1]], p);
                canvas->drawLine(fVertices[i[1]], fVertices[i[2]], p);
                canvas->drawLine(fVertices[i[2]], fVertices[i[0]], p);
                canvas->drawLine(fVertices[i[3]], fVertices[i[4]], p);
                canvas->drawLine(fVertices[i[4]], fVertices[i[5]], p);
                canvas->drawLine(fVertices[i[5]], fVertices[i[3]], p);
            }

            p.setStrokeCap(SkPaint::kRound_Cap);
            p.setStrokeWidth(5/std::sqrt(kMeshSize.fWidth * kMeshSize.fHeight));
            canvas->drawPoints(SkCanvas::kPoints_PointMode, fVertexCount, fVertices.data(), p);
        }

        this->drawControls();
    }

    bool animate(double nanos) override {
        fCurrentAnimator->fAanimate(fUVs, nanos*0.000001, fVertices);
        return true;
    }

private:
    void initMesh() {
        const auto n = static_cast<size_t>(std::sqrt(fVertexCount));
        SkASSERT(n > 0);
        SkASSERT(n == std::sqrt(fVertexCount));

        fVertices.resize(fVertexCount);
        fUVs.resize(fVertexCount);
        for (size_t i = 0; i < fVertexCount; ++i) {
            fVertices[i] = fUVs[i] = {
                static_cast<float>(i % n) / (n - 1),
                static_cast<float>(i / n) / (n - 1),
            };
        }

        const size_t triangle_count = 2*(n - 1)*(n - 1),
                        index_count = 3*triangle_count;

        fIndices.clear();
        fIndices.reserve(index_count);
        for (size_t i = 0; i < n - 1; ++i) {
            for (size_t j = 0; j < n - 1; ++j) {
                const auto ri0 = j*n + i,
                           ri1 = ri0 + n,
                           of0 = (i + j) % 2,
                           of1 = 1 - of0;

                fIndices.push_back(ri0 +   0);
                fIndices.push_back(ri0 +   1);
                fIndices.push_back(ri1 + of0);

                fIndices.push_back(ri0 + of1);
                fIndices.push_back(ri1 +   1);
                fIndices.push_back(ri1 +   0);
            }
        }

        SkASSERT(fIndices.size() == index_count);
    }

    void initShader(const ShaderFactory& fact) {
        fShader = fact.fBuild();
        fCurrentShaderFactory = &fact;
    }

    void drawControls() {
        ImGui::Begin("Mesh Options");

        if (ImGui::BeginCombo("##combo", fCurrentShaderFactory->fName)) {
            for (const auto& fact : gShaderFactories) {
                const auto is_selected = (fCurrentShaderFactory->fName == fact.fName);
                if (ImGui::Selectable(fact.fName) && !is_selected) {
                    this->initShader(fact);
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("##combo2", fCurrentAnimator->fName)) {
            for (const auto& anim : gVertexAnimators) {
                const auto is_selected = (fCurrentAnimator->fName == anim.fName);
                if (ImGui::Selectable(anim.fName) && !is_selected) {
                    fCurrentAnimator = &anim;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Checkbox("Show mesh", &fShowMesh);

        ImGui::End();
    }

    static constexpr SkSize kMeshSize = {512, 512};

    SkSize                  fSize;
    sk_sp<SkShader>         fShader;
    size_t                  fVertexCount = 256;
    std::vector<SkPoint>    fVertices,
                            fUVs;
    std::vector<uint16_t>   fIndices;

    // UI stuff

    const ShaderFactory*  fCurrentShaderFactory = &gShaderFactories[0];
    const VertexAnimator* fCurrentAnimator      = &gVertexAnimators[0];
    bool                  fShowMesh             = false;
};

}  // anonymous namespace

DEF_SLIDE(return new MeshSlide{};)
