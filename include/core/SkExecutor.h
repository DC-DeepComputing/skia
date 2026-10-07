/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkExecutor_DEFINED
#define SkExecutor_DEFINED

#include <functional>
#include <memory>
#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"

template <typename Work, typename Traits>
class SK_API SkExecutor {
public:
    using WorkType = Work;
    using WorkTraits = Traits;

    virtual ~SkExecutor();

    // Create a thread pool SkExecutor with a fixed thread count, by default the number of cores.
    static std::unique_ptr<SkExecutor> MakeFIFOThreadPool(int threads = 0,
                                                          bool allowBorrowing = true);
    static std::unique_ptr<SkExecutor> MakeLIFOThreadPool(int threads = 0,
                                                          bool allowBorrowing = true);

    // A work list is the queue or stack to which work is added and removed. The above two
    // factory functions create an executor with only one list while the following two factories
    // can create executors with multiple work lists. Having multiple work lists allows for
    // prioritization with work being pulled from the lower indexed work lists first - with
    // work list '0' being the highest priority.
    static std::unique_ptr<SkExecutor> MakeMultiListFIFOThreadPool(int numWorkLists,
                                                                   int threads = 0,
                                                                   bool allowBorrowing = true);
    static std::unique_ptr<SkExecutor> MakeMultiListLIFOThreadPool(int numWorkLists,
                                                                   int threads = 0,
                                                                   bool allowBorrowing = true);

    // There is always a default SkExecutor available by calling SkExecutor::GetDefault().
    static SkExecutor& GetDefault();
    static void SetDefault(SkExecutor*);  // Does not take ownership.  Not thread safe.

    // Add work to execute.
    virtual void add(Work, int workList) = 0;

    // deprecated
    virtual void add(Work work) { this->add(std::move(work), 0); }

    // If it makes sense for this executor, use this thread to execute work for a little while.
    virtual void borrow() {}

protected:
    SkExecutor() = default;
    SkExecutor(const SkExecutor&) = delete;
    SkExecutor& operator=(const SkExecutor&) = delete;

private:
    static SkExecutor<Work, Traits>* gDefaultExecutor;
};

//--------------------------------------------------------------------------------------------------
typedef std::function<void(void)> StdFunction;

struct StdFunctionTraits {
    static void Execute(std::function<void(void)> fn) { fn(); }
    static bool IsValid(std::function<void(void)> fn) { return SkToBool(fn); }
    static StdFunction Null() { return nullptr;}
};

typedef SkExecutor<StdFunction, StdFunctionTraits> StdFunctionExecutor;

//--------------------------------------------------------------------------------------------------
using ExecutionContext = void*;
using ExecuteFunction = void(*)(ExecutionContext);

typedef std::pair<ExecuteFunction, ExecutionContext> ExecuteCallbackPair;

struct ExecuteCallbackTraits {
    static void Execute(ExecuteCallbackPair p) { p.first(p.second); }
    static bool IsValid(ExecuteCallbackPair p) { return SkToBool(p.first); }
    static ExecuteCallbackPair Null() { return {nullptr, nullptr}; }
};

typedef SkExecutor<ExecuteCallbackPair, ExecuteCallbackTraits> CallbackExecutor;

#endif//SkExecutor_DEFINED
