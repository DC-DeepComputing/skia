/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodecsICCProfileChromium_DEFINED
#define SkCodecsICCProfileChromium_DEFINED

#include <memory>

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"
#include "modules/skcms/skcms.h"

namespace SkCodecs {

// Force all ICC profile parsing to use skcms instead of the build-default
// parser. This is intended for Chromium to have a kill-switch to fall back
// to skcms if moxcms causes issues in the field.
//
// This is a global setting and is NOT thread-safe with respect to concurrent
// codec operations. It should be called once early during process startup.
SK_API void ForceSkcmsForICCProfiles(bool force);

// Allows Chromium code that does not use SkCodec to parse ICC profiles
// using the same parser that SkCodec uses (skcms or moxcms).
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

#endif  // SkCodecsICCProfileChromium_DEFINED
