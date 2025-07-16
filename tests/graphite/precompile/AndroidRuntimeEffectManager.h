/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AndroidRuntimeEffectManager_DEFINED
#define AndroidRuntimeEffectManager_DEFINED

#include "include/core/SkRefCnt.h"

class SkRuntimeEffect;

namespace ui {
    enum Class {
        UNKNOWN,
        SRGB,
    };
}

class RuntimeEffectManager {
public:
    RuntimeEffectManager();

    enum class KnownId {
        kBlurFilter_MixEffect,
        kEdgeExtensionEffect,
        kKawaseBlurDualFilter_HighSampleBlurEffect,
        kKawaseBlurDualFilter_LowSampleBlurEffect,
        kMouriMap_BlurEffect,
        kMouriMap_CrossTalkAndChunk16x16Effect,
        kMouriMap_Chunk8x8Effect,
        kMouriMap_TonemapEffect,
    };

    sk_sp<SkRuntimeEffect> getKnownRuntimeEffect(KnownId id);

    void getOrCreateLinearRuntimeEffect();

private:
    sk_sp<SkRuntimeEffect> fMixEffect;
    sk_sp<SkRuntimeEffect> fEdgeExtensionEffect;
    sk_sp<SkRuntimeEffect> fKawaseHighSampleEffect;
    sk_sp<SkRuntimeEffect> fKawaseLowSampleEffect;
    sk_sp<SkRuntimeEffect> fBlurEffect;
    sk_sp<SkRuntimeEffect> fCrosstalkAndChunk16x16Effect;
    sk_sp<SkRuntimeEffect> fChunk8x8Effect;
    sk_sp<SkRuntimeEffect> fToneMapEffect;
};

#endif // AndroidRuntimeEffectManager_DEFINED
