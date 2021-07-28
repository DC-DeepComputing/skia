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
 * used instead.
 */
std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> src,
                                          std::unique_ptr<GrFragmentProcessor> dst,
                                          SkBlendMode mode);

/**
 * Blends src and dst inputs according to the blend mode. Creates a blend-function FP with two
 * color inputs. If either FP is null, the associated color input (src or dst) is used instead.
 */
std::unique_ptr<GrFragmentProcessor> MakeBlendFunction(std::unique_ptr<GrFragmentProcessor> src,
                                                       std::unique_ptr<GrFragmentProcessor> dst,
                                                       SkBlendMode mode);

}  // namespace GrBlendFragmentProcessor

#endif
