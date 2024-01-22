/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_CompilationSingleton_DEFINED
#define skgpu_graphite_CompilationSingleton_DEFINED

#include "include/core/SkRefCnt.h"

#include "src/gpu/graphite/Precompile.h"

namespace skgpu::graphite {

class PaintParams;

class CompilationSingleton : public SkRefCnt {
public:
    CompilationSingleton();
    ~CompilationSingleton() override;

    // Intended to be called from the Context.
    void compile(const PaintOptions&, CompilationPriority);

    // Assumes kMedium priority. Intended to be called by the Recorders.
    void compile(const PaintParams&);

private:
    int fQueues[kCompilationPriorityTypeCnt];
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_CompilationSingleton_DEFINED
