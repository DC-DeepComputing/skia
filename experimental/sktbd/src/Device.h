/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Device_DEFINED
#define Device_DEFINED

#include "src/core/SkDevice.h"

namespace sktbd {

class Device final : public SkBaseDevice  {
public:
    Device(const SkImageInfo&);
private:
};

} // namespace sktbd

#endif // Device_DEFINED
