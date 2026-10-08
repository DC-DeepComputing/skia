/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkExecutor_DEFINED
#define SkExecutor_DEFINED

#include "include/core/SkTypes.h"

#include <functional>
#include <utility>

template <typename WorkT, typename TraitsT>
class SK_API SkExecutorBase {
public:
    using WorkType = WorkT;
    using WorkTraits = TraitsT;

    virtual ~SkExecutorBase();

    // Add work to execute.
    virtual void add(WorkT work, int /* workList */) { this->add(std::move(work)); }

    // deprecated
    virtual void add(WorkT work) = 0;

    // If it makes sense for this executor, use this thread to execute work for a little while.
    virtual void borrow() {}

protected:
    SkExecutorBase() = default;
    SkExecutorBase(const SkExecutorBase&) = delete;
    SkExecutorBase& operator=(const SkExecutorBase&) = delete;
};

//--------------------------------------------------------------------------------------------------
using SkExecutorStdFunction = std::function<void(void)>;

struct SkExecutorStdFunctionTraits {
    static void Execute(const SkExecutorStdFunction& fn) { fn(); }
    static bool IsValid(const SkExecutorStdFunction& fn) { return static_cast<bool>(fn); }
    static SkExecutorStdFunction Null() { return nullptr; }
};

extern template class SK_API SkExecutorBase<SkExecutorStdFunction, SkExecutorStdFunctionTraits>;

using SkStdFunctionExecutor = SkExecutorBase<SkExecutorStdFunction, SkExecutorStdFunctionTraits>;
using SkExecutor = SkStdFunctionExecutor;

//--------------------------------------------------------------------------------------------------
using SkExecutorCallbackContext = void*;
using SkExecutorCallbackFunction = void(*)(SkExecutorCallbackContext);

using SkExecutorCallbackPair = std::pair<SkExecutorCallbackFunction, SkExecutorCallbackContext>;

struct SkExecutorCallbackTraits {
    static void Execute(SkExecutorCallbackPair p) { p.first(p.second); }
    static bool IsValid(SkExecutorCallbackPair p) { return p.first != nullptr; }
    static SkExecutorCallbackPair Null() { return {nullptr, nullptr}; }
};

extern template class SK_API SkExecutorBase<SkExecutorCallbackPair, SkExecutorCallbackTraits>;

using SkCallbackExecutor = SkExecutorBase<SkExecutorCallbackPair, SkExecutorCallbackTraits>;

#endif // SkExecutor_DEFINED
