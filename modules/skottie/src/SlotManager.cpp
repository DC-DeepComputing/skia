/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/SlotManager.h"

skottie::SlotManager::SlotManager() {}

void skottie::SlotManager::setColorSlot(SlotID slotID, SkColor c) {
    const auto it = fColorMap.find(slotID);
    if (it != fColorMap.end()) {
        for (auto& cPair : fColorMap[slotID]) {
            *(cPair.first) = c;
            cPair.second->invalidate();
        }
    }
}

void skottie::SlotManager::setImageSlot(SlotID slotID, sk_sp<SkImage> i) {

}

void skottie::SlotManager::setScalarSlot(SlotID slotID, SkScalar s) {
    const auto it = fScalarMap.find(slotID);
    if (it != fScalarMap.end()) {
        for (auto& sPair : fScalarMap[slotID]) {
            *(sPair.first) = s;
            sPair.second->invalidate();
        }
    }
}

SkColor skottie::SlotManager::getColorSlot(SlotID slotID) {
    const auto it = fColorMap.find(slotID);
    if (it != fColorMap.end()) {
        return *(fColorMap[slotID].at(0).first);
    }
    return SK_ColorBLACK;
}

sk_sp<SkImage> skottie::SlotManager::getImageSlot(SlotID slotID) {
    return nullptr;
}

SkScalar skottie::SlotManager::getScalarSlot(SlotID slotID) {
    const auto it = fScalarMap.find(slotID);
    if (it != fScalarMap.end()) {
        return *(fScalarMap[slotID].at(0).first);
    }
    return -1;
}

void skottie::SlotManager::trackColorValue(SlotID slotID, SkColor* colorValue,
                                           sk_sp<sksg::Node> node) {
    ValuePair<SkColor*> p = std::make_pair(colorValue, node);
    fColorMap[slotID].push_back(p);
}

void skottie::SlotManager::trackImageValue(SlotID slotID, sk_sp<skresources::ImageAsset> imageAsset,
                                           sk_sp<sksg::Node> node) {

}
void skottie::SlotManager::trackScalarValue(SlotID slotID, SkScalar* scalarValue,
                                            sk_sp<sksg::Node> node) {
    ValuePair<SkScalar*> p = std::make_pair(scalarValue, node);
    fScalarMap[slotID].push_back(p);
}
