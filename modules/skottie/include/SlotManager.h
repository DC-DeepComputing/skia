/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SlotManager_DEFINED
#define SlotManager_DEFINED

#include "include/core/SkImage.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/sksg/include/SkSGNode.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace skjson {
class ObjectValue;
}
namespace skottie {

namespace internal {
class AnimationBuilder;
} // namespace internal

class SK_API SlotManager final : public SkRefCnt {

public:

    using SlotID = std::string;

    SlotManager();

    void setColorSlot(SlotID, SkColor);
    void setImageSlot(SlotID, sk_sp<SkImage>);
    void setScalarSlot(SlotID, SkScalar);
    //TODO: surface Text value options

    SkColor        getColorSlot(SlotID);
    sk_sp<SkImage> getImageSlot(SlotID);
    SkScalar       getScalarSlot(SlotID);

    struct SlotInfo {
        SlotID slotID;
        int type;
    };

    // Helper function to get all slot IDs and their value types
    const std::vector<SlotInfo>& getSlotInfo() const { return fSlotInfos; }

private:

    // pass value to the SlotManager for manipulation and node for invalidation
    void trackColorValue(SlotID,    SkColor*,                       sk_sp<sksg::Node>);
    void trackImageValue(SlotID,    sk_sp<skresources::ImageAsset>, sk_sp<sksg::Node>);
    void trackScalarValue(SlotID,   SkScalar*,                      sk_sp<sksg::Node>);

    std::vector<SlotInfo> fSlotInfos;

    template <typename T>
    using ValuePair = std::pair<T, sk_sp<sksg::Node>>;

    template <typename T>
    using SlotMap = std::unordered_map<SlotID, std::vector<ValuePair<T>>>;

    SlotMap<SkColor*>                       fColorMap;
    SlotMap<SkScalar*>                      fScalarMap;
    SlotMap<sk_sp<skresources::ImageAsset>> fImageMap;

    // Handle to "slots" JSON Object, used to grab slot values while building
    const skjson::ObjectValue* fSlotsRoot;

    friend class skottie::internal::AnimationBuilder;
};

} // namespace skottie

#endif // SlotManager_DEFINED
