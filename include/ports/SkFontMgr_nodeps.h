/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_nodeps_DEFINED
#define SkFontMgr_nodeps_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

class SkFontMgr;

/** Create a custom font manager that contains no built-in fonts
 *  and no dependencies to any typefaces
 */
SK_API sk_sp<SkFontMgr> SkFontMgr_Get_NoDeps();

#endif // SkFontMgr_nodeps_DEFINED
