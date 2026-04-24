/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfaceSpy_DEFINED
#define SkSurfaceSpy_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "src/image/SkSurface_Base.h"

#include <functional>

class SkCanvas;

/**
 *  SkSurfaceSpy is a surface that allows for spying on internal surface events.
 *  It is intended for testing and debugging purposes only.
 */
class SkSurfaceSpy : public SkSurface_Base {
public:
    SkSurfaceSpy(int width, int height, const SkSurfaceProps* props = nullptr);
    SkSurfaceSpy(const SkImageInfo& info, const SkSurfaceProps* props = nullptr);

    // From SkSurface_Base
    Type type() const override { return Type::kNull; }

    // Callbacks for internal events
    std::function<SkCanvas*(const SkImageInfo&)>  onNewCanvasCallback;
    std::function<sk_sp<SkImage>(const SkIRect*)> onNewImageSnapshotCallback;
    std::function<bool(ContentChangeMode)>        onCopyOnWriteCallback;
    std::function<void(const SkPixmap&, int, int)> onWritePixelsCallback;

protected:
    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo& info) override;
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* bounds) override;
    bool onCopyOnWrite(ContentChangeMode mode) override;
    void onWritePixels(const SkPixmap& pixmap, int x, int y) override;

private:
    using INHERITED = SkSurface_Base;
};

#endif
