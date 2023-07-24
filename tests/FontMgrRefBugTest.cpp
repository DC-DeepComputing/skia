/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "tests/Test.h"

DEF_TEST(FontMgr_PotentialRefBug, reporter) {
  sk_sp<SkFontMgr> mgr = SkFontMgr::RefDefault();
}
