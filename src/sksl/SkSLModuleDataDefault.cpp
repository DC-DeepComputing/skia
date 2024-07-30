/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLModule.h"

#include <string>

// We include minified SkSL module code and pass it directly to the compiler.
#if defined(SK_ENABLE_OPTIMIZE_SIZE) || !defined(SK_DEBUG)
    #include "src/sksl/generated/sksl_shared.minified.sksl"
    #include "src/sksl/generated/sksl_compute.minified.sksl"
    #include "src/sksl/generated/sksl_frag.minified.sksl"
    #include "src/sksl/generated/sksl_gpu.minified.sksl"
    #include "src/sksl/generated/sksl_public.minified.sksl"
    #include "src/sksl/generated/sksl_rt_shader.minified.sksl"
    #include "src/sksl/generated/sksl_vert.minified.sksl"
    // #if defined(SK_GRAPHITE)
    // #include "src/sksl/generated/sksl_graphite_frag.minified.sksl"
    // #include "src/sksl/generated/sksl_graphite_vert.minified.sksl"
    // #include "src/sksl/generated/sksl_graphite_frag_es2.minified.sksl"
    // #include "src/sksl/generated/sksl_graphite_vert_es2.minified.sksl"
    // #endif
#else
    #include "src/sksl/generated/sksl_shared.unoptimized.sksl"
    #include "src/sksl/generated/sksl_compute.unoptimized.sksl"
    #include "src/sksl/generated/sksl_frag.unoptimized.sksl"
    #include "src/sksl/generated/sksl_gpu.unoptimized.sksl"
    #include "src/sksl/generated/sksl_public.unoptimized.sksl"
    #include "src/sksl/generated/sksl_rt_shader.unoptimized.sksl"
    #include "src/sksl/generated/sksl_vert.unoptimized.sksl"
    // #if defined(SK_GRAPHITE)
    // #include "src/sksl/generated/sksl_graphite_frag.unoptimized.sksl"
    // #include "src/sksl/generated/sksl_graphite_vert.unoptimized.sksl"
    // #include "src/sksl/generated/sksl_graphite_frag_es2.unoptimized.sksl"
    // #include "src/sksl/generated/sksl_graphite_vert_es2.unoptimized.sksl"
    // #endif
#endif

namespace {
static SkSL::Loader::LoadFn gLoad_sksl_graphite_frag = nullptr;
static SkSL::Loader::LoadFn gLoad_sksl_graphite_frag_es2 = nullptr;
static SkSL::Loader::LoadFn gLoad_sksl_graphite_vert = nullptr;
static SkSL::Loader::LoadFn gLoad_sksl_graphite_vert_es2 = nullptr;
}

namespace SkSL {

std::string GetModuleData(ModuleType type, const char* /*filename*/) {
#define M(name) case ModuleType::name: return std::string(SKSL_MINIFIED_##name);
#define G(name) case ModuleType::name:          \
    if (gLoad_##name) { return gLoad_##name();} \
    else { return ""; }

    switch (type) {
        M(sksl_shared)
        M(sksl_compute)
        M(sksl_frag)
        M(sksl_gpu)
        M(sksl_public)
        M(sksl_rt_shader)
        M(sksl_vert)

        G(sksl_graphite_frag)
        G(sksl_graphite_frag_es2)
        G(sksl_graphite_vert)
        G(sksl_graphite_vert_es2)
        default:
            SkUNREACHABLE;
    }
#undef M
}

}  // namespace SkSL
