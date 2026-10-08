/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkExecutor_DEFINED
#define SkExecutor_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"

#include <functional>
#include <memory>

template <typename WorkT, typename TraitsT>
class SK_API SkExecutorBase {
public:
    using WorkType = WorkT;
    using WorkTraits = TraitsT;

    virtual ~SkExecutorBase();

    // Add work to execute.
    virtual void add(WorkT, int workList) = 0;

    // deprecated
    virtual void add(WorkT work) { this->add(std::move(work), 0); }

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
    static void Execute(SkExecutorStdFunction fn) { fn(); }
    static bool IsValid(SkExecutorStdFunction fn) { return SkToBool(fn); }
    static SkExecutorStdFunction Null() { return nullptr;}
};

typedef SkExecutorBase<SkExecutorStdFunction, SkExecutorStdFunctionTraits> SkStdFunctionExecutor;
using SkExecutor = SkStdFunctionExecutor;

//--------------------------------------------------------------------------------------------------
using SkExecutorCallbackContext = void*;
using SkExecutorCallbackFunction = void(*)(SkExecutorCallbackContext);

using SkExecutorCallbackPair = std::pair<SkExecutorCallbackFunction, SkExecutorCallbackContext>;

struct SkExecutorCallbackTraits {
    static void Execute(SkExecutorCallbackPair p) { p.first(p.second); }
    static bool IsValid(SkExecutorCallbackPair p) { return SkToBool(p.first); }
    static SkExecutorCallbackPair Null() { return {nullptr, nullptr}; }
};

typedef SkExecutorBase<SkExecutorCallbackPair, SkExecutorCallbackTraits> SkCallbackExecutor;

#endif // SkExecutor_DEFINED
