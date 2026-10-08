/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ProcsUtils_DEFINED
#define ProcsUtils_DEFINED

#include "include/core/SkSerialProcs.h"

#if defined(SK_GANESH) || defined(SK_GRAPHITE)
#include "include/private/chromium/Slug.h"
#endif

namespace ToolUtils {
namespace internal {
SkDeserialProcs default_deserial_procs();
}  // namespace internal

// Returns the default SkDeserialProcs used by Skia's tools when Deserializing Skps. This adds
// default values for the SkDeserialImageProc and SkDeserialTypefaceProc.
inline SkDeserialProcs default_deserial_procs() {
    SkDeserialProcs procs = internal::default_deserial_procs();
#if defined(SK_GANESH) || defined(SK_GRAPHITE)
    sktext::gpu::Slug::AddDeserialProcs(&procs);
#endif
    return procs;
}

// Returns the default SkSerialProcs used by Skia's tools when serializing Skps. This adds
// default values for the SkSerialImageProc.
SkSerialProcs default_serial_procs();

}  // namespace ToolUtils

#endif  // ToolUtils_DEFINED
