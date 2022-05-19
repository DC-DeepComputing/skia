/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPerfTrace_DEFINED
#define SkPerfTrace_DEFINED

#include "include/utils/SkEventTracer.h"
#include "tools/trace/EventTracingPriv.h"
#include "third_party/externals/perfetto/sdk/perfetto.h"

PERFETTO_DEFINE_CATEGORIES();

/**
 * This class is used to support Perfetto tracing. It hooks into the SkEventTracer system.
 */
class SkPerfTrace : public SkEventTracer {
public:
    SkPerfTrace();
    ~SkPerfTrace() override;

    SkEventTracer::Handle addTraceEvent(char phase,
                                        const uint8_t* categoryEnabledFlag,
                                        const char* name,
                                        uint64_t id,
                                        int numArgs,
                                        const char** argNames,
                                        const uint8_t* argTypes,
                                        const uint64_t* argValues,
                                        uint8_t flags) override;


    void updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                  const char* name,
                                  SkEventTracer::Handle handle) override;

    const uint8_t* getCategoryGroupEnabled(const char* name) override;

    const char* getCategoryGroupName(const uint8_t* categoryEnabledFlag) override;

private:
    SkPerfTrace(const SkPerfTrace&) = delete;
    SkPerfTrace& operator=(const SkPerfTrace&) = delete;
    SkEventTracingCategories fCategories;
    std::unique_ptr<perfetto::TracingSession> tracingSession;
    int fd {-1};

    /* One can optionally elect to have different trace output files created for each
    * testing benchmark. So while the output file path and suffix are constant, the file name itself
    * is mutable. Initialized to a default file name. */
    std::string outputFileName {"trace"};
    const std::string outputPath {"third_party/externals/perfetto/"};
    const std::string outputFileSuffix {".perfetto-trace"};

    void closeTracingSession();
    void openNewTracingSession(const std::string& baseFileName);

    /* Overloaded private methods to initiate a trace event with 0-2 arguments.
     * Although perfetto can support an arbitrary number of debug annotations,
     * the existing trace structure supports 0-2, so that is all that is supported here.
     */
    void triggerTraceEvent(const uint8_t* categoryEnabledFlag,
                          const char* eventName);
    void triggerTraceEvent(const uint8_t* categoryEnabledFlag, const char* eventName,
                          const char* arg1Name, const uint8_t& arg1Type,
                          const uint64_t& arg1Val);
    void triggerTraceEvent(const uint8_t* categoryEnabledFlag,
                          const char* eventName, const char* arg1Name, const uint8_t& arg1Type,
                          const uint64_t& arg1Val, const char* arg2Name,
                          const uint8_t& arg2Type, const uint64_t& arg2Val);
};

#endif
