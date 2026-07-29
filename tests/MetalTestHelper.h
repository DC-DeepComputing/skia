/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef MetalTestHelper_DEFINED
#define MetalTestHelper_DEFINED

#if defined(SK_METAL)
#define DECLARE_FORCE_LINK_SYM(name) extern "C" void force_link_##name()
#define CALL_FORCE_LINK_SYM(name) force_link_##name()

// Declare external force link symbols
DECLARE_FORCE_LINK_SYM(MtlBackendAllocationTest);
DECLARE_FORCE_LINK_SYM(MtlCopySurfaceTest);
DECLARE_FORCE_LINK_SYM(MtlBackendTextureTest);

inline void force_link_metal_tests() {
    CALL_FORCE_LINK_SYM(MtlBackendAllocationTest);
    CALL_FORCE_LINK_SYM(MtlCopySurfaceTest);
    CALL_FORCE_LINK_SYM(MtlBackendTextureTest);
}
#else
inline void force_link_metal_tests() {}
#endif

#endif // MetalTestHelper_DEFINED
