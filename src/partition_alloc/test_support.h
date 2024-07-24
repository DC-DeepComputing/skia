/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef PartitionAlloc_TestSupport_DEFINED
#define PartitionAlloc_TestSupport_DEFINED

namespace skia_private {

// Enable using PartitionAlloc's allocator.
void InitializePartitionAllocForTesting();

}  // namespace skia_private

#endif  // PartitionAlloc_TestSupport_DEFINED
