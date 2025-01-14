/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/pathops/SkPathOps.h"
#include "src/pathops/SkPathOpsInternal.h"

#include <optional>

namespace skia::pathops {

/*
 *  Here we dispatch from the "public" API for pathops to the implementation.
 *
 *  At the moment this is always known at link-time, but the idea is to allow
 *  for runtime registration of the imp, so this would change to use that.
 */

std::optional<SkPath> Op(const SkPath& one, const SkPath& two, SkPathOp op) {
    SkPath result;
    if (skpathopsinternal::Op(one, two, op, &result)) {
        return result;
    }
    return {};
}

std::optional<SkPath> Simplify(const SkPath& path) {
    SkPath result;
    if (skpathopsinternal::Simplify(path, &result)) {
        return result;
    }
    return {};
}

std::optional<SkRect> TightBounds(const SkPath& path) {
    SkRect result;
    if (skpathopsinternal::TightBounds(path, &result)) {
        return result;
    }
    return {};
}

std::optional<SkPath> AsWinding(const SkPath& path) {
    SkPath result;
    if (skpathopsinternal::AsWinding(path, &result)) {
        return result;
    }
    return {};
}

}
