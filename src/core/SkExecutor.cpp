/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "include/private/SkMutex.h"
#include "include/private/SkSemaphore.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTPin.h"
#include "src/core/SkNoDestructor.h"

#include <deque>
#include <thread>
#include <utility>

using namespace skia_private;

#if defined(SK_BUILD_FOR_WIN)
    #include "src/core/SkLeanWindows.h"
    static int num_cores() {
        SYSTEM_INFO sysinfo;
        GetNativeSystemInfo(&sysinfo);
        return (int)sysinfo.dwNumberOfProcessors;
    }
#else
    #include <unistd.h>
    static int num_cores() {
        return (int)sysconf(_SC_NPROCESSORS_ONLN);
    }
#endif

template <typename Work, typename Traits>
SkExecutor<Work, Traits>::~SkExecutor() {}

// The default default SkExecutor is an SkTrivialExecutor, which just runs the work right away.
template <typename Work, typename Traits>
class SkTrivialExecutor final : public SkExecutor<Work> {
public:
#if 0
    void add(ExecuteFunction execute, ExecutionContext context, int /* workList */) override {
        execute(context);
    }
    void add(std::function<void(void)> work, int /* workList */) override {
        work();
    }
    void add(std::function<void(void)> work) override {
        work();
    }
#else
    void add(Work work, int /* workList */) override {
        work();
    }
    void add(Work work) override {
        work();
    }
#endif
    int discardAllPendingWork() override { return 0;}
};

template <typename Work, typename Traits>
static SkExecutor<Work, Traits>& trivial_executor() {
    static SkNoDestructor<SkTrivialExecutor<Work, Traits>> executor;
    return *executor;
}

template <typename Work, typename Traits>
SkExecutor<Work>* SkExecutor<Work, Traits>::gDefaultExecutor = nullptr;

template <typename Work, typename Traits>
SkExecutor<Work, Traits>& SkExecutor<Work, Traits>::GetDefault() {
    if (gDefaultExecutor) {
        return *gDefaultExecutor;
    }
    return trivial_executor<Work, Traits>();
}

template <typename Work, typename Traits>
void SkExecutor<Work, Traits>::SetDefault(SkExecutor<Work, Traits>* executor) {
    gDefaultExecutor = executor;
}

// We'll always push_back() new work, but pop from the front of deques or the back of SkTArray.
template <typename Work, typename Traits>
static inline Work pop(std::deque<Work, Traits>* list) {
    Work work = std::move(list->front());
    list->pop_front();
    return work;
}
template <typename Work, typename Traits>
static inline Work pop(TArray<Work>* list) {
    Work work = std::move(list->back());
    list->pop_back();
    return work;
}



// An SkThreadPool is an executor that runs work on a fixed pool of OS threads.
template <typename WorkList, typename Work>
class SkThreadPool final : public SkExecutor<Work> {
public:
    explicit SkThreadPool(int numWorkLists, int threads, bool allowBorrowing)
            : fNumWorkLists(numWorkLists < 1 ? 1 : numWorkLists)
            , fAllowBorrowing(allowBorrowing) {

        fWorkLists = std::make_unique<WorkList[]>(fNumWorkLists);

        for (int i = 0; i < threads; i++) {
            fThreads.emplace_back(&Loop, this);
        }
    }

    ~SkThreadPool() override {
        // Signal each thread that it's time to shut down.
        for (int i = 0; i < fThreads.size(); i++) {
            // Add the notification to the highest priority list
            this->add(nullptr, /* workList= */ 0);
        }
        // Wait for each thread to shut down.
        for (int i = 0; i < fThreads.size(); i++) {
            fThreads[i].join();
        }
    }

#if 0
    void add(std::function<void(void)> work, int workList) override {
        workList = SkTPin(workList, 0, fNumWorkLists-1);

        // Add some work to our pile of work to do.
        {
            SkAutoMutexExclusive lock(fWorkLock);

            fWorkLists[workList].emplace_back(std::move(work));
        }
        // Tell the Loop() threads to pick it up.
        fWorkAvailable.signal(1);
    }

    void add(std::function<void(void)> work) override {
        this->add(std::move(work), /* workList= */ 0);
    }
#endif

    void add(Work work, int workList) override {
        workList = SkTPin(workList, 0, fNumWorkLists-1);

        // Add some work to our pile of work to do.
        {
            SkAutoMutexExclusive lock(fWorkLock);

            fWorkLists[workList].emplace_back(work);
        }
        // Tell the Loop() threads to pick it up.
        fWorkAvailable.signal(1);
    }

    int discardAllPendingWork() override {
        SkAutoMutexExclusive lock(fWorkLock);

        int numDiscarded = 0;
        for (int i = 0; i < fNumWorkLists; ++i) {
            numDiscarded += fWorkLists[i].size();
            fWorkLists[i].clear();
        }

        return numDiscarded;
    }

    void borrow() override {
        // If there is work waiting and we're allowed to borrow work, do it.
        if (fAllowBorrowing && fWorkAvailable.try_wait()) {
            SkAssertResult(this->do_work());
        }
    }

private:
    // This method should usually be called only when fWorkAvailable indicates there's work to do.
    bool do_work() {
//        std::function<void(void)> work;
        Work work;
        bool workAvailable = false;
        {
            SkAutoMutexExclusive lock(fWorkLock);

            for (int i = 0; i < fNumWorkLists; ++i) {
                if (!fWorkLists[i].empty()) {
                    workAvailable = true;
                    work = pop(&fWorkLists[i]);
                    break;
                }
            }
        }

        if (!workAvailable) {
            // Because we can discard work asynchronous to Loop() we can sometimes get in this
            // method with no work to do
            return true;
        }

        if (!work.isValid()) {
            return false;  // This is Loop()'s signal to shut down.
        }

        work();
        return true;
    }

    static void Loop(void* ctx) {
        auto pool = (SkThreadPool*)ctx;
        do {
            pool->fWorkAvailable.wait();
        } while (pool->do_work());
    }

    // Both SkMutex and SkSpinlock can work here.
    using Lock = SkMutex;

    TArray<std::thread>         fThreads;
    const int                   fNumWorkLists; // guaranteed >= 1
    std::unique_ptr<WorkList[]> fWorkLists SK_GUARDED_BY(fWorkLock);
    Lock                        fWorkLock;
    SkSemaphore                 fWorkAvailable;
    const bool                  fAllowBorrowing;
};

template <typename Work, typename Traits>
std::unique_ptr<SkExecutor<Work, Traits>> SkExecutor<Work, Traits>::MakeFIFOThreadPool(
        int threads,
        bool allowBorrowing) {
    return SkExecutor::MakeMultiListFIFOThreadPool(/* numWorkLists= */ 1, threads, allowBorrowing);
}
template <typename Work, typename Traits>
std::unique_ptr<SkExecutor<Work, Traits>> SkExecutor<Work, Traits>::MakeLIFOThreadPool(
        int threads,
        bool allowBorrowing) {
    return SkExecutor::MakeMultiListLIFOThreadPool(/* numWorkLists= */ 1, threads, allowBorrowing);
}

template <typename Work, typename Traits>
std::unique_ptr<SkExecutor<Work, Traits>> SkExecutor<Work, Traits>::MakeMultiListFIFOThreadPool(
        int numWorkLists,
        int threads,
        bool allowBorrowing) {
//    using WorkList = std::deque<std::function<void(void)>>;
    using WorkList = std::deque<Work, Traits>;
    return std::make_unique<SkThreadPool<WorkList, Work>>(numWorkLists,
                                                          threads > 0 ? threads : num_cores(),
                                                          allowBorrowing);
}
template <typename Work, typename Traits>
std::unique_ptr<SkExecutor<Work, Traits>> SkExecutor<Work, Traits>::MakeMultiListLIFOThreadPool(
        int numWorkLists,
        int threads,
        bool allowBorrowing) {
//    using WorkList = TArray<std::function<void(void)>>;
    using WorkList = TArray<Work>;
    return std::make_unique<SkThreadPool<WorkList, Work>>(numWorkLists,
                                                          threads > 0 ? threads : num_cores(),
                                                          allowBorrowing);
}
