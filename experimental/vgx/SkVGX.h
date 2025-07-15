/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkVGX_DEFINED
#define SkVGX_DEFINED

class SkCanvas;
class SkStream;

namespace skvgx {

bool Parse(SkStream*, SkCanvas*);

}  // namespace skvgx

#endif  // SkVGX_DEFINED
