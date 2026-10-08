/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"

template <typename Work, typename Traits>
SkExecutorBase<Work, Traits>::~SkExecutorBase() {}

/*
 * For Skia to use an SkExecutor it needs to explicitly know about its templated
 * structure beforehand. There is no case in which a client can create some new templated
 * flavor of SkExecutor and expect Skia to make use of it. Thus, we can force instantiation
 * here.
 */
template class SK_API SkExecutorBase<SkExecutorStdFunction, SkExecutorStdFunctionTraits>;
template class SK_API SkExecutorBase<SkExecutorCallbackPair, SkExecutorCallbackTraits>;
