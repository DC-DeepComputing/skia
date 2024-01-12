/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/CompilationSingleton.h"

namespace skgpu::graphite {

CompilationSingleton::CompilationSingleton() {
    for (int i = 0; i < kCompilationPriorityTypeCnt; ++i) {
        fQueues[i] = 0;
    }
}
CompilationSingleton::~CompilationSingleton() {}

void CompilationSingleton::compile(const PaintOptions& options, CompilationPriority priority) {

}

void CompilationSingleton::compile(const PaintParams& paintParams) {

}

} // namespace skgpu::graphite
