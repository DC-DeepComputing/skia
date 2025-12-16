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
#include <string>

class SkRuntimeEffect;

namespace ui {

enum class Dataspace : int32_t {
    UNKNOWN,
    SRGB,
    BT2020,
    BT2020_ITU_PQ,
    DISPLAY_P3,
    V0_SRGB,
};

} // namespace ui

namespace shaders {

struct LinearEffect {
    ui::Dataspace inputDataspace = ui::Dataspace::SRGB;
    ui::Dataspace outputDataspace = ui::Dataspace::SRGB;
    bool undoPremultipliedAlpha = false;
    ui::Dataspace fakeOutputDataspace = ui::Dataspace::UNKNOWN;

    enum SkSLType { Shader, ColorFilter };
    SkSLType type = Shader;
};

} // namespace shaders

#define SK_ALL_ANDROID_KNOWN_EFFECTS(M) \
    M(BlurFilter_MixEffect) \
    M(EdgeExtensionEffect) \
    M(GainmapEffect) \
    M(KawaseBlurDualFilterV2_QuarterResDownSampleBlurEffect) \
    M(KawaseBlurDualFilterV2_HalfResDownSampleBlurEffect) \
    M(KawaseBlurDualFilterV2_UpSampleBlurEffect) \
    M(KawaseBlurEffect) \
    M(LutEffect) \
    M(MouriMap_CrossTalkAndChunk16x16Effect) \
    M(MouriMap_Chunk8x8Effect) \
    M(MouriMap_BlurEffect) \
    M(MouriMap_TonemapEffect) \
    M(StretchEffect) \
    M(BoxShadowEffect)

class RuntimeEffectManager {
public:
    RuntimeEffectManager();

    enum class KnownId {
#define M(id) k ## id,
        SK_ALL_ANDROID_KNOWN_EFFECTS(M)
#undef M
    };

    sk_sp<SkRuntimeEffect> getKnownRuntimeEffect(KnownId id);

    sk_sp<SkRuntimeEffect> getOrCreateLinearRuntimeEffect(const shaders::LinearEffect&);

private:
    sk_sp<SkRuntimeEffect> fBlurFilter_MixEffect;
    sk_sp<SkRuntimeEffect> fEdgeExtensionEffect;
    sk_sp<SkRuntimeEffect> fGainmapEffect;
    sk_sp<SkRuntimeEffect> fKawaseBlurDualFilterV2_QuarterResDownSampleBlurEffect;
    sk_sp<SkRuntimeEffect> fKawaseBlurDualFilterV2_HalfResDownSampleBlurEffect;
    sk_sp<SkRuntimeEffect> fKawaseBlurDualFilterV2_UpSampleBlurEffect;
    sk_sp<SkRuntimeEffect> fKawaseBlurEffect;
    sk_sp<SkRuntimeEffect> fLutEffect;
    sk_sp<SkRuntimeEffect> fMouriMap_CrossTalkAndChunk16x16Effect;
    sk_sp<SkRuntimeEffect> fMouriMap_Chunk8x8Effect;
    sk_sp<SkRuntimeEffect> fMouriMap_BlurEffect;
    sk_sp<SkRuntimeEffect> fMouriMap_TonemapEffect;
    sk_sp<SkRuntimeEffect> fStretchEffect;
    sk_sp<SkRuntimeEffect> fBoxShadowEffect;

    std::map<std::string, sk_sp<SkRuntimeEffect>> fLinearEffects;
};

#endif // AndroidRuntimeEffectManager_DEFINED
