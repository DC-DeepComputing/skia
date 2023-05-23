/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentProcessors_DEFINED
#define GrFragmentProcessors_DEFINED

#include <memory>

class SkBlenderBase;
class GrFragmentProcessor;
class SkMaskFilter;
struct GrFPArgs;
class SkMatrix;

namespace GrFragmentProcessors {
std::unique_ptr<GrFragmentProcessor> Make(const SkMaskFilter*,
                                          const GrFPArgs&,
                                          const SkMatrix& ctm);

/**
 * Returns a GrFragmentProcessor that implements this blend for the Ganesh GPU backend.
 * The GrFragmentProcessor expects premultiplied inputs and returns a premultiplied output.
 */
std::unique_ptr<GrFragmentProcessor> Make(const SkBlenderBase*,
                                          std::unique_ptr<GrFragmentProcessor> srcFP,
                                          std::unique_ptr<GrFragmentProcessor> dstFP,
                                          const GrFPArgs& fpArgs);

bool IsSupported(const SkMaskFilter*);
}

#endif
