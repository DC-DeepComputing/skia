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
class SkTrivialExecutor final : public SkExecutor<Work, Traits> {
public:
    void add(Work work, int /* workList */) override {
        Traits::Execute(work);
    }
    void add(Work work) override {
        Traits::Execute(work);
    }
};

template <typename Work, typename Traits>
static SkExecutor<Work, Traits>& trivial_executor() {
    static SkNoDestructor<SkTrivialExecutor<Work, Traits>> executor;
    return *executor;
}

template <typename Work, typename Traits>
SkExecutor<Work, Traits>* SkExecutor<Work, Traits>::gDefaultExecutor = nullptr;

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
template <typename Work>
static inline Work pop(std::deque<Work>* list) {
    Work work = std::move(list->front());
    list->pop_front();
    return work;
}
template <typename Work>
static inline Work pop(TArray<Work>* list) {
    Work work = std::move(list->back());
    list->pop_back();
    return work;
}

// An SkThreadPool is an executor that runs work on a fixed pool of OS threads.
template <typename WorkList1, typename ExecutorType>
class SkThreadPool final : public ExecutorType {
public:
    explicit SkThreadPool(int numWorkLists, int threads, bool allowBorrowing)
            : fNumWorkLists(numWorkLists < 1 ? 1 : numWorkLists)
            , fAllowBorrowing(allowBorrowing) {

        fWorkLists = std::make_unique<WorkList1[]>(fNumWorkLists);

        for (int i = 0; i < threads; i++) {
            fThreads.emplace_back(&Loop, this);
        }
    }

    ~SkThreadPool() override {
        // Signal each thread that it's time to shut down.
        for (int i = 0; i < fThreads.size(); i++) {
            // Add the notification to the highest priority list
            this->add(ExecutorType::WorkTraits::Null(), /* workList= */ 0);
        }
        // Wait for each thread to shut down.
        for (int i = 0; i < fThreads.size(); i++) {
            fThreads[i].join();
        }
    }

    void add(ExecutorType::WorkType work, int workList) override {
        workList = SkTPin(workList, 0, fNumWorkLists-1);

        // Add some work to our pile of work to do.
        {
            SkAutoMutexExclusive lock(fWorkLock);

            fWorkLists[workList].emplace_back(std::move(work));
        }
        // Tell the Loop() threads to pick it up.
        fWorkAvailable.signal(1);
    }

    void add(ExecutorType::WorkType work) override {
        this->add(std::move(work), /* workList= */ 0);
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
        typename ExecutorType::WorkType work;
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

        if (!ExecutorType::WorkTraits::IsValid(work)) {
            return false;  // This is Loop()'s signal to shut down.
        }

        ExecutorType::WorkTraits::Execute(work);
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
    std::unique_ptr<WorkList1[]> fWorkLists SK_GUARDED_BY(fWorkLock);
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
    using WorkList = std::deque<Work>;
    using ExecutorType = SkExecutor<Work, Traits>;
    return std::make_unique<SkThreadPool<WorkList, ExecutorType>>(numWorkLists,
                                                                  threads > 0 ? threads : num_cores(),
                                                                  allowBorrowing);
}
template <typename Work, typename Traits>
std::unique_ptr<SkExecutor<Work, Traits>> SkExecutor<Work, Traits>::MakeMultiListLIFOThreadPool(
        int numWorkLists,
        int threads,
        bool allowBorrowing) {
    using WorkList = TArray<Work>;
    using ExecutorType = SkExecutor<Work, Traits>;
    return std::make_unique<SkThreadPool<WorkList, ExecutorType>>(numWorkLists,
                                                                  threads > 0 ? threads : num_cores(),
                                                                  allowBorrowing);
}

/*
 * For Skia to use an SkExecutor it needs to explicitly needs to know about its templated
 * structure before hand. There is no case in which a client can create some new templated
 * flavor of SkExecutor and expect Skia to make use of it. Thus, we can force instantiation
 * here.
 */
template class SkExecutor<StdFunction, StdFunctionTraits>;
template class SkExecutor<ExecuteCallbackPair, ExecuteCallbackTraits>;
