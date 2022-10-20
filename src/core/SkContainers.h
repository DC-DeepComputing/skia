// Copyright 2022 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SkContainers_DEFINED
#define SkContainers_DEFINED

#include "include/core/SkSpan.h"

SkSpan<std::byte> sk_allocate(int atLeast, size_t sizeOfT, double growthFactor);

#endif  // SkContainers_DEFINED
