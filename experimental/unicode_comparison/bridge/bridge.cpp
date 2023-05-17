// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bridge.h"
#include "../../../modules/skunicode/include/SkUnicode.h"
#include "../../../include/core/SkString.h"
#include "../../../include/core/SkTime.h"

namespace {
static std::unique_ptr<SkUnicode> gUnicode = nullptr;
static skia_private::TArray<SkUnicode::CodeUnitFlags, true> gCodeUnitFlags;
}

bool make_unicode() {
    gUnicode = SkUnicode::Make();
    auto ptr = reinterpret_cast<void*>(gUnicode.get());
    if (ptr == nullptr) {
        SkDebugf("Could not create Unicode object\n");
        return false;
    }
    return true;
}

void destroy_unicode() {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist\n");
        return;
    }
    delete gUnicode.get();
}

void  destroy_str(void* str) {
    auto ptr = reinterpret_cast<SkString*>(str);
    delete ptr;
}

double initialize(char* text) {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist\n");
        return -1;
    }
    double time = SkTime::GetNSecs();
    gUnicode->computeCodeUnitFlags(text, strlen(text), false, &gCodeUnitFlags);
    return (SkTime::GetNSecs() - time);
}

bool  getFlags(int index, void* flags) {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist");
        return false;
    }
    if (index < 0 || index >= gCodeUnitFlags.size()) {
        if (gCodeUnitFlags.size() == 0) {
            SkDebugf("Unicode object is empty or not initialized\n");
        } else {
            SkDebugf("Unicode object is not initialized\n");
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
