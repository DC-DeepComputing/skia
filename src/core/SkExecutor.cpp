/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkSemaphore.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkNoDestructor.h"

#include <deque>
#include <thread>
#include <utility>

using namespace skia_private;

#if defined(SK_BUILD_FOR_WIN)
    #include "src/base/SkLeanWindows.h"
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

SkExecutor::~SkExecutor() {}

// The default default SkExecutor is an SkTrivialExecutor, which just runs the work right away.
class SkTrivialExecutor final : public SkExecutor {
    void add1(std::function<void(void)> work) override {
        work();
    }
    void add2(std::function<void(void)> work, int /* workQueue */) override {
        work();
    }
};

static SkExecutor& trivial_executor() {
    static SkNoDestructor<SkTrivialExecutor> executor;
    return *executor;
}

static SkExecutor* gDefaultExecutor = nullptr;

SkExecutor& SkExecutor::GetDefault() {
    if (gDefaultExecutor) {
        return *gDefaultExecutor;
    }
    return trivial_executor();
}

void SkExecutor::SetDefault(SkExecutor* executor) {
    gDefaultExecutor = executor;
}

// We'll always push_back() new work, but pop from the front of deques or the back of SkTArray.
static inline std::function<void(void)> pop(std::deque<std::function<void(void)>>* list) {
    std::function<void(void)> fn = std::move(list->front());
    list->pop_front();
    return fn;
}
static inline std::function<void(void)> pop(TArray<std::function<void(void)>>* list) {
    std::function<void(void)> fn = std::move(list->back());
    list->pop_back();
    return fn;
}

// An SkThreadPool is an executor that runs work on a fixed pool of OS threads.
template <typename WorkList>
class SkThreadPool final : public SkExecutor {
public:
    explicit SkThreadPool(int numWorkQueues, int threads, bool allowBorrowing)
        : fNumWorkQueues(numWorkQueues < 1 ? 1 : numWorkQueues)
        , fAllowBorrowing(allowBorrowing) {

        fWorkQueues = new WorkList[fNumWorkQueues];

        for (int i = 0; i < threads; i++) {
            fThreads.emplace_back(&Loop, this);
        }
    }

    ~SkThreadPool() override {
        // Signal each thread that it's time to shut down.
        for (int i = 0; i < fThreads.size(); i++) {
            // Add the notification to the highest priority queue
            this->add2(nullptr, /* workQueue= */ 0);
        }
        // Wait for each thread to shut down.
        for (int i = 0; i < fThreads.size(); i++) {
            fThreads[i].join();
        }
    }

    void add1(std::function<void(void)> work) override {
        this->add2(work, /* workQueue= */ 0);
    }

    void add2(std::function<void(void)> work, int workQueue) override {
        workQueue = SkTPin(workQueue, 0, fNumWorkQueues-1);

        // Add some work to our pile of work to do.
        {
            SkAutoMutexExclusive lock(fWorkLock);

            fWorkQueues[workQueue].emplace_back(std::move(work));
        }
        // Tell the Loop() threads to pick it up.
        fWorkAvailable.signal(1);
    }

    void borrow1() override {
        // If there is work waiting and we're allowed to borrow work, do it.
        if (fAllowBorrowing && fWorkAvailable.try_wait()) {
            SkAssertResult(this->do_work());
        }
    }

private:
    // This method should be called only when fWorkAvailable indicates there's work to do.
    bool do_work() {
        std::function<void(void)> work;
        {
            SkAutoMutexExclusive lock(fWorkLock);

            // TODO: return true if all the work queues are empty ?
            for (int i = 0; i < fNumWorkQueues; ++i) {
                if (!fWorkQueues[i].empty()) {
                    work = pop(&fWorkQueues[i]);
                    break;
                }
            }
        }

        if (!work) {
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

    TArray<std::thread> fThreads;
    int                 fNumWorkQueues; // >= 1
    WorkList*           fWorkQueues;
    Lock                fWorkLock;
    SkSemaphore         fWorkAvailable;
    bool                fAllowBorrowing;
};

std::unique_ptr<SkExecutor> SkExecutor::MakeFIFOThreadPool1(int threads, bool allowBorrowing) {
    using WorkList = std::deque<std::function<void(void)>>;
    return std::make_unique<SkThreadPool<WorkList>>(/* numWorkQueues= */ 1,
                                                    threads > 0 ? threads : num_cores(),
                                                    allowBorrowing);
}
std::unique_ptr<SkExecutor> SkExecutor::MakeLIFOThreadPool1(int threads, bool allowBorrowing) {
    using WorkList = TArray<std::function<void(void)>>;
    return std::make_unique<SkThreadPool<WorkList>>(/* numWorkQueues= */ 1,
                                                    threads > 0 ? threads : num_cores(),
                                                    allowBorrowing);
}

std::unique_ptr<SkExecutor> SkExecutor::MakeMultiQueueFIFOThreadPool(int numWorkQueues,
                                                                     int threads,
                                                                     bool allowBorrowing) {
    using WorkList = std::deque<std::function<void(void)>>;
    return std::make_unique<SkThreadPool<WorkList>>(numWorkQueues,
                                                    threads > 0 ? threads : num_cores(),
                                                    allowBorrowing);
}
std::unique_ptr<SkExecutor> SkExecutor::MakeMultiQueueLIFOThreadPool(int numWorkQueues,
                                                                     int threads,
                                                                     bool allowBorrowing) {
    using WorkList = TArray<std::function<void(void)>>;
    return std::make_unique<SkThreadPool<WorkList>>(numWorkQueues,
                                                    threads > 0 ? threads : num_cores(),
                                                    allowBorrowing);
}
