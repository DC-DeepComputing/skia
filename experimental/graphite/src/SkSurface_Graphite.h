/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Graphite_DEFINED
#define SkSurface_Graphite_DEFINED

#include "src/image/SkSurface_Base.h"

namespace skgpu {

class Device;

class SkSurface_Graphite final : public SkSurface_Base {
public:
    SkSurface_Graphite(sk_sp <Device>);
    ~SkSurface_Graphite() override;

    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    void onWritePixels(const SkPixmap&, int x, int y) override;
    void onCopyOnWrite(ContentChangeMode) override;

private:
    sk_sp<Device> fDevice;
};

} // namespace skgpu

#endif // SkSurface_Graphite_DEFINED
