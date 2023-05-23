/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/SlotManager.h"

skottie::SlotManager::SlotManager() {}

void skottie::SlotManager::setColorSlot(SlotID slotID, SkColor c) {

}

void skottie::SlotManager::setImageSlot(SlotID slotID, sk_sp<SkImage> i) {

}

void skottie::SlotManager::setScalarSlot(SlotID slotID, SkScalar s) {

}

SkColor skottie::SlotManager::getColorSlot(SlotID slotID) {
    return SK_ColorBLACK;
}

sk_sp<SkImage> skottie::SlotManager::getImageSlot(SlotID slotID) {
    return nullptr;
}

SkScalar skottie::SlotManager::getScalarSlot(SlotID slotID) {
    return 0;
}

void skottie::SlotManager::trackColorValue(SlotID slotID, SkColor* colorValue,
                                           sk_sp<sksg::Node> node) {

}

void skottie::SlotManager::trackImageValue(SlotID slotID, sk_sp<skresources::ImageAsset> imageAsset,
                                           sk_sp<sksg::Node> node) {

}
void skottie::SlotManager::trackScalarValue(SlotID slotID, SkScalar* colorValue,
                                            sk_sp<sksg::Node> node) {

}
