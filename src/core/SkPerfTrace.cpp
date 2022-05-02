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

SkPerfTrace::SkPerfTrace() {
    if (!perfetto::Tracing::IsInitialized()) {
      perfetto::TracingInitArgs args;

      // The backends determine where trace events are recorded. You may select one
      // or more of:
      // 1) The in-process backend only records within the app itself.
      args.backends |= perfetto::kInProcessBackend;
      // 2) The system backend writes events into a system Perfetto daemon,
      //    allowing merging app and system events (e.g., ftrace) on the same
      //    timeline. Requires the Perfetto `traced` daemon to be running (e.g.,
      //    on Android Pie and newer).
      // args.backends |= perfetto::kSystemBackend;

      /* Set the shared memory buffer size higher than the default to
      reduce trace writer packet loss occurrences associated with larger traces. */
      args.shmem_size_hint_kb = 32000;

      perfetto::Tracing::Initialize(args);
      perfetto::TrackEvent::Register();

      // Set up event tracing configuration.
      perfetto::protos::gen::TrackEventConfig track_event_cfg;
      perfetto::TraceConfig cfg;
      /* Set the central memory buffer size higher than the default to
      reduce data overridfe occurrences associated with larger traces. */
      cfg.add_buffers()->set_size_kb(64000); // Record up to 64 MB.
      /* Enable continuous file writing/ "streaming mode" to output trace data
      throughout the program instead of one large dump at the end. */
      cfg.set_write_into_file(true);
      // Set how often trace data gets written to the output file.
      cfg.set_file_write_period_ms(100);
      /* Force periodic commitment of shared memory buffer pages to the central buffer.
      Helps prevent out-of-order event slices with long traces. */
      cfg.set_flush_period_ms(10000);

      auto* ds_cfg = cfg.add_data_sources()->mutable_config();
      ds_cfg->set_name("track_event");
      ds_cfg->set_track_event_config_raw(track_event_cfg.SerializeAsString());

      // Begin a tracing session.
      tracingSession = perfetto::Tracing::NewTrace();
      fd = open(outputFilePath.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
      tracingSession->Setup(cfg, fd);
      tracingSession->StartBlocking();
    }
}

SkPerfTrace::~SkPerfTrace() {
    // Wrap up the tracing session.
    perfetto::TrackEvent::Flush();
    tracingSession->StopBlocking();
    close(fd);
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
    if (TRACE_EVENT_PHASE_COMPLETE == phase ||
        TRACE_EVENT_PHASE_INSTANT == phase) {
        TRACE_EVENT_BEGIN("test", nullptr, [&](perfetto::EventContext ctx) {
          ctx.event()->set_name(name);
        });
        eventsStarted++;
    } else if (TRACE_EVENT_PHASE_END == phase) {
        TRACE_EVENT_END("test");
        eventsTerminated++;
    }

    if (TRACE_EVENT_PHASE_INSTANT == phase) {
        TRACE_EVENT_END("test");
        eventsTerminated++;
    }
    return 0;
}

void SkPerfTrace::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                        const char* name,
                                        SkEventTracer::Handle handle) {
    // This is only ever called from a scoped trace event so we will just end the PerfTrace section.
    TRACE_EVENT_END("test");
    eventsTerminated++;
}

const uint8_t* SkPerfTrace::getCategoryGroupEnabled(const char* name) {
    return fCategories.getCategoryGroupEnabled(name);
}

const char* SkPerfTrace::getCategoryGroupName(const uint8_t* categoryEnabledFlag) {
    return fCategories.getCategoryGroupName(categoryEnabledFlag);
}
