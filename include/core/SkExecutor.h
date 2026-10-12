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

template <typename WorkT>
class SK_API SkExecutorBase {
public:
    using WorkType = WorkT;

    virtual ~SkExecutorBase();

    // Add work to execute.
    virtual void add(WorkT work, int /* workList */) { this->add(std::move(work)); }

    // deprecated
    virtual void add(WorkT work) = 0;

    // If it makes sense for this executor, use this thread to execute work for a little while.
    virtual void borrow() {}

    // Does not take ownership.  Not thread safe.
    // deprecated
    static void SetDefault(SkExecutorBase<WorkT>*);

protected:
    SkExecutorBase() = default;
    SkExecutorBase(const SkExecutorBase&) = delete;
    SkExecutorBase& operator=(const SkExecutorBase&) = delete;
};

//--------------------------------------------------------------------------------------------------
using SkExecutorStdFunction = std::function<void(void)>;

extern template class SK_API SkExecutorBase<SkExecutorStdFunction>;

using SkStdFunctionExecutor = SkExecutorBase<SkExecutorStdFunction>;

// deprecated. A class (vs. a using declaration) to keep pre-existing client functionality working.
class SK_API SkExecutor : public SkStdFunctionExecutor {};

//--------------------------------------------------------------------------------------------------
using SkExecutorCallbackContext = void*;
using SkExecutorCallbackFunction = void(*)(SkExecutorCallbackContext);

struct SkExecutorCallbackWork {
    SkExecutorCallbackFunction fCallback = nullptr;
    SkExecutorCallbackContext  fContext  = nullptr;
};

extern template class SK_API SkExecutorBase<SkExecutorCallbackWork>;

using SkCallbackExecutor = SkExecutorBase<SkExecutorCallbackWork>;

#endif // SkExecutor_DEFINED
