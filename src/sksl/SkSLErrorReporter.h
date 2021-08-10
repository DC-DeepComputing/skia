/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ERRORREPORTER
#define SKSL_ERRORREPORTER

#include "include/sksl/DSLErrorHandling.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLPosition.h"

namespace SkSL {

/**
 * Error reporter for tests that need an SkSL context; aborts immediately if an error is reported.
 */
class TestingOnly_AbortErrorHandler : public dsl::ErrorHandler {
public:
    void handleError(const char* msg, dsl::PositionInfo pos) override { SK_ABORT("%s", msg); }
};

}  // namespace SkSL

#endif
