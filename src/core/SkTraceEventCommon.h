// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef SkTraceEventCommon_DEFINED
#define SkTraceEventCommon_DEFINED

#include "include/core/SkTypes.h"
#include "include/utils/SkTraceEventPhase.h"

// Trace events are for tracking application performance and resource usage.
// Macros are provided to track:
//    Duration of scoped regions
//    Instantaneous events
//    Counters
//
// The first two arguments to all TRACE macros are the category and name. Both are strings, and
// must have application lifetime (statics or literals). The same applies to arg_names, and string
// argument values. However, you can force a copy of a string argument value with TRACE_STR_COPY:
//     TRACE_EVENT1("category", "name", "arg1", "literal string is only referenced");
//     TRACE_EVENT1("category", "name", "arg1", TRACE_STR_COPY("string will be copied"));
//
//
// Categories are used to group events, and
// can be enabled or disabled by the tracing framework. The trace system will automatically add the
// process id, thread id, and microsecond timestamp to all events.
//
//
// The TRACE_EVENT[0-2] macros trace the duration of entire scopes:
//   void doSomethingCostly() {
//     TRACE_EVENT0("MY_SUBSYSTEM", "doSomethingCostly");
//     ...
//   }
//
// Additional parameters can be associated with an event:
//   void doSomethingCostly2(int howMuch) {
//     TRACE_EVENT1("MY_SUBSYSTEM", "doSomethingCostly", "howMuch", howMuch);
//     ...
//   }
//
//
// Trace event also supports counters, which is a way to track a quantity as it varies over time.
// Counters are created with the following macro:
//   TRACE_COUNTER1("MY_SUBSYSTEM", "myCounter", g_myCounterValue);
//
// Counters are process-specific. The macro itself can be issued from any thread, however.
//
// Sometimes, you want to track two counters at once. You can do this with two counter macros:
//   TRACE_COUNTER1("MY_SUBSYSTEM", "myCounter0", g_myCounterValue[0]);
//   TRACE_COUNTER1("MY_SUBSYSTEM", "myCounter1", g_myCounterValue[1]);
// Or you can do it with a combined macro:
//   TRACE_COUNTER2("MY_SUBSYSTEM", "myCounter",
//                  "bytesPinned", g_myCounterValue[0],
//                  "bytesAllocated", g_myCounterValue[1]);
// The tracing UI will show these counters in a single graph, as a summed area chart.

#if defined(TRACE_EVENT0)
#error "Another copy of this file has already been included."
#endif

// --- Temporary Perfetto migration shim preamble ---
// Tracing on Android, and tracing with Perfetto, are both in a partially migrated state (but fully
// functional).
//
// For Android:
// ---
// 1. If SK_USE_PERFETTO_MACRO_OVERRIDES is not defined, then the prior ATrace implementation is
// used, and Skia won't link against/include Perfetto.
// SkAndroidFrameworkTraceUtil::setEnableTracing(bool) is used to control whether only some or all
// events are traced.
//
// 2. If SK_USE_PERFETTO_MACRO_OVERRIDES *is* defined, then the tracing backend can be switched
// between ATrace and Perfetto at runtime. This is currently *only* supported in Android framework.
// SkAndroidFrameworkTraceUtil::setEnableTracing(bool) will still control tracig overall, but
// SkAndroidFrameworkTraceUtil::setUsePerfettoTrackEvents(bool) will now determine whether that
// tracing is done with ATrace (default/false) or Perfetto (true).
//
// Note: if setUsePerfettoTrackEvents(true) is called, then Perfetto will remain initialized until
// the process ends. This means some minimal state overhead will remain even after subseqently
// switching the process back to ATrace, but individual trace events will be correctly routed to
// whichever system is active in the moment. However, trace events which have begun but have not yet
// ended when a switch occurs will likely be corrupted. Thus, it's best to minimize the frequency of
// switching backend tracing systems at runtime.
//
// For Perfetto outside of Android (e.g. tools):
// ---
// SK_USE_PERFETTO (mutually exclusive with SK_USE_PERFETTO_MACRO_OVERRIDES) can be used to unlock
// SkPerfettoTrace, which can be used for in-process tracing via the standard Skia tracing flow of
// SkEventTracer::SetInstance(...). This is enabled in tools with the `--trace perfetto` argument.
// See https://skia.org/docs/dev/tools/tracing/#tracing-with-perfetto for more on SK_USE_PERFETTO.

#ifdef SK_USE_PERFETTO_MACRO_OVERRIDES

// PERFETTO_TRACK_EVENT_NAMESPACE must be defined before including Perfetto. This allows Skia to
// maintain separate "track event" category storage, etc. from codebases linked into the same
// executable, and avoid symbol duplication errors.
//
// NOTE: A side-effect of this is we must use skia::TrackEvent instead of perfetto::TrackEvent.
#define PERFETTO_TRACK_EVENT_NAMESPACE skia
#include <perfetto/tracing.h>

// Currently kept entirely separate from SkPerfettoTrace for simplicity, which uses dynamic categories
// and doesn't need these static category definitions.
PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("GM"),
    perfetto::Category("skia"),
    perfetto::Category("skia.android"),
    perfetto::Category("skia.gpu"),
    perfetto::Category("skia.gpu.cache"),
    perfetto::Category("skia.objects"),
    perfetto::Category("skia.shaders"),
    perfetto::Category("skottie"),
    perfetto::Category("test"),
    perfetto::Category("test_cpu"),
    perfetto::Category("test_ganesh"),
    perfetto::Category("test_graphite"),
);

// Allows callers to optionally pass strings that aren't known at compile time to trace functions.
//
// E.g. TRACE_EVENT0("skia", DYNAMIC_TRACE_STRING(functionThatReturnsString()));
// No-op when Perfetto is disabled, or outside of Android framework.
#define DYNAMIC_TRACE_STRING ::perfetto::DynamicString

// ATrace can't accept ::perfetto::DynamicString, so any trace event names that were wrapped in
// DYNAMIC_TRACE_STRING need to be unboxed back to char* before being passed to ATrace.
inline const char* skUnboxPerfettoDynamicString(const DYNAMIC_TRACE_STRING& str) {
    return str.value;
}
inline const char* skUnboxPerfettoDynamicString(const char* str) {
    return str;
}

#else // !SK_USE_PERFETTO_MACRO_OVERRIDES
// Allows callers to optionally pass strings that aren't known at compile time to trace functions.
//
// E.g. TRACE_EVENT0("skia", DYNAMIC_TRACE_STRING(functionThatReturnsString()));
// No-op when Perfetto is disabled, or outside of Android framework.
#define DYNAMIC_TRACE_STRING(str) str
#endif // SK_USE_PERFETTO_MACRO_OVERRIDES

#ifdef SK_DEBUG
static void skprintf_like_noop(const char format[], ...) SK_PRINTF_LIKE(1, 2);
static inline void skprintf_like_noop(const char format[], ...) {}
static inline void sk_noop(...) {}
#define TRACE_EMPTY(...) do { sk_noop(__VA_ARGS__); } while (0)
#define TRACE_EMPTY_FMT(fmt, ...) do { skprintf_like_noop(fmt, ##__VA_ARGS__); } while (0)
#else
#define TRACE_EMPTY(...) do {} while (0)
#define TRACE_EMPTY_FMT(fmt, ...) do {} while (0)
#endif

#ifdef SK_DISABLE_TRACING

#define ATRACE_ANDROID_FRAMEWORK(fmt, ...) TRACE_EMPTY_FMT(fmt, ##__VA_ARGS__)
#define ATRACE_ANDROID_FRAMEWORK_ALWAYS(fmt, ...) TRACE_EMPTY_FMT(fmt, ##__VA_ARGS__)
#define TRACE_EVENT0(cg, n) TRACE_EMPTY(cg, n)
#define TRACE_EVENT0_ALWAYS(cg, n) TRACE_EMPTY(cg, n)
#define TRACE_EVENT1(cg, n, a1n, a1v) TRACE_EMPTY(cg, n, a1n, a1v)
#define TRACE_EVENT2(cg, n, a1n, a1v, a2n, a2v) TRACE_EMPTY(cg, n, a1n, a1v, a2n, a2v)
#define TRACE_EVENT_INSTANT0(cg, n, scope) TRACE_EMPTY(cg, n, scope)
#define TRACE_EVENT_INSTANT1(cg, n, scope, a1n, a1v) TRACE_EMPTY(cg, n, scope, a1n, a1v)
#define TRACE_EVENT_INSTANT2(cg, n, scope, a1n, a1v, a2n, a2v)  \
    TRACE_EMPTY(cg, n, scope, a1n, a1v, a2n, a2v)
#define TRACE_COUNTER1(cg, n, value) TRACE_EMPTY(cg, n, value)
#define TRACE_COUNTER2(cg, n, v1n, v1v, v2n, v2v) TRACE_EMPTY(cg, n, v1n, v1v, v2n, v2v)

#elif defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)

#include <cutils/trace.h>
#include <stdarg.h>

class SkAndroidFrameworkTraceUtil {
public:
    SkAndroidFrameworkTraceUtil(const char* name) {
        if (CC_UNLIKELY(gEnableAndroidTracing)) {
            ATRACE_BEGIN(name);
        }
    }
    SkAndroidFrameworkTraceUtil(bool, const char* fmt, ...) {
        if (CC_LIKELY((!gEnableAndroidTracing) || (!ATRACE_ENABLED()))) return;

        const int BUFFER_SIZE = 256;
        va_list ap;
        char buf[BUFFER_SIZE];

        va_start(ap, fmt);
        vsnprintf(buf, BUFFER_SIZE, fmt, ap);
        va_end(ap);

        ATRACE_BEGIN(buf);
    }
    ~SkAndroidFrameworkTraceUtil() {
        if (CC_UNLIKELY(gEnableAndroidTracing)) {
            ATRACE_END();
        }
    }

    // Controls whether tracing is enabled.
    //
    // Atrace is used by default, but can be replaced with Perfetto by calling setUsePerfettoTrackEvents(true)
    static void setEnableTracing(bool enableAndroidTracing) {
#ifdef SK_USE_PERFETTO_MACRO_OVERRIDES
        // If Perfetto is already the preferred tracing method, then ensure it is initialized when
        // tracing in general transitions from disabled to enabled.
        if (!gEnableAndroidTracing && enableAndroidTracing && gUsePerfettoTrackEvents) {
            initPerfetto();
        }
#endif // SK_USE_PERFETTO_MACRO_OVERRIDES
        gEnableAndroidTracing = enableAndroidTracing;
    }

    // Controls whether tracing uses Perfetto track events instead of Atrace.
    //
    // Returns true if Skia was built with Perfetto, false otherwise.
    //
    static bool setUsePerfettoTrackEvents(bool usePerfettoTrackEvents) {
#ifdef SK_USE_PERFETTO_MACRO_OVERRIDES
        // If tracing is already enabled, then ensure Perfetto is initialized when it is chosen as
        // the preferred tracing method.
        if (!gUsePerfettoTrackEvents && usePerfettoTrackEvents && gEnableAndroidTracing) {
            initPerfetto();
        }
        gUsePerfettoTrackEvents = usePerfettoTrackEvents;
        return true;
#else // !SK_USE_PERFETTO_MACRO_OVERRIDES
        // Note: please reach out if you encounter this message unexpectedly.
        SkDebugf("Tracing Skia with Perfetto is not supported in this environment! (Host build?)");
        return false;
#endif // SK_USE_PERFETTO_MACRO_OVERRIDES
    }

    static bool getEnableTracing() {
        return gEnableAndroidTracing;
    }

    static bool getUsePerfettoTrackEvents() {
        return gUsePerfettoTrackEvents;
    }

private:
    static bool gEnableAndroidTracing;
    static bool gUsePerfettoTrackEvents;

#ifdef SK_USE_PERFETTO_MACRO_OVERRIDES
    // Initializes tracing systems, and establishes a connection to the 'traced' daemon.
    //
    // Can be called multiple times.
    static void initPerfetto() {
        ::perfetto::TracingInitArgs perfettoArgs;
        perfettoArgs.backends |= perfetto::kSystemBackend;
        ::perfetto::Tracing::Initialize(perfettoArgs);
        ::skia::TrackEvent::Register();
    }
#endif // SK_USE_PERFETTO_MACRO_OVERRIDES
};

class SkAndroidFrameworkTraceUtilAlways {
public:
    SkAndroidFrameworkTraceUtilAlways(const char* name) {
        ATRACE_BEGIN(name);
    }

    SkAndroidFrameworkTraceUtilAlways(bool, const char* fmt, ...) {
        if (!ATRACE_ENABLED()) return;

        const int BUFFER_SIZE = 256;
        va_list ap;
        char buf[BUFFER_SIZE];

        va_start(ap, fmt);
        vsnprintf(buf, BUFFER_SIZE, fmt, ap);
        va_end(ap);

        ATRACE_BEGIN(buf);
    }
    ~SkAndroidFrameworkTraceUtilAlways() {
        ATRACE_END();
    }
};

#ifndef SK_USE_PERFETTO_MACRO_OVERRIDES // Normal ATrace route first for clarity.

#define ATRACE_ANDROID_FRAMEWORK(fmt, ...) SkAndroidFrameworkTraceUtil __trace(true, fmt, ##__VA_ARGS__)
#define ATRACE_ANDROID_FRAMEWORK_ALWAYS(fmt, ...) SkAndroidFrameworkTraceUtilAlways __trace_always(true, fmt, ##__VA_ARGS__)

// Records a pair of begin and end events called "name" for the current scope, with 0, 1 or 2
// associated arguments. In the framework, the arguments are ignored.
#define TRACE_EVENT0(category_group, name) \
    SkAndroidFrameworkTraceUtil __trace(name)
#define TRACE_EVENT0_ALWAYS(category_group, name) \
    SkAndroidFrameworkTraceUtilAlways __trace_always(name)
#define TRACE_EVENT1(category_group, name, arg1_name, arg1_val) \
    SkAndroidFrameworkTraceUtil __trace(name)
#define TRACE_EVENT2(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val) \
    SkAndroidFrameworkTraceUtil __trace(name)

// Records a single event called "name" immediately, with 0, 1 or 2 associated arguments. If the
// category is not enabled, then this does nothing.
#define TRACE_EVENT_INSTANT0(category_group, name, scope) \
    do { SkAndroidFrameworkTraceUtil __trace(name); } while(0)

#define TRACE_EVENT_INSTANT1(category_group, name, scope, arg1_name, arg1_val) \
    do { SkAndroidFrameworkTraceUtil __trace(name); } while(0)

#define TRACE_EVENT_INSTANT2(category_group, name, scope, arg1_name, arg1_val, \
                             arg2_name, arg2_val)                              \
    do { SkAndroidFrameworkTraceUtil __trace(name); } while(0)

// Records the value of a counter called "name" immediately. Value
// must be representable as a 32 bit integer.
#define TRACE_COUNTER1(category_group, name, value) \
    if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) { \
        ATRACE_INT(name, value); \
    }

// Records the values of a multi-parted counter called "name" immediately.
// In Chrome, this macro produces a stacked bar chart. ATrace doesn't support
// that, so this just produces two separate counters.
#define TRACE_COUNTER2(category_group, name, value1_name, value1_val, value2_name, value2_val) \
    do { \
        if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) { \
            ATRACE_INT(name "-" value1_name, value1_val); \
            ATRACE_INT(name "-" value2_name, value2_val); \
        } \
    } while (0)

// ATrace has no object tracking
#define TRACE_EVENT_OBJECT_CREATED_WITH_ID(category_group, name, id) \
    TRACE_EMPTY(category_group, name, id)
#define TRACE_EVENT_OBJECT_SNAPSHOT_WITH_ID(category_group, name, id, snapshot) \
    TRACE_EMPTY(category_group, name, id, snapshot)
#define TRACE_EVENT_OBJECT_DELETED_WITH_ID(category_group, name, id) \
    TRACE_EMPTY(category_group, name, id)

// Macro to efficiently determine if a given category group is enabled.
// This is only used for some shader text logging that isn't supported in ATrace anyway.
#define TRACE_EVENT_CATEGORY_GROUP_ENABLED(category_group, ret)             \
  do { *ret = false; } while (0)

#else // SK_USE_PERFETTO_MACRO_OVERRIDES (allows runtime switching between ATrace and Perfetto)

// Generate a unique variable name with a given prefix.
#define PERFETTO_INTERNAL_CONCAT2(a, b) a##b
#define PERFETTO_INTERNAL_CONCAT(a, b) PERFETTO_INTERNAL_CONCAT2(a, b)
#define PERFETTO_UID(prefix) PERFETTO_INTERNAL_CONCAT(prefix, __LINE__)

// If tracing is enabled, then a trace event will be created with either ATrace or Perfetto.
// If tracing is disabled but force_always_trace is true, then an event will be traced with ATrace,
// regardless of whether ATrace or Perfetto is the currently preferred tracing backend. This quirk
// is to work around the fact that Perfetto would not have been initialized if overall tracing is
// disabled, and is an edge-case scenario that shouldn't arise under correct usage.
#define TRACE_EVENT_ATRACE_OR_PERFETTO_FORCEABLE(force_always_trace, category, name, ...)   \
    struct PERFETTO_UID(ScopedEvent) {                                                      \
        struct EventFinalizer {                                                             \
            /* The parameter is an implementation detail. It allows the          */         \
            /* anonymous struct to use aggregate initialization to invoke the    */         \
            /* lambda (which emits the BEGIN event and returns an integer)       */         \
            /* with the proper reference capture for any                         */         \
            /* TrackEventArgumentFunction in |__VA_ARGS__|. This is required so  */         \
            /* that the scoped event is exactly ONE line and can't escape the    */         \
            /* scope if used in a single line if statement.                      */         \
            EventFinalizer(...) {}                                                          \
            ~EventFinalizer() {                                                             \
                if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing()             \
                        && SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents())) {     \
                    TRACE_EVENT_END(category);                                              \
                }                                                                           \
                else if (force_always_trace                                                 \
                        || CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) {  \
                    ATRACE_END();                                                           \
                }                                                                           \
            }                                                                               \
                                                                                            \
            EventFinalizer(const EventFinalizer&) = delete;                                 \
            inline EventFinalizer& operator=(const EventFinalizer&) = delete;               \
                                                                                            \
            EventFinalizer(EventFinalizer&&) = default;                                     \
            EventFinalizer& operator=(EventFinalizer&&) = delete;                           \
        } finalizer;                                                                        \
    } PERFETTO_UID(scoped_event) {                                                          \
        [&]() {                                                                             \
            if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing()                 \
                    && SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents())) {         \
                TRACE_EVENT_BEGIN(category, name, ##__VA_ARGS__);                           \
            }                                                                               \
            else if (force_always_trace                                                     \
                    || CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) {      \
                ATRACE_BEGIN(skUnboxPerfettoDynamicString(name));                           \
            }                                                                               \
            return 0;                                                                       \
        }()                                                                                 \
    }

// Records an event with the current tracing backend, if overall Skia tracing is also enabled.
#define TRACE_EVENT_ATRACE_OR_PERFETTO(category, name, ...)                     \
    TRACE_EVENT_ATRACE_OR_PERFETTO_FORCEABLE(                                   \
            /* force_always_trace = */ false, category, name, ##__VA_ARGS__)

#define ATRACE_ANDROID_FRAMEWORK_BUFFER_SIZE 256
#define ATRACE_ANDROID_FRAMEWORK(fmt, ...)                                                                  \
    char PERFETTO_UID(skTraceStrBuf)[ATRACE_ANDROID_FRAMEWORK_BUFFER_SIZE];                                 \
    if (SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents()) {                                         \
        snprintf(PERFETTO_UID(skTraceStrBuf), ATRACE_ANDROID_FRAMEWORK_BUFFER_SIZE, fmt, ##__VA_ARGS__);    \
    }                                                                                                       \
    TRACE_EVENT0("skia.android", DYNAMIC_TRACE_STRING(PERFETTO_UID(skTraceStrBuf)))

#define ATRACE_ANDROID_FRAMEWORK_ALWAYS(fmt, ...)                                                       \
    char PERFETTO_UID(skTraceStrBuf)[ATRACE_ANDROID_FRAMEWORK_BUFFER_SIZE];                             \
    snprintf(PERFETTO_UID(skTraceStrBuf), ATRACE_ANDROID_FRAMEWORK_BUFFER_SIZE, fmt, ##__VA_ARGS__);    \
    TRACE_EVENT0_ALWAYS("skia.android", DYNAMIC_TRACE_STRING(PERFETTO_UID(skTraceStrBuf)))

// Records a pair of begin and end events called "name" for the current scope, with 0, 1 or 2
// associated arguments. Arguments are only recorded when Perfetto is set as the current backend!
#define TRACE_EVENT0(category_group, name) \
    TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name)
#define TRACE_EVENT0_ALWAYS(category_group, name) \
    TRACE_EVENT_ATRACE_OR_PERFETTO_FORCEABLE(/* force_always_trace = */ true, category_group, name)
#define TRACE_EVENT1(category_group, name, arg1_name, arg1_val) \
    TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name, arg1_name, arg1_val)
#define TRACE_EVENT2(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val) \
    TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val)

// Records a single event called "name" immediately, with 0, 1 or 2 associated arguments.
// Arguments are only recorded when Perfetto is set as the current backend!
#define TRACE_EVENT_INSTANT0(category_group, name, scope) \
    do { TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name); } while(0)

#define TRACE_EVENT_INSTANT1(category_group, name, scope, arg1_name, arg1_val) \
    do { TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name, arg1_name, arg1_val); } while(0)

#define TRACE_EVENT_INSTANT2(category_group, name, scope, arg1_name, arg1_val, \
                             arg2_name, arg2_val)                              \
    do { TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val); } while(0)

// Records the value of a counter called "name" immediately. Value
// must be representable as a 32 bit integer.
#define TRACE_COUNTER1(category_group, name, value)                     \
    if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) { \
        if (SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents()) { \
            TRACE_COUNTER(category_group, name, value);                 \
        }                                                               \
        else {                                                          \
            ATRACE_INT(name, value);                                    \
        }                                                               \
    }

// Records the values of a multi-parted counter called "name" immediately.
// In Chrome, this macro produces a stacked bar chart. Perfetto doesn't support
// that (related: b/242349575), so this just produces two separate counters.
#define TRACE_COUNTER2(category_group, name, value1_name, value1_val, value2_name, value2_val)      \
    if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) {                             \
        if (SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents()) {                             \
            TRACE_COUNTER(category_group, name "-" value1_name, value1_val);                        \
            TRACE_COUNTER(category_group, name "-" value2_name, value2_val);                        \
        }                                                                                           \
        else {                                                                                      \
            ATRACE_INT(name "-" value1_name, value1_val);                                           \
            ATRACE_INT(name "-" value2_name, value2_val);                                           \
        }                                                                                           \
    }

// ATrace has no object tracking, and would require a legacy shim for Perfetto (which likely no-ops
// here). Further, these don't appear to currently be used outside of tests.
#define TRACE_EVENT_OBJECT_CREATED_WITH_ID(category_group, name, id) \
    TRACE_EMPTY(category_group, name, id)
#define TRACE_EVENT_OBJECT_SNAPSHOT_WITH_ID(category_group, name, id, snapshot) \
    TRACE_EMPTY(category_group, name, id, snapshot)
#define TRACE_EVENT_OBJECT_DELETED_WITH_ID(category_group, name, id) \
    TRACE_EMPTY(category_group, name, id)

// Macro to efficiently determine if a given category group is enabled. Only works with Perfetto.
// This is only used for some shader text logging that isn't supported in ATrace anyway.
#define TRACE_EVENT_CATEGORY_GROUP_ENABLED(category_group, ret)                     \
    if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing()                 \
                 && SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents)) {      \
        *ret = TRACE_EVENT_CATEGORY_ENABLED(category_group);                        \
    }                                                                               \
    else {                                                                          \
        *ret = false;                                                               \
    }

#endif // SK_USE_PERFETTO_MACRO_OVERRIDES

#else // !SK_BUILD_FOR_ANDROID_FRAMEWORK && !SK_DISABLE_TRACING

#define ATRACE_ANDROID_FRAMEWORK(fmt, ...) TRACE_EMPTY_FMT(fmt, ##__VA_ARGS__)
#define ATRACE_ANDROID_FRAMEWORK_ALWAYS(fmt, ...) TRACE_EMPTY_FMT(fmt, ##__VA_ARGS__)

// Records a pair of begin and end events called "name" for the current scope, with 0, 1 or 2
// associated arguments. If the category is not enabled, then this does nothing.
#define TRACE_EVENT0(category_group, name) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name)

#define TRACE_EVENT0_ALWAYS(category_group, name) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name)

#define TRACE_EVENT1(category_group, name, arg1_name, arg1_val) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name, arg1_name, arg1_val)

#define TRACE_EVENT2(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val)

// Records a single event called "name" immediately, with 0, 1 or 2 associated arguments. If the
// category is not enabled, then this does nothing.
#define TRACE_EVENT_INSTANT0(category_group, name, scope)                   \
  INTERNAL_TRACE_EVENT_ADD(TRACE_EVENT_PHASE_INSTANT, category_group, name, \
                           TRACE_EVENT_FLAG_NONE | scope)

#define TRACE_EVENT_INSTANT1(category_group, name, scope, arg1_name, arg1_val) \
  INTERNAL_TRACE_EVENT_ADD(TRACE_EVENT_PHASE_INSTANT, category_group, name,    \
                           TRACE_EVENT_FLAG_NONE | scope, arg1_name, arg1_val)

#define TRACE_EVENT_INSTANT2(category_group, name, scope, arg1_name, arg1_val, \
                             arg2_name, arg2_val)                              \
  INTERNAL_TRACE_EVENT_ADD(TRACE_EVENT_PHASE_INSTANT, category_group, name,    \
                           TRACE_EVENT_FLAG_NONE | scope, arg1_name, arg1_val, \
                           arg2_name, arg2_val)

// Records the value of a counter called "name" immediately. Value
// must be representable as a 32 bit integer.
#define TRACE_COUNTER1(category_group, name, value)                         \
  INTERNAL_TRACE_EVENT_ADD(TRACE_EVENT_PHASE_COUNTER, category_group, name, \
                           TRACE_EVENT_FLAG_NONE, "value",                  \
                           static_cast<int>(value))

// Records the values of a multi-parted counter called "name" immediately.
// The UI will treat value1 and value2 as parts of a whole, displaying their
// values as a stacked-bar chart.
#define TRACE_COUNTER2(category_group, name, value1_name, value1_val,       \
                       value2_name, value2_val)                             \
  INTERNAL_TRACE_EVENT_ADD(TRACE_EVENT_PHASE_COUNTER, category_group, name, \
                           TRACE_EVENT_FLAG_NONE, value1_name,              \
                           static_cast<int>(value1_val), value2_name,       \
                           static_cast<int>(value2_val))

#define TRACE_EVENT_ASYNC_BEGIN0(category, name, id)                                           \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(                                                          \
        TRACE_EVENT_PHASE_ASYNC_BEGIN, category, name, id, TRACE_EVENT_FLAG_NONE)
#define TRACE_EVENT_ASYNC_BEGIN1(category, name, id, arg1_name, arg1_val)                      \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(TRACE_EVENT_PHASE_ASYNC_BEGIN,                            \
        category, name, id, TRACE_EVENT_FLAG_NONE, arg1_name, arg1_val)
#define TRACE_EVENT_ASYNC_BEGIN2(category, name, id, arg1_name, arg1_val, arg2_name, arg2_val) \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(TRACE_EVENT_PHASE_ASYNC_BEGIN,                            \
        category, name, id, TRACE_EVENT_FLAG_NONE, arg1_name, arg1_val, arg2_name, arg2_val)

#define TRACE_EVENT_ASYNC_END0(category, name, id)                                           \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(TRACE_EVENT_PHASE_ASYNC_END,                            \
        category, name, id, TRACE_EVENT_FLAG_NONE)
#define TRACE_EVENT_ASYNC_END1(category, name, id, arg1_name, arg1_val)                      \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(TRACE_EVENT_PHASE_ASYNC_END,                            \
        category, name, id, TRACE_EVENT_FLAG_NONE, arg1_name, arg1_val)
#define TRACE_EVENT_ASYNC_END2(category, name, id, arg1_name, arg1_val, arg2_name, arg2_val) \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(TRACE_EVENT_PHASE_ASYNC_END,                            \
        category, name, id, TRACE_EVENT_FLAG_NONE, arg1_name, arg1_val, arg2_name, arg2_val)

// Macros to track the life time and value of arbitrary client objects.
#define TRACE_EVENT_OBJECT_CREATED_WITH_ID(category_group, name, id) \
  INTERNAL_TRACE_EVENT_ADD_WITH_ID(                                  \
      TRACE_EVENT_PHASE_CREATE_OBJECT, category_group, name, id,     \
      TRACE_EVENT_FLAG_NONE)

#define TRACE_EVENT_OBJECT_SNAPSHOT_WITH_ID(category_group, name, id, \
                                            snapshot)                 \
  INTERNAL_TRACE_EVENT_ADD_WITH_ID(                                   \
      TRACE_EVENT_PHASE_SNAPSHOT_OBJECT, category_group, name,        \
      id, TRACE_EVENT_FLAG_NONE, "snapshot", snapshot)

#define TRACE_EVENT_OBJECT_DELETED_WITH_ID(category_group, name, id) \
  INTERNAL_TRACE_EVENT_ADD_WITH_ID(                                  \
      TRACE_EVENT_PHASE_DELETE_OBJECT, category_group, name, id,     \
      TRACE_EVENT_FLAG_NONE)

// Macro to efficiently determine if a given category group is enabled.
#define TRACE_EVENT_CATEGORY_GROUP_ENABLED(category_group, ret)             \
  do {                                                                      \
    INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group);                 \
    if (INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE()) { \
      *ret = true;                                                          \
    } else {                                                                \
      *ret = false;                                                         \
    }                                                                       \
  } while (0)

#endif

// Flags for changing the behavior of TRACE_EVENT_API_ADD_TRACE_EVENT.
#define TRACE_EVENT_FLAG_NONE (static_cast<unsigned int>(0))
#define TRACE_EVENT_FLAG_COPY (static_cast<unsigned int>(1 << 0))
#define TRACE_EVENT_FLAG_HAS_ID (static_cast<unsigned int>(1 << 1))
#define TRACE_EVENT_FLAG_MANGLE_ID (static_cast<unsigned int>(1 << 2))
#define TRACE_EVENT_FLAG_SCOPE_OFFSET (static_cast<unsigned int>(1 << 3))
#define TRACE_EVENT_FLAG_SCOPE_EXTRA (static_cast<unsigned int>(1 << 4))
#define TRACE_EVENT_FLAG_EXPLICIT_TIMESTAMP (static_cast<unsigned int>(1 << 5))
#define TRACE_EVENT_FLAG_ASYNC_TTS (static_cast<unsigned int>(1 << 6))
#define TRACE_EVENT_FLAG_BIND_TO_ENCLOSING (static_cast<unsigned int>(1 << 7))
#define TRACE_EVENT_FLAG_FLOW_IN (static_cast<unsigned int>(1 << 8))
#define TRACE_EVENT_FLAG_FLOW_OUT (static_cast<unsigned int>(1 << 9))
#define TRACE_EVENT_FLAG_HAS_CONTEXT_ID (static_cast<unsigned int>(1 << 10))

#define TRACE_EVENT_FLAG_SCOPE_MASK                          \
  (static_cast<unsigned int>(TRACE_EVENT_FLAG_SCOPE_OFFSET | \
                             TRACE_EVENT_FLAG_SCOPE_EXTRA))

// Type values for identifying types in the TraceValue union.
#define TRACE_VALUE_TYPE_BOOL (static_cast<unsigned char>(1))
#define TRACE_VALUE_TYPE_UINT (static_cast<unsigned char>(2))
#define TRACE_VALUE_TYPE_INT (static_cast<unsigned char>(3))
#define TRACE_VALUE_TYPE_DOUBLE (static_cast<unsigned char>(4))
#define TRACE_VALUE_TYPE_POINTER (static_cast<unsigned char>(5))
#define TRACE_VALUE_TYPE_STRING (static_cast<unsigned char>(6))
#define TRACE_VALUE_TYPE_COPY_STRING (static_cast<unsigned char>(7))
#define TRACE_VALUE_TYPE_CONVERTABLE (static_cast<unsigned char>(8))

// Enum reflecting the scope of an INSTANT event. Must fit within TRACE_EVENT_FLAG_SCOPE_MASK.
#define TRACE_EVENT_SCOPE_GLOBAL (static_cast<unsigned char>(0 << 3))
#define TRACE_EVENT_SCOPE_PROCESS (static_cast<unsigned char>(1 << 3))
#define TRACE_EVENT_SCOPE_THREAD (static_cast<unsigned char>(2 << 3))

#define TRACE_EVENT_SCOPE_NAME_GLOBAL ('g')
#define TRACE_EVENT_SCOPE_NAME_PROCESS ('p')
#define TRACE_EVENT_SCOPE_NAME_THREAD ('t')

#endif  // SkTraceEventCommon_DEFINED
