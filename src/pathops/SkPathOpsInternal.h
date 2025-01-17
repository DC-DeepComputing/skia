/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathOpsInternal_DEFINED
#define SkPathOpsInternal_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkTypes.h"

#include "include/pathops/SkPathOps.h"

// These should just be visible to pathops internals, as they used to be public,
// but we want the implementation to not call the public functions anymore.

namespace skpathopsinternal {

bool Op(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result);
bool Simplify(const SkPath& path, SkPath* result);
bool TightBounds(const SkPath& path, SkRect* result);
bool AsWinding(const SkPath& path, SkPath* result);

}

#endif
