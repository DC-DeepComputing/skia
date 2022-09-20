/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skiatest_Test_DEFINED
#define skiatest_Test_DEFINED

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkNoncopyable.h"
#include "include/private/SkTArray.h"
#include "src/core/SkTraceEvent.h"
#include "tests/CtsEnforcement.h"
#include "tools/Registry.h"

#include <atomic>
#include <cstdint>

struct GrContextOptions;

namespace skiatest {

SkString GetTmpDir();

struct Failure {
    Failure(const char* f, int l, const char* c, const SkString& m)
        : fileName(f), lineNo(l), condition(c), message(m) {}
    const char* fileName;
    int lineNo;
    const char* condition;
    SkString message;
    SkString toString() const;
};

class Reporter : SkNoncopyable {
public:
    virtual ~Reporter() {}
    virtual void bumpTestCount();
    virtual void reportFailed(const skiatest::Failure&) = 0;
    virtual bool allowExtendedTest() const;
    virtual bool verbose() const;
    virtual void* stats() const { return nullptr; }

    void reportFailedWithContext(const skiatest::Failure&);

    void push(const SkString& message) {
        fContextStack.push_back(message);
    }
    void pop() {
        fContextStack.pop_back();
    }

private:
    SkTArray<SkString> fContextStack;
};

#define REPORT_FAILURE(reporter, cond, message) \
    reporter->reportFailedWithContext(skiatest::Failure(__FILE__, __LINE__, cond, message))

class ReporterContext : SkNoncopyable {
public:
    ReporterContext(Reporter* reporter, const SkString& message) : fReporter(reporter) {
        fReporter->push(message);
    }
    ~ReporterContext() {
        fReporter->pop();
    }

private:
    Reporter* fReporter;
};

typedef void (*CPUTestProc)(skiatest::Reporter*);
typedef void (*GPUTestProc)(skiatest::Reporter*, const GrContextOptions&);
typedef void (*ContextOptionsProc)(GrContextOptions*);

enum TestType : uint8_t {
    kCPU_TestType,
    kGPUv1_TestType,
    kGPUv2_TestType,
    kGraphite_TestType = kGPUv2_TestType,
};

struct Test {
    Test(const char* name,
         CPUTestProc cpuProc)
            : fName(name)
            , fType(kCPU_TestType)
            , fCTSEnforcement(CtsEnforcement::kNever)
            , fCPUProc(cpuProc)
            , fGPUProc(nullptr)
            , fContextOptionsProc(nullptr) {}
    Test(const char* name,
         TestType gpuTestType,
         CtsEnforcement cts,
         GPUTestProc gpuProc,
         ContextOptionsProc optsProc = nullptr)
            : fName(name)
            , fType(gpuTestType)
            , fCTSEnforcement(cts)
            , fCPUProc(nullptr)
            , fGPUProc(gpuProc)
            , fContextOptionsProc(optsProc) {
        SkASSERT(fType != kCPU_TestType);
    }
    // FIXME(kjlubick) GPU (Ganesh) and Graphite options

    const char* fName;
    TestType fType;
    CtsEnforcement fCTSEnforcement;
    CPUTestProc fCPUProc;
    GPUTestProc fGPUProc;
    ContextOptionsProc fContextOptionsProc;

    void modifyGrContextOptions(GrContextOptions* options) {
        if (fContextOptionsProc) {
            (*fContextOptionsProc)(options);
        }
    }

    void run(skiatest::Reporter* r) const {
        SkASSERT(this->fCPUProc);
        TRACE_EVENT1("test_cpu", TRACE_FUNC, "name", this->fName/*these are static*/);
        this->fCPUProc(r);
    }

    void run(skiatest::Reporter* r, const GrContextOptions& options) const {
        SkASSERT(this->fGPUProc);
        TRACE_EVENT1("test_gpu", TRACE_FUNC, "name", this->fName/*these are static*/);
        this->fGPUProc(r, options);
    }
};

using TestRegistry = sk_tools::Registry<Test>;


/** Timer provides wall-clock duration since its creation. */
class Timer {
public:
    /** Starts the timer. */
    Timer();

    /** Nanoseconds since creation. */
    double elapsedNs() const;

    /** Milliseconds since creation. */
    double elapsedMs() const;

    /** Milliseconds since creation as an integer.
        Behavior is undefined for durations longer than SK_MSecMax.
    */
    SkMSec elapsedMsInt() const;
private:
    double fStartNanos;
};

}  // namespace skiatest

/*
    Use the following macros to make use of the skiatest classes, e.g.

    #include "tests/Test.h"

    DEF_TEST(TestName, reporter) {
        ...
        REPORTER_ASSERT(reporter, x == 15);
        ...
        REPORTER_ASSERT(reporter, x == 15, "x should be 15");
        ...
        if (x != 15) {
            ERRORF(reporter, "x should be 15, but is %d", x);
            return;
        }
        ...
    }
*/

#define REPORTER_ASSERT(r, cond, ...)                              \
    do {                                                           \
        if (!(cond)) {                                             \
            REPORT_FAILURE(r, #cond, SkStringPrintf(__VA_ARGS__)); \
        }                                                          \
    } while (0)

#define ERRORF(r, ...)                                      \
    do {                                                    \
        REPORT_FAILURE(r, "", SkStringPrintf(__VA_ARGS__)); \
    } while (0)

#define INFOF(REPORTER, ...)         \
    do {                             \
        if ((REPORTER)->verbose()) { \
            SkDebugf(__VA_ARGS__);   \
        }                            \
    } while (0)

#define DEF_CONDITIONAL_TEST(name, reporter, condition)                              \
    static void test_##name(skiatest::Reporter*);                                    \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name,                  \
                                                             test_##name),           \
                                              condition);                            \
    void test_##name(skiatest::Reporter* reporter)

#define DEF_TEST(name, reporter) DEF_CONDITIONAL_TEST(name, reporter, true)

#define DEF_TEST_DISABLED(name, reporter) DEF_CONDITIONAL_TEST(name, reporter, false)

#ifdef SK_BUILD_FOR_UNIX
    #define UNIX_ONLY_TEST DEF_TEST
#else
    #define UNIX_ONLY_TEST DEF_TEST_DISABLED
#endif

#define REQUIRE_PDF_DOCUMENT(TEST_NAME, REPORTER)                          \
    do {                                                                   \
        SkNullWStream testStream;                                          \
        auto testDoc = SkPDF::MakeDocument(&testStream);                   \
        if (!testDoc) {                                                    \
            INFOF(REPORTER, "PDF disabled; %s test skipped.", #TEST_NAME); \
            return;                                                        \
        }                                                                  \
    } while (false)

#endif
