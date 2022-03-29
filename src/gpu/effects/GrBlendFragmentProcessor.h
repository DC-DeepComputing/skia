/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBlendFragmentProcessor_DEFINED
#define GrBlendFragmentProcessor_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"

class GrFragmentProcessor;

namespace GrBlendFragmentProcessor {

/**
 * Blends src and dst inputs according to the blend mode. If either input is null, fInputColor is
 * used instead. Most Porter-Duff blend types share the same logic (reducing shader count).
 */
std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> src,
                                          std::unique_ptr<GrFragmentProcessor> dst,
                                          SkBlendMode mode,
                                          bool shareBlendLogic = true);
/**
 * Blends src and dst inputs according to the blend mode. If either input is null, fInputColor is
 * used instead. Hard-wires a single blend mode into the code (slightly reducing complexity).
 */
template <SkBlendMode mode>
std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> src,
                                          std::unique_ptr<GrFragmentProcessor> dst) {
    return Make(std::move(src), std::move(dst), mode, /*shareBlendLogic=*/false);
}


}  // namespace GrBlendFragmentProcessor

#endif
