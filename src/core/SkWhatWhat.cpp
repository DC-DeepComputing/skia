/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkWhatWhat.h"

#include "experimental/graphite/src/RecorderPriv.h"
#include "experimental/graphite/src/ResourceProvider.h"

SkShaderCodeDictionary* SkWhatWhat::dict() const {
    if (fRecorder) {
        return fRecorder->priv().resourceProvider()->shaderCodeDictionary();
    }

    return nullptr;
}
