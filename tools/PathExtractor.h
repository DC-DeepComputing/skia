/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PathExtractor_DEFINED
#define PathExtractor_DEFINED

#include <functional>

class SkMatrix;
class SkPaint;
class SkPath;

namespace ToolUtils {
using PathSniffCallback = void(const SkMatrix&, const SkPath&, const SkPaint&);

// Calls the provided PathSniffCallback for each path in the given file.
// Supported file formats are .svg and .skp.
void ExtractPaths(const char filepath[], std::function<PathSniffCallback>);


}  // namespace ToolUtils

#endif
