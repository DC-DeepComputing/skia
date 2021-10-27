/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ProgramCache_DEFINED
#define skgpu_ProgramCache_DEFINED

#include "experimental/graphite/include/private/GraphiteTypesPriv.h"

namespace skgpu {

struct Combination;

class ProgramCache {
public:
    // Returns the program ID of the program needed for the combination
    uint32_t findOrCreateProgram(Combination);

private:
};

} // namespace skgpu

#endif // skgpu_ProgramCache_DEFINED
