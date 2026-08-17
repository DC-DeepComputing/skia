/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/trace/EventTracingPriv.h"

#if defined(SK_USE_PERFETTO)
#include <perfetto/tracing.h>
#include <fcntl.h>
#include <fstream>
#include "tools/flags/CommandLineFlags.h"
#include <cstdlib>

static DEFINE_string(perfettoOutputDir, "./",
                     "Output directory for perfetto trace file(s).\n"
                     "Note: not the name of the file itself.\n"
                     "Will only have an effect if perfetto tracing is enabled. See --trace.");
static DEFINE_string(perfettoOutputFileName, "trace",
                     "Output file name (excluding path and file extension) for the perfetto trace"
                     "file.\nNote: When splitting trace files by benchmark (see "
                     "--splitPerfettoTracesByBenchmark), file name will be determined by the "
                     "benchmark name.\n"
                     "Will only have an effect if perfetto tracing is enabled. See --trace.");
static DEFINE_string(perfettoOutputFileExtension, ".perfetto-trace",
                     "Output file extension for perfetto trace file(s).\n"
                     "Will only have an effect if perfetto tracing is enabled. See --trace.");
static DEFINE_bool(longPerfettoTrace, false,
                   "Perfetto within Skia is optimized for tracing performance of 'smaller' traces"
                   "(~10 seconds or less). Set this flag to true to optimize for longer tracing"
                   "sessions.\n"
                   "Will only have an effect if perfetto tracing is enabled. See --trace.");

class SkiaPerfettoSession {
public:
    static SkiaPerfettoSession& GetInstance() {
        static SkiaPerfettoSession instance;
        return instance;
    }

    void start(const char* baseFileName) {
        if (fSession) {
            return; // Already running
        }

        fCurrentSessionFullOutputPath = FLAGS_perfettoOutputDir[0] +
                                        std::string(baseFileName) +
                                        FLAGS_perfettoOutputFileExtension[0];

        perfetto::TraceConfig cfg;
        cfg.add_buffers()->set_size_kb(32000);

        if (FLAGS_longPerfettoTrace) {
            cfg.set_write_into_file(true);
            cfg.set_file_write_period_ms(5000);
            cfg.set_flush_period_ms(10000);
        }

        auto* ds_cfg = cfg.add_data_sources()->mutable_config();
        ds_cfg->set_name("track_event");

        fSession = perfetto::Tracing::NewTrace();
        if (FLAGS_longPerfettoTrace) {
            fFd = open(fCurrentSessionFullOutputPath.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
            fSession->Setup(cfg, fFd);
        } else {
            fSession->Setup(cfg);
        }
        fSession->StartBlocking();
    }

    void stop() {
        if (!fSession) {
            return;
        }

        perfetto::TrackEvent::Flush();
        fSession->StopBlocking();

        if (!FLAGS_longPerfettoTrace) {
            std::vector<char> trace_data(fSession->ReadTraceBlocking());
            std::ofstream output;
            output.open(fCurrentSessionFullOutputPath, std::ios::out | std::ios::binary);
            output.write(&trace_data[0], trace_data.size());
            output.close();
        } else {
            if (fFd != -1) {
                close(fFd);
                fFd = -1;
            }
        }
        fSession.reset();
    }

private:
    SkiaPerfettoSession() = default;
    ~SkiaPerfettoSession() { this->stop(); }

    std::unique_ptr<perfetto::TracingSession> fSession;
    std::string fCurrentSessionFullOutputPath;
    int fFd = -1;
};

static bool gPerfettoInitialized = false;
static void initPerfetto() {
    if (gPerfettoInitialized) {
        return;
    }
    perfetto::TracingInitArgs args;
    args.backends |= perfetto::kInProcessBackend;
    if (FLAGS_longPerfettoTrace) {
        args.shmem_size_hint_kb = 2000;
    }
    perfetto::Tracing::Initialize(args);
    perfetto::TrackEvent::Register();
    gPerfettoInitialized = true;
}

#endif // SK_USE_PERFETTO

static DEFINE_string(trace,
              "",
              "Log trace events in one of several modes:\n"
              "  perfetto   : Send events to Perfetto (Linux, Android, and Mac only)\n"
              "  <filename> : Any other string is interpreted as a filename. Writes\n"
              "               trace events to specified file as Perfetto proto");

void initializeEventTracingForTools(const char* traceFlag) {
#if defined(SK_USE_PERFETTO)
    if (!traceFlag) {
        if (FLAGS_trace.isEmpty()) {
            return;
        }
        traceFlag = FLAGS_trace[0];
    }

    static bool atexitRegistered = false;
    if (0 == strcmp(traceFlag, "perfetto")) {
        initPerfetto();
        SkiaPerfettoSession::GetInstance().start(FLAGS_perfettoOutputFileName[0]);
        if (!atexitRegistered) {
            std::atexit(terminateEventTracingForTools);
            atexitRegistered = true;
        }
    } else if (traceFlag && strlen(traceFlag) > 0) {
        // interpret as filename
        initPerfetto();
        SkiaPerfettoSession::GetInstance().start(traceFlag);
        if (!atexitRegistered) {
            std::atexit(terminateEventTracingForTools);
            atexitRegistered = true;
        }
    }
#endif
}

void terminateEventTracingForTools() {
#if defined(SK_USE_PERFETTO)
    SkiaPerfettoSession::GetInstance().stop();
#endif
}

void traceEventNewTracingSection(const char* name) {
#if defined(SK_USE_PERFETTO)
    SkiaPerfettoSession::GetInstance().stop();
    SkiaPerfettoSession::GetInstance().start(name);
#endif
}
