/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPromiseImageTexture.h"

<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#if SK_SUPPORT_GPU
=======
#if defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)

SkPromiseImageTexture::SkPromiseImageTexture(const GrBackendTexture& backendTexture) {
    SkASSERT(backendTexture.isValid());
    fBackendTexture = backendTexture;
}

SkPromiseImageTexture::~SkPromiseImageTexture() {}

<<<<<<< HEAD   (86f93b Enforce IWYU on SkPicture-related files)
#endif // SK_SUPPORT_GPU
=======
#endif // defined(SK_GANESH)
>>>>>>> CHANGE (0f7b44 Use SK_GANESH and SK_GRAPHITE everywhere)
