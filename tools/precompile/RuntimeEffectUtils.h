/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RuntimeEffectUtils_DEFINED
#define RuntimeEffectUtils_DEFINED

#include "include/core/SkRefCnt.h"

class SkData;
class SkRuntimeEffect;

namespace RuntimeEffectUtils {

struct Result {
    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkData> fUniforms;
};

// Creates a Blender SkRuntimeEffect that just returns the source color
void SrcBlender(Result*);
// Creates a Blender SkRuntimeEffect that just returns the destination color
void DestBlender(Result*);
// Creates a Blender SkRuntimeEffect that just lerps between two child Blenders based on a
// float uniform
void ComboBlender(Result*);

} // namespace RuntimeEffectUtils

#endif // RuntimeEffectUtils_DEFINED
