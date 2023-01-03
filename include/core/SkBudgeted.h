/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBudgeted_DEFINED
#define SkBudgeted_DEFINED

/** Indicates whether an allocation should count against a cache budget. */
enum class SkBudgeted : bool {
    kNo  = false,
    kYes = true
};

#endif
