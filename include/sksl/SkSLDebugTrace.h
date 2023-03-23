/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DEBUG_TRACE
#define SKSL_DEBUG_TRACE

#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"

#include <memory>
#include <string>
#include <vector>

class SkWStream;

namespace SkSL {

class TraceHook;

class DebugTrace : public SkRefCnt {
public:
    DebugTrace();
    ~DebugTrace() override;

    /**
     * Sets the device-coordinate pixel to trace. If it's not set, the point at (0, 0) will be used.
     */
    void setTraceCoord(const SkIPoint& coord);

    /** Attaches the SkSL source to be debugged. */
    void setSource(std::string source);

    /** Serializes a debug trace to JSON which can be parsed by our debugger. */
    virtual void writeTrace(SkWStream* w) const = 0;

    /** Generates a human-readable dump of the debug trace. */
    virtual void dump(SkWStream* o) const = 0;

    /** The device-coordinate pixel to trace (controlled by setTraceCoord) */
    SkIPoint fTraceCoord = {};

    /** The SkSL code, split line-by-line. */
    std::vector<std::string> fSource;

    /**
     * A trace hook populates the trace info during shader evaluation. This will be populated
     * automatically by the SkSL code generator.
     */
    std::unique_ptr<SkSL::TraceHook> fTraceHook;
};

} // namespace SkSL

#endif
