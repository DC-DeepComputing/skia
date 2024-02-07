/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_SwizzlePriv_DEFINED
#define skgpu_SwizzlePriv_DEFINED

#include "src/gpu/Swizzle.h"

namespace skgpu {

// This class is friended by Swizzle and allows the backend ContextFactory functions to
// trampoline through this to call the private Context ctor. We can't directly friend the factory
// functions in Context because they are in a different namespace and we don't want to declare the
// functions in Context.h
class SwizzleCtorAccessor {
public:
    static Swizzle Make(uint16_t key) { return Swizzle(key); }
};

} // namespace skgpu

#endif // skgpu_SwizzlePriv_DEFINED
