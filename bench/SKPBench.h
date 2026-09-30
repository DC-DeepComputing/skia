/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKPBench_DEFINED
#define SKPBench_DEFINED

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/private/SkTDArray.h"

class SkSurface;

/**
 * Runs an SkPicture as a benchmark by repeatedly drawing it scaled inside a device clip.
 */
class SKPBench : public Benchmark {
public:
    SKPBench(const char* name, const SkPicture*, const SkIRect& devClip,
             SkScalar scale, const SkISize& tileSize, bool doLooping);
    ~SKPBench() override;

    bool shouldLoop() const override {
        return fDoLooping;
    }

    // Returns true if a non-empty tile size is set, indicating the picture should be rendered in tiles.
    bool shouldTile() const {
        return !fTileSize.isEmpty();
    }

    void getGpuStats(SkCanvas*,
                     skia_private::TArray<SkString>* keys,
                     skia_private::TArray<double>* values) override;
    bool getDMSAAStats(GrRecordingContext*) override;

protected:
    const char* onGetName() override;
    const char* onGetUniqueName() override;
    void onPerCanvasPreDraw(SkCanvas*) override;
    void onPerCanvasPostDraw(SkCanvas*) override;
    bool isSuitableFor(Backend backend) override;
    void onDraw(int loops, SkCanvas*) override { SkASSERT(false); }
    void onDrawFrame(int loops, SkCanvas*, std::function<void()> submitFrame) override;
    SkISize onGetSize() override;

    virtual void drawPicture();

    const SkPicture* picture() const { return fPic.get(); }

    struct TileInfo {
        TileInfo(sk_sp<SkPicture> picture, sk_sp<SkSurface> surface,
                 const SkIRect& tileRect, const SkRect& clipRect, const SkM44& mat)
            : fPicture(std::move(picture))
            , fSurface(std::move(surface))
            , fTileRect(tileRect)
            , fClipRect(clipRect)
            , fMat(mat) {}

        SkPicture* picture() const { return fPicture.get(); }
        SkSurface* surface() const { return fSurface.get(); }
        SkIRect tileRect() const { return fTileRect; }
        SkRect clipRect() const { return fClipRect; }
        const SkM44& mat() const { return fMat; }

    private:
        sk_sp<SkPicture> fPicture;
        sk_sp<SkSurface> fSurface;
        SkIRect fTileRect;
        SkRect fClipRect;
        SkM44 fMat;
    };

    const skia_private::TArray<TileInfo>& tileInfo() const { return fTiles; }

private:
    void createTiles(SkCanvas* canvas, const SkIRect& bounds);
    bool submitsInternalFrames() override { return true; }

    sk_sp<const SkPicture> fPic;
    const SkIRect fClip;
    const SkScalar fScale;
    SkString fName;
    SkString fUniqueName;
    const SkISize fTileSize;

    skia_private::TArray<TileInfo> fTiles;

    const bool fDoLooping;

    using INHERITED = Benchmark;
};

#endif
