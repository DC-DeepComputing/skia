/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkSurfaceSpy.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"

SkSurfaceSpy::SkSurfaceSpy(int width, int height, const SkSurfaceProps* props)
    : SkSurface_Base(width, height, props) {}

SkSurfaceSpy::SkSurfaceSpy(const SkImageInfo& info, const SkSurfaceProps* props)
    : SkSurface_Base(info, props) {}

SkCanvas* SkSurfaceSpy::onNewCanvas() {
    if (onNewCanvasCallback) {
        return onNewCanvasCallback(this->imageInfo());
    }
    return new SkCanvas(this->width(), this->height());
}

sk_sp<SkSurface> SkSurfaceSpy::onNewSurface(const SkImageInfo& info) {
    return sk_make_sp<SkSurfaceSpy>(info, &this->props());
}

sk_sp<SkImage> SkSurfaceSpy::onNewImageSnapshot(const SkIRect* bounds) {
    if (onNewImageSnapshotCallback) {
        return onNewImageSnapshotCallback(bounds);
    }
    return nullptr;
}

bool SkSurfaceSpy::onCopyOnWrite(ContentChangeMode mode) {
    if (onCopyOnWriteCallback) {
        return onCopyOnWriteCallback(mode);
    }
    return true;
}

void SkSurfaceSpy::onWritePixels(const SkPixmap& pixmap, int x, int y) {
    if (onWritePixelsCallback) {
        onWritePixelsCallback(pixmap, x, y);
    }
}
