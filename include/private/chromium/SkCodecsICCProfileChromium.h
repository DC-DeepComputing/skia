/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodecsICCProfileChromium_DEFINED
#define SkCodecsICCProfileChromium_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"
#include "modules/skcms/skcms.h"

namespace SkCodecs {

class SK_API ICCProfileChromium {
public:
    virtual ~ICCProfileChromium() = default;

    // Returns nullptr if parsing fails. May retain `data`. If `forceSkcms` is
    // true, then will use skcms for parsing, otherwise it may use Rust.
    static std::unique_ptr<ICCProfileChromium> Make(
        sk_sp<SkData> data, bool forceSkcms);

    // Return the parsed profile. The pointers in the structure are guaranteed
    // to be valid until `this` is destroyed.
    virtual const skcms_ICCProfile& GetProfile() const = 0;
};

}  // namespace SkCodecs

#endif
