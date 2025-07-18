/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skcpu_ContextImpl_DEFINED
#define skcpu_ContextImpl_DEFINED

#include "include/core/SkCPUContext.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkResourceCache.h"

namespace skcpu {
class ContextImpl final : public Context {
public:
    ContextImpl(std::unique_ptr<SkResourceCache>, SkResourceCache*);

    static const ContextImpl* TODO();

private:
    std::unique_ptr<SkResourceCache> fCache;
    SkResourceCache* fCachePtr;
};
}  // namespace skcpu

#endif
