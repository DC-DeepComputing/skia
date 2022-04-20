/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPerfTrace.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkTraceEventCommon.h"
#include "perfetto/sdk/perfetto.h"

SkPerfTrace::SkPerfTrace() {


  //   if (!perfetto::Tracing::IsInitialized()) {
  //     perfetto::TracingInitArgs args;

  //     // The backends determine where trace events are recorded. You may select one
  //     // or more of:

  //     // 1) The in-process backend only records within the app itself.
  //     args.backends |= perfetto::kInProcessBackend;

  //     // 2) The system backend writes events into a system Perfetto daemon,
  //     //    allowing merging app and system events (e.g., ftrace) on the same
  //     //    timeline. Requires the Perfetto `traced` daemon to be running (e.g.,
  //     //    on Android Pie and newer).
  //     // args.backends |= perfetto::kSystemBackend;

  //     perfetto::Tracing::Initialize(args);
  // }
  // fIsEnabled = []{ return true; };//static_cast<bool>( perfetto::Tracing::IsInitialized()); };

  //   if (!fIsEnabled) {
  //       fIsEnabled = []{ return false; };
  //   }
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
    // if (!perfetto::Tracing::IsInitialized()) {
    //   perfetto::TracingInitArgs args;

      // The backends determine where trace events are recorded. You may select one
      // or more of:
      // 1) The in-process backend only records within the app itself.
      // args.backends |= perfetto::kInProcessBackend;

      // 2) The system backend writes events into a system Perfetto daemon,
      //    allowing merging app and system events (e.g., ftrace) on the same
      //    timeline. Requires the Perfetto `traced` daemon to be running (e.g.,
      //    on Android Pie and newer).
      // args.backends |= perfetto::kSystemBackend;

      // perfetto::Tracing::Initialize(args);
  // }
    bool test = perfetto::Tracing::IsInitialized();
    return 0;
}

void SkPerfTrace::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                        const char* name,
                                        SkEventTracer::Handle handle) {
    // This is only ever called from a scoped trace event so we will just end the PerfTrace section.
}

const uint8_t* SkPerfTrace::getCategoryGroupEnabled(const char* name) {
    // Chrome tracing is setup to not repeatly call this function once it has been initialized. So
    // we can't use this to do a check for PerfTrace isEnabled(). Thus we will always return yes here
    // and then check to see if PerfTrace is enabled when beginning and ending a section.
    static uint8_t yes = SkEventTracer::kEnabledForRecording_CategoryGroupEnabledFlags;
    return &yes;
}



