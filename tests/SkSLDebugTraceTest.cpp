/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLDebugTrace.h"
#include "tests/Test.h"

#include <string>
#include <vector>

class SkWStream;

namespace {

class ConcreteDebugTrace : public SkSL::DebugTrace {
public:
    void writeTrace(SkWStream* w) const override {}
    void dump(SkWStream* o) const override {}
};

}  // namespace

DEF_TEST(SkSLDebugTraceSetSource, r) {
    ConcreteDebugTrace i;
    i.setSource("SkSL::DebugTrace::setSource unit test\n"
                "\t// first line\n"
                "\t// second line\n"
                "\t// third line");

    REPORTER_ASSERT(r, i.fSource.size() == 4);
    REPORTER_ASSERT(r, i.fSource[0] == "SkSL::DebugTrace::setSource unit test");
    REPORTER_ASSERT(r, i.fSource[1] == "\t// first line");
    REPORTER_ASSERT(r, i.fSource[2] == "\t// second line");
    REPORTER_ASSERT(r, i.fSource[3] == "\t// third line");
}

DEF_TEST(SkSLDebugTraceSetSourceReplacesExistingText, r) {
    ConcreteDebugTrace i;
    i.setSource("One");
    i.setSource("Two");
    i.setSource("Three");

    REPORTER_ASSERT(r, i.fSource.size() == 1);
    REPORTER_ASSERT(r, i.fSource[0] == "Three");
}
