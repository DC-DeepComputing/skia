/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skshaper/include/SkShaper_factory.h"

namespace SkShapers {

// A static variable inside a function avoids a static initializer.
// https://chromium.googlesource.com/chromium/src/+/HEAD/docs/static_initializers.md#removing-static-initializers
static sk_sp<Factory>* get_factory() {
    static sk_sp<Factory> factory = SkShapers::Primitive::Factory();
    return &factory;
}

sk_sp<Factory> GetFactory() {
    return *get_factory();
}

void RegisterFactory(sk_sp<Factory> newFactory) {
    *get_factory() = newFactory;
}

}  // namespace SkShapers
