/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_SparseStripTypes_DEFINED
#define skgpu_graphite_SparseStripTypes_DEFINED

namespace skgpu::graphite {

struct Point {
    float x;
    float y;
};

struct Line {
    Point p0;
    Point p1;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_SparseStripTypes_DEFINED
