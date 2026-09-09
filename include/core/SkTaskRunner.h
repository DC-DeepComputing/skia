/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskRunner_DEFINED
#define SkTaskRunner_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

/*
 * A task is a single chunk of Skia work that can be executed on a separate thread.
 */
class SK_API SkTask : public SkRefCnt {
public:
    ~SkTask() override {}

    virtual void execute() = 0;
};

/*
 * The SkTaskRunner provides a means for clients to allow Skia to perform threaded work.
 * Skia will break threadable work into SkTasks and give them to the client provided
 * SkTaskRunner-derived class. The client's implementation should then "execute" the
 * SkTasks on some thread - ideally taking into account the supplied priority.
 *
 * On shutdown Skia will call the SkTaskRunner's "wait" method. The client's implementation
 * should wait for all previously added SkTasks to complete before returning.
 */
class SK_API SkTaskRunner : public SkRefCnt {
public:
    ~SkTaskRunner() override {}

    /*
     * Ideally, kHigh priority tasks will take precedence over kLow priority tasks.
     */
    enum class Priority { kHigh = 0, kLow = 1 };

    /*
     * Execute the provided SkTask on some thread. Ideally, at the provided priority.
     */
    virtual void add(sk_sp<SkTask>, Priority) = 0;

    /*
     * Wait for all previously provided SkTasks to complete before returning.
     */
    virtual void wait() = 0;

protected:
    SkTaskRunner() = default;
    SkTaskRunner(const SkTaskRunner&) = delete;
    SkTaskRunner& operator=(const SkTaskRunner&) = delete;
};

#endif // SkTaskRunner_DEFINED
