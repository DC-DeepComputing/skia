/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/GrPromiseImageTexture.h"

GrPromiseImageTexture::GrPromiseImageTexture(GrBackendTexture&& backendTexture) {
    SkASSERT(backendTexture.isValid());
    fBackendTexture = std::move(backendTexture);
}

GrPromiseImageTexture::~GrPromiseImageTexture() {}
