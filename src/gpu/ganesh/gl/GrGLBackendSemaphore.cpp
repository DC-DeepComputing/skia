/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/gl/GrGLBackendSemaphore.h"

#include "include/gpu/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrBackendSemaphorePriv.h"

class GrGLBackendSemaphoreData final : public GrBackendSemaphoreData {
public:
    GrGLBackendSemaphoreData(GrGLsync sync) : fSync(sync) {}

    GrGLsync sync() const { return fSync; }
private:
    void copyTo(AnySemaphoreData& data) const override {
        data.emplace<GrGLBackendSemaphoreData>(fSync);
}

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kOpenGL; }
#endif

    GrGLsync fSync;
};

static const GrGLBackendSemaphoreData* get_and_cast_data(const GrBackendSemaphore& sem) {
    auto data = GrBackendSemaphorePriv::GetBackendData(sem);
    SkASSERT(!data || data->type() == GrBackendApi::kOpenGL);
    return static_cast<const GrGLBackendSemaphoreData*>(data);
}

namespace GrBackendSemaphores {
GrBackendSemaphore MakeGL(GrGLsync sync) {
    GrGLBackendSemaphoreData data(sync);
    return GrBackendSemaphorePriv::MakeGrBackendSemaphore(GrBackendApi::kOpenGL,
                                                          data);
}

GrGLsync GetGLSync(const GrBackendSemaphore& sem) {
    SkASSERT(sem.backend() == GrBackendApi::kOpenGL);
    const GrGLBackendSemaphoreData* data = get_and_cast_data(sem);
    SkASSERT(data);
    return data->sync();
}
}  // namespace GrBackendSemaphores
