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

static DEFINE_bool(splitPerfettoTracesByBenchmark, true,
                   "Create separate perfetto trace files for each benchmark?\n"
                   "Will only take effect if perfetto tracing is enabled. See --trace.");

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

    // Skia's event tracing macros offer support to add up to 2 debug annontations/
    // arguments, so parse the arguments to pass them into perfetto.
    if (TRACE_EVENT_PHASE_COMPLETE == phase ||
        TRACE_EVENT_PHASE_INSTANT == phase) {
        if (numArgs == 0 ) {
          TRACE_EVENT_BEGIN(category, nullptr, [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(name);
          });
        } else if (numArgs == 1) {
          TRACE_EVENT_BEGIN(category, nullptr, argNames[0], argValues[0], [&](perfetto::EventContext ctx) {
            ctx.event()->set_name(name);
          });
        } else if (numArgs == 2) {
          if (FLAGS_splitPerfettoTracesByBenchmark && strcmp(name, "Benchmark") == 0) {
            /* If the splitPerfettoTracesByBenchmark flag is set to true, then
             * upon receiving an event labeled as "benchmark" the prior
             * tracing session should be concluded and a new one started. */
            closeTracingSession();
            skia::tracing_internals::TraceValueUnion benchmarkName;
            benchmarkName.as_uint = argValues[0];
            openNewTracingSession(benchmarkName.as_string);
          } else {
            TRACE_EVENT_BEGIN(category, nullptr, argNames[0], argValues[0], argNames[1], argValues[1], [&](perfetto::EventContext ctx) {
              ctx.event()->set_name(name);
            });
          }
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
      args.shmem_size_hint_kb = 2000;

      perfetto::Tracing::Initialize(args);
      perfetto::TrackEvent::Register();

      // Set up event tracing configuration.
      perfetto::protos::gen::TrackEventConfig track_event_cfg;
      perfetto::TraceConfig cfg;
      /* Set the central memory buffer size higher than the default to
      reduce data overridfe occurrences associated with larger traces. */
      // cfg.add_buffers()->set_size_kb(64000); // Record up to 64 MB.
      cfg.add_buffers()->set_size_kb(32000);
      /* Enable continuous file writing/ "streaming mode" to output trace data
      throughout the program instead of one large dump at the end. */
      cfg.set_write_into_file(true);
      // Set how often trace data gets written to the output file.
      //cfg.set_file_write_period_ms(100);
      /* Force periodic commitment of shared memory buffer pages to the central buffer.
      Helps prevent out-of-order event slices with long traces. */
      cfg.set_flush_period_ms(10000);

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
