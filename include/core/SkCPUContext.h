/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skcpu_Context_DEFINED
#define skcpu_Context_DEFINED

#include "include/private/base/SkAPI.h"

#include <memory>

class SkDiscardableMemory;

namespace skcpu {
class Recorder;

using DiscardableFactory = SkDiscardableMemory* (*)(size_t bytes);

class SK_API Context {
public:
    struct Options {
        // One of these two options may be set to configure caching
        // inside of Skia.
        size_t fResourceCacheByteLimit = 0;
        DiscardableFactory fResourceCacheFactory = nullptr;
    };

    std::unique_ptr<Recorder> makeRecorder() const;

    static std::unique_ptr<const Context> Make(const Options&);
    static std::unique_ptr<const Context> Make();

protected:
    Context() = default;
};
}  // namespace skcpu

#endif
