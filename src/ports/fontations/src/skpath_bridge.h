// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include <memory>
#include "include/core/SkPath.h"

#ifndef SkPathBridge_DEFINED
#define SkPathBridge_DEFINED

class SkPath;

namespace fontations_ffi {
class SkPathWrapper {
public:
    SkPathWrapper();
    void move_to(float x, float y);
    void line_to(float x, float y);
    void quad_to(float cx0, float cy0, float x, float y);
    void curve_to(float cx0, float cy0, float cx1, float cy1, float x, float y);
    void close();
    void dump();
    const SkPath& getPath();

private:
  SkPath path_;
};

}  // namespace fontations_ffi

#endif
