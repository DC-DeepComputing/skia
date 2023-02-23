/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ReadWriteSwizzle_DEFINED
#define ReadWriteSwizzle_DEFINED

namespace skgpu::graphite {
/**
 * Enumerate the few possible read and write swizzle options for smaller storage.
*/
enum class ReadSwizzle {
    kRGBA, // Default
    kRGB1,
    k000R,
    kRRR1,
    kBGRA,
    kLastReadSwizzle = kBGRA,
};

static constexpr int kReadSwizzleCount = static_cast<int>(ReadSwizzle::kLastReadSwizzle) + 1;

enum class WriteSwizzle {
    kRGBA, // Default
    kA000,
    kBGRA,
    kLastWriteSwizzle = kBGRA,
};

static constexpr int kWriteSwizzleCount = static_cast<int>(WriteSwizzle::kLastWriteSwizzle) + 1;

}

#endif // ReadWriteSwizzle_DEFINED
