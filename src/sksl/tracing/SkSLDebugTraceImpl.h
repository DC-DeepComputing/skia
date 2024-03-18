/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSLDEBUGTRACEIMPL
#define SKSLDEBUGTRACEIMPL

#include "include/core/SkRefCnt.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#include <cstdint>
#include <string>

class SkStream;
class SkWStream;
struct SkIPoint;

namespace SkSL {

class DebugTraceImpl : public DebugTracePriv {
public:
    /**
     * Sets the device-coordinate pixel to trace. If it's not set, the point at (0, 0) will be used.
     */
    void setTraceCoord(const SkIPoint& coord) override;

    /** Attaches the SkSL source to be debugged. */
    void setSource(const std::string& source) override;

    /** Serializes a debug trace to JSON which can be parsed by our debugger. */
    bool readTrace(SkStream* r) override;
    void writeTrace(SkWStream* w) const override;

    /** Generates a human-readable dump of the debug trace. */
    void dump(SkWStream* o) const override;

    /** Returns a slot's component as a variable-name suffix, e.g. ".x" or "[2][2]". */
    std::string getSlotComponentSuffix(int slotIndex) const override;

    /** Bit-casts a slot's value, then converts to text, e.g. "3.14" or "true" or "12345". */
    std::string getSlotValue(int slotIndex, int32_t value) const override;

    /** Bit-casts a value for a given slot into a double, honoring the slot's NumberKind. */
    double interpretValueBits(int slotIndex, int32_t valueBits) const override;

    /** Converts a numeric value into text, based on the slot's NumberKind. */
    std::string slotValueToString(int slotIndex, double value) const override;

    static sk_sp<DebugTracePriv> Make();
};

}  // namespace SkSL

#endif
