/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkTaskGroup.h"

#include "include/core/SkExecutor.h"

#include <type_traits>
#include <utility>

SkTaskGroup::SkTaskGroup(SkExecutor& executor) : fExecutor(executor) {}

void SkTaskGroup::add(std::function<void()> fn, int workList) {
    fExecutor.add(std::move(fn), workList);
}

void SkTaskGroup::wait() {
    fExecutor.wait();
}

SkTaskGroup::Enabler::Enabler(int threads) {
    if (threads) {
        fThreadPool = SkExecutor::MakeLIFOThreadPool(threads);
        SkExecutor::SetDefault(fThreadPool.get());
    }
}
