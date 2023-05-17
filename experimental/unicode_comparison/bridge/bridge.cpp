// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bridge.h"
#include "../../../modules/skunicode/include/SkUnicode.h"
#include "../../../include/core/SkString.h"

namespace {
static std::unique_ptr<SkUnicode> gUnicode = nullptr;
static skia_private::TArray<SkUnicode::CodeUnitFlags, true> gCodeUnitFlags;
}

bool make_unicode() {
    gUnicode = SkUnicode::Make();
    auto ptr = reinterpret_cast<void*>(gUnicode.get());
    return ptr != nullptr;
}

void destroy_unicode() {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist");
        return;
    }
    delete gUnicode.get();
}

void  destroy_str(void* str) {
    auto ptr = reinterpret_cast<SkString*>(str);
    delete ptr;
}

int initialize(char* text) {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist");
        return -1;
    }
    if (!gUnicode->computeCodeUnitFlags(text, strlen(text), false, &gCodeUnitFlags)) {
        return -1;
    }
    return gCodeUnitFlags.size();
}

bool  getFlags(int index, void* flags) {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist");
        return false;
    }
    if (index < 0 || index >= gCodeUnitFlags.size()) {
        if (gCodeUnitFlags.size() == 0) {
            SkDebugf("Unicode object is empty or not initialized");
        } else {
            SkDebugf("Unicode object is not initialized");
        }
        return false;
    }
    auto ptr = reinterpret_cast<SkUnicode::CodeUnitFlags*>(flags);
    SkASSERT(ptr);
    *ptr = gCodeUnitFlags[index];
    return true;
}

void* upper(char* str) {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist");
        return nullptr;
    }
    auto res = new SkString(gUnicode->toUpper(SkString(str)));
    return reinterpret_cast<void*>(res);
}

void  print(void* str) {
    auto ptr = reinterpret_cast<SkString*>(str);
    SkDebugf("%s\n", ptr->c_str());
}
