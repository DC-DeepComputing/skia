// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bridge.h"
#include "../../../modules/skunicode/include/SkUnicode.h"
#include "../../../include/core/SkString.h"

static std::unique_ptr<SkUnicode> s = nullptr;
void* make_unicode() {
    s = SkUnicode::Make();
    auto ptr = reinterpret_cast<void*>(s.get());
    return ptr;
}

void destroy_unicode(void* unicode) {
    if (s && unicode == s.get()) {
        delete s.get();
    }
}

void  destroy_str(void* str) {
    auto ptr = reinterpret_cast<SkString*>(str);
    delete ptr;
}

void* upper(char* str) {
    auto res = new SkString(s->toUpper(SkString(str)));
    return reinterpret_cast<void*>(res);
}

void  print(void* str) {
    auto ptr = reinterpret_cast<SkString*>(str);
    SkDebugf("%s\n", ptr->c_str());
}
