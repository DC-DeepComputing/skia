/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_GRAPHITE_MODULES
#define SKSL_GRAPHITE_MODULES

namespace SkSL::Loader {
struct GraphiteModules {
    const char* fFragmentShader;
    const char* fVertexShader;
};

// Initializes the Graphite SkSL modules in the central module loader. Safe to call multiple times.
void LoadGraphiteModules();
void SetGraphiteModuleData(const GraphiteModules&);

}  // namespace SkSL::Loader

#endif  // SKSL_GRAPHITE_MODULES
