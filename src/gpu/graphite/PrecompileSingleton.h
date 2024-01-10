/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PrecompileSingleton_DEFINED
#define skgpu_graphite_PrecompileSingleton_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {

class PaintOptions;
enum class PrecompilationPriority : int;

class PrecompileSingleton : public SkRefCnt {
public:
    PrecompileSingleton();
    ~PrecompileSingleton() override;

    void foo(const PaintOptions& options, PrecompilationPriority priority);

private:
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileSingleton_DEFINED
