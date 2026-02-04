/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AndroidRuntimeEffectManager_DEFINED
#define AndroidRuntimeEffectManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/effects/SkRuntimeEffect.h"

#include <map>
#include <string>

namespace ui {

enum class Dataspace : int32_t {
    UNKNOWN = 0,
    SRGB = 513 /* 0x201 */,
    BT2020 = 147193856 /* STANDARD_BT2020 | TRANSFER_SMPTE_170M | RANGE_FULL */,
    BT2020_ITU_PQ = 298188800 /* STANDARD_BT2020 | TRANSFER_ST2084 | RANGE_LIMITED */,
    BT2020_HLG = 168165376 /* STANDARD_BT2020 | TRANSFER_HLG | RANGE_FULL */,
    DISPLAY_P3 = 143261696 /* STANDARD_DCI_P3 | TRANSFER_SRGB | RANGE_FULL */,
    V0_SRGB = 142671872 /* STANDARD_BT709 | TRANSFER_SRGB | RANGE_FULL */,
};

} // namespace ui

enum class HdrRenderType {
    SDR,
    DISPLAY_HDR,
    GENERIC_HDR
};

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

// Keep in sync with RuntimeEffects.inl
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

    sk_sp<SkRuntimeEffect> getKnownRuntimeEffect(KnownId id) {
        switch (id) {
#define M(id) case KnownId::k##id : return f##id;
            SK_ALL_ANDROID_KNOWN_EFFECTS(M)
#undef M
        }

        SkUNREACHABLE;
    }

    sk_sp<SkRuntimeEffect> getOrCreateLinearRuntimeEffect(const shaders::LinearEffect&);

private:
#define M(id) sk_sp<SkRuntimeEffect> f##id;
    SK_ALL_ANDROID_KNOWN_EFFECTS(M)
#undef M

    std::map<std::string, sk_sp<SkRuntimeEffect>> fLinearEffects;
};

#endif // AndroidRuntimeEffectManager_DEFINED
