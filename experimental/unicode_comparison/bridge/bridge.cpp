// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bridge.hxx"
#include "modules/skunicode/include/SkUnicode.h"

void* make_unicode() {
    return (void*) SkUnicode::Make().get();
}

void destroy_unicode(void* unicode) {
    delete (SkUnicode*)unicode;
}