/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AndroidRuntimeEffectManager_DEFINED
#define AndroidRuntimeEffectManager_DEFINED

#include "include/core/SkRefCnt.h"

#include <map>

class SkRuntimeEffect;

namespace ui {
    enum class Dataspace {
        UNKNOWN,
        SRGB,
        BT2020,
        BT2020_ITU_PQ,
        DISPLAY_P3,
        V0_SRGB,
    };
}

namespace shaders {
    namespace LinearEffect {
        enum class SkSLType {
            Shader,
        };
    }
}

struct foo1 {
    ui::Dataspace inputDataspace;
    ui::Dataspace outputDataspace;
    bool undoPremultipliedAlpha;
    ui::Dataspace fakeOutputDataspace;
    shaders::LinearEffect::SkSLType type;
};

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


    sk_sp<SkRuntimeEffect> getOrCreateLinearRuntimeEffect(foo1);

private:
    sk_sp<SkRuntimeEffect> fMixEffect;
    sk_sp<SkRuntimeEffect> fEdgeExtensionEffect;
    sk_sp<SkRuntimeEffect> fKawaseHighSampleEffect;
    sk_sp<SkRuntimeEffect> fKawaseLowSampleEffect;
    sk_sp<SkRuntimeEffect> fBlurEffect;
    sk_sp<SkRuntimeEffect> fCrosstalkAndChunk16x16Effect;
    sk_sp<SkRuntimeEffect> fChunk8x8Effect;
    sk_sp<SkRuntimeEffect> fToneMapEffect;

    std::map<std::string, sk_sp<SkRuntimeEffect>> fLinearEffects;
};

#endif // AndroidRuntimeEffectManager_DEFINED
