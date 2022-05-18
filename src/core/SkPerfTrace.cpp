/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPerfTrace.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkTraceEventCommon.h"
#include <fcntl.h>
#include <perfetto.h>
#include <variant>
#include "tools/flags/CommandLineFlags.h"

/* Define a template to help handle all the possible TRACE_EVENT_BEGIN macro call
 * combinations with 2 arguments of various types (defined in TraceValueUnion). */
template <typename T, class X>
void beginEventWithSecondArg(X category, const char* eventName, const char* arg1Name,
                          T arg1Val, const char* arg2Name,
                          const uint8_t& arg2Type, const uint64_t& arg2Val) {

      skia::tracing_internals::TraceValueUnion value;
      value.as_uint = arg2Val;

      switch (arg2Type) {
        case TRACE_VALUE_TYPE_BOOL:
          TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val, arg2Name, value.as_bool, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(eventName);
          });
        break;
        case TRACE_VALUE_TYPE_UINT:
          TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val, arg2Name, value.as_uint, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(eventName);
          });
          break;
        case TRACE_VALUE_TYPE_INT:
          TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val, arg1Name, value.as_int, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(eventName);
          });
          break;
        case TRACE_VALUE_TYPE_DOUBLE:
          TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val, arg2Name, value.as_double, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(eventName);
          });
          break;
        case TRACE_VALUE_TYPE_POINTER:
          TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val, arg2Name, value.as_pointer, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(eventName);
          });
          break;
        case TRACE_VALUE_TYPE_STRING:
          TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val, arg2Name, value.as_string, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(eventName);
          });
          break;
        case TRACE_VALUE_TYPE_COPY_STRING:
          TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val, arg2Name, value.as_string, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(eventName);
          });
          break;
        default:
          TRACE_EVENT_BEGIN(category, nullptr, arg1Name, arg1Val, arg2Name, value.as_uint, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(eventName);
          });
          break;
      }
}

static DEFINE_bool(splitPerfettoTracesByBenchmark, true,
                   "Create separate perfetto trace files for each benchmark?\n"
                   "Will only take effect if perfetto tracing is enabled. See --trace.");

static DEFINE_int(perfettoCMBSize, 32000,
                  "Perfetto central memory buffer size in KB\n"
                  "Will only have an effect if perfetto tracing is enabled. See --trace.");
static DEFINE_int(perfettoSMBSize, 2000,
                  "Perfetto shared memory buffer size in KB\n"
                  "Will only have an effect if perfetto tracing is enabled. See --trace.");
static DEFINE_int(perfettoFlushPeriod, 10000,
                  "Perfetto buffer flush period in ms\n"
                  "Will only have an effect if perfetto tracing is enabled. See --trace.");
static DEFINE_int(perfettoFileWritePeriod, 5000,
                  "How often perfetto should perform a file write operation in ms\n"
                  "Will only have an effect if perfetto tracing is enabled. See --trace.");

SkPerfTrace::SkPerfTrace() {
    if (!perfetto::Tracing::IsInitialized()) {
      openNewTracingSession(outputFileName);
    }
}

SkPerfTrace::~SkPerfTrace() {
  closeTracingSession();
}

SkEventTracer::Handle SkPerfTrace::addTraceEvent(char phase,
                                              const uint8_t* categoryEnabledFlag,
                                              const char* name,
                                              uint64_t id,
                                              int numArgs,
                                              const char** argNames,
                                              const uint8_t* argTypes,
                                              const uint64_t* argValues,
                                              uint8_t flags) {


    perfetto::DynamicCategory category {getCategoryGroupName(categoryEnabledFlag)};
    if (TRACE_EVENT_PHASE_COMPLETE == phase ||
        TRACE_EVENT_PHASE_INSTANT == phase) {

      switch (numArgs) {
        case 0: triggerTraceEvent(categoryEnabledFlag, name); break;
        case 1:
          triggerTraceEvent(categoryEnabledFlag, name, argNames[0],
            argTypes[0], argValues[0]);
          break;
        case 2:
          if (FLAGS_splitPerfettoTracesByBenchmark && strcmp(name, "Benchmark") == 0) {
            /* If the splitPerfettoTracesByBenchmark flag is set to true, then
             * upon receiving an event labeled as "benchmark" the prior
             * tracing session should be concluded and a new one started. */
            closeTracingSession();
            skia::tracing_internals::TraceValueUnion benchmarkName;
            benchmarkName.as_uint = argValues[0];
            openNewTracingSession(benchmarkName.as_string);
          }
          triggerTraceEvent(categoryEnabledFlag, name, argNames[0], argTypes[0],
            argValues[0], argNames[1], argTypes[1], argValues[1]);
          break;
          // TODO (Nicolette) throw error or at least print out issue when >2 args provided
        default: break;
      }
    } else if (TRACE_EVENT_PHASE_END == phase) {
        TRACE_EVENT_END(category);
    }

    if (TRACE_EVENT_PHASE_INSTANT == phase) {
        TRACE_EVENT_END(category);
    }
    return 0;
}

void SkPerfTrace::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                        const char* name,
                                        SkEventTracer::Handle handle) {
    // This is only ever called from a scoped trace event so we will just end the PerfTrace section.
    perfetto::DynamicCategory category {getCategoryGroupName(categoryEnabledFlag)};
    TRACE_EVENT_END(category);
}

const uint8_t* SkPerfTrace::getCategoryGroupEnabled(const char* name) {
    return fCategories.getCategoryGroupEnabled(name);
}

const char* SkPerfTrace::getCategoryGroupName(const uint8_t* categoryEnabledFlag) {
    return fCategories.getCategoryGroupName(categoryEnabledFlag);
}

void SkPerfTrace::closeTracingSession() {
    perfetto::TrackEvent::Flush();
    tracingSession->StopBlocking();
    close(fd);
}

void SkPerfTrace::openNewTracingSession(const std::string& baseFileName) {
    perfetto::TracingInitArgs args;

    // The backend(s) determine where trace events are recorded.
    // 1) The in-process backend only records within the app itself.
    args.backends |= perfetto::kInProcessBackend;
    // 2) The system backend writes events into a system Perfetto daemon,
    //    Requires the Perfetto `traced` daemon to be running (e.g.,
    //    on Android Pie and newer).
    // args.backends |= perfetto::kSystemBackend;

    /* Set the shared memory buffer size higher than the default of 256 KB to
    reduce trace writer packet loss occurrences associated with larger traces. */
    args.shmem_size_hint_kb = FLAGS_perfettoSMBSize;

    perfetto::Tracing::Initialize(args);
    perfetto::TrackEvent::Register();

    // Set up event tracing configuration.
    perfetto::protos::gen::TrackEventConfig track_event_cfg;
    perfetto::TraceConfig cfg;
    /* Set the central memory buffer size higher than the default to
    reduce data overridfe occurrences associated with larger traces. */
    // cfg.add_buffers()->set_size_kb(64000); // Record up to 64 MB.
    cfg.add_buffers()->set_size_kb(FLAGS_perfettoCMBSize);
    /* Enable continuous file writing/ "streaming mode" to output trace data
    throughout the program instead of one large dump at the end. */
    cfg.set_write_into_file(true);
    // If set to a value other than the default, set how often trace data gets written to the output file.
    if (FLAGS_perfettoFileWritePeriod != 5000) {
      cfg.set_file_write_period_ms(FLAGS_perfettoFileWritePeriod);
    }
    /* Force periodic commitment of shared memory buffer pages to the central buffer.
    Helps prevent out-of-order event slices with long traces. */
    cfg.set_flush_period_ms(FLAGS_perfettoFlushPeriod);

    auto* ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event");
    ds_cfg->set_track_event_config_raw(track_event_cfg.SerializeAsString());

    // Begin a tracing session.
    tracingSession = perfetto::Tracing::NewTrace();
    std::string fullFilePath = outputPath + baseFileName + outputFileSuffix;
    fd = open(fullFilePath.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
    tracingSession->Setup(cfg, fd);
    tracingSession->StartBlocking();
}

void SkPerfTrace::triggerTraceEvent(const uint8_t* categoryEnabledFlag,
                                    const char* eventName) {
    perfetto::DynamicCategory category {getCategoryGroupName(categoryEnabledFlag)};
    TRACE_EVENT_BEGIN(category, nullptr, [&](perfetto::EventContext ctx) {
      ctx.event()->set_name(eventName);
    });
}

void SkPerfTrace::triggerTraceEvent(const uint8_t* categoryEnabledFlag, const char* eventName,
                      const char* arg1Name, const uint8_t& arg1Type,
                      const uint64_t& arg1Val) {
    perfetto::DynamicCategory category {getCategoryGroupName(categoryEnabledFlag)};
    skia::tracing_internals::TraceValueUnion value;
    value.as_uint = arg1Val;

    switch (arg1Type) {
      case TRACE_VALUE_TYPE_BOOL:
        TRACE_EVENT_BEGIN(category, nullptr, arg1Name, value.as_bool, [&](perfetto::EventContext ctx) {
          ctx.event()->set_name(eventName);
        });
      break;
      case TRACE_VALUE_TYPE_UINT:
        TRACE_EVENT_BEGIN(category, nullptr, arg1Name, value.as_uint, [&](perfetto::EventContext ctx) {
          ctx.event()->set_name(eventName);
        });
        break;
      case TRACE_VALUE_TYPE_INT:
        TRACE_EVENT_BEGIN(category, nullptr, arg1Name, value.as_int, [&](perfetto::EventContext ctx) {
          ctx.event()->set_name(eventName);
        });
        break;
      case TRACE_VALUE_TYPE_DOUBLE:
        TRACE_EVENT_BEGIN(category, nullptr, arg1Name, value.as_double, [&](perfetto::EventContext ctx) {
          ctx.event()->set_name(eventName);
        });
        break;
      case TRACE_VALUE_TYPE_POINTER:
        TRACE_EVENT_BEGIN(category, nullptr, arg1Name, value.as_pointer, [&](perfetto::EventContext ctx) {
          ctx.event()->set_name(eventName);
        });
        break;
      case TRACE_VALUE_TYPE_STRING:
        TRACE_EVENT_BEGIN(category, nullptr, arg1Name, value.as_string, [&](perfetto::EventContext ctx) {
          ctx.event()->set_name(eventName);
        });
        break;
      case TRACE_VALUE_TYPE_COPY_STRING:
        TRACE_EVENT_BEGIN(category, nullptr, arg1Name, value.as_string, [&](perfetto::EventContext ctx) {
          ctx.event()->set_name(eventName);
        });
        break;
      default:
        TRACE_EVENT_BEGIN(category, nullptr, arg1Name, value.as_uint, [&](perfetto::EventContext ctx) {
          ctx.event()->set_name(eventName);
        });
        break;
    }
}

void SkPerfTrace::triggerTraceEvent(const uint8_t* categoryEnabledFlag,
                      const char* eventName, const char* arg1Name, const uint8_t& arg1Type,
                      const uint64_t& arg1Val, const char* arg2Name,
                      const uint8_t& arg2Type, const uint64_t& arg2Val) {

    perfetto::DynamicCategory category {getCategoryGroupName(categoryEnabledFlag)};
    skia::tracing_internals::TraceValueUnion value;
    value.as_uint = arg1Val;

    switch (arg1Type) {
      case TRACE_VALUE_TYPE_BOOL:
        beginEventWithSecondArg(category, eventName, arg1Name, value.as_bool, arg2Name, arg2Type, arg2Val);
        break;
      case TRACE_VALUE_TYPE_UINT:
        beginEventWithSecondArg(category, eventName, arg1Name, value.as_uint, arg2Name, arg2Type, arg2Val);
        break;
      case TRACE_VALUE_TYPE_INT:
        beginEventWithSecondArg(category, eventName, arg1Name, value.as_int, arg2Name, arg2Type, arg2Val);
        break;
      case TRACE_VALUE_TYPE_DOUBLE:
        beginEventWithSecondArg(category, eventName, arg1Name, value.as_double, arg2Name, arg2Type, arg2Val);
        break;
      case TRACE_VALUE_TYPE_POINTER:
        beginEventWithSecondArg(category, eventName, arg1Name, value.as_pointer, arg2Name, arg2Type, arg2Val);
        break;
      case TRACE_VALUE_TYPE_STRING:
        beginEventWithSecondArg(category, eventName, arg1Name, value.as_string, arg2Name, arg2Type, arg2Val);
        break;
      case TRACE_VALUE_TYPE_COPY_STRING:
        beginEventWithSecondArg(category, eventName, arg1Name, value.as_string, arg2Name, arg2Type, arg2Val);
        break;
      default:
        beginEventWithSecondArg(category, eventName, arg1Name, value.as_uint, arg2Name, arg2Type, arg2Val);
        break;
    }
}
