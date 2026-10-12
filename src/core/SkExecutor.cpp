/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"

#include "src/core/SkExecutors.h"

template<>
void SkStdFunctionExecutor::SetDefault(SkStdFunctionExecutor* executor) {
    SkStdFunctionExecutors::SetDefault(executor);
}

template<>
void SkCallbackExecutor::SetDefault(SkCallbackExecutor* executor) {
    SkCallbackExecutors::SetDefault(executor);
}
