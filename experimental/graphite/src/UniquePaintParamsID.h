/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniquePaintParamsID_DEFINED
#define skgpu_UniquePaintParamsID_DEFINED

#include "include/core/SkTypes.h"

namespace skgpu {

class UniquePaintParamsID {
public:
    static UniquePaintParamsID InvalidID() { return UniquePaintParamsID(); }
    static UniquePaintParamsID Next();

    UniquePaintParamsID() : fID(SK_InvalidUniqueID) {}

    bool operator==(const UniquePaintParamsID &that) const { return fID == that.fID; }
    bool operator!=(const UniquePaintParamsID &that) const { return !(*this == that); }

    bool isValid() const { return fID != SK_InvalidUniqueID; }
    uint32_t asUInt() const { return fID; }

private:
    explicit UniquePaintParamsID(uint32_t id) : fID(id) {}

    uint32_t fID;
};

} // namespace skgpu

#endif // skgpu_UniquePaintParamsID_DEFINED
