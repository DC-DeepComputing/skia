/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/partition_alloc/raw_ref.h"

#ifndef SKSL_CONTEXT

#define SKSL_CONTEXT

#include "src/partition_alloc/raw_ptr.h"

#include "include/private/SkAssert.h"

namespace SkSL {

class BuiltinTypes;
class ErrorReporter;
struct Module;
struct ProgramConfig;
class SymbolTable;

/**
 * Contains compiler-wide objects and state.
 */
class Context {
public:
    Context(const BuiltinTypes& types, ErrorReporter& errors);
    ~Context();

    // The Context holds a reference to all of the built-in types.
    const bisect_raw_ref<const BuiltinTypes> fTypes;

    // The Context holds a pointer to the configuration of the program being compiled.
    bisect_raw_ptr<ProgramConfig, DanglingUntriaged> fConfig = nullptr;

    // The Context holds a pointer to our error reporter.
    bisect_raw_ptr<ErrorReporter> fErrors;

    void setErrorReporter(ErrorReporter* e) {
        SkASSERT(e);
        fErrors = e;
    }

    // The Context holds a pointer to our module with built-in declarations.
    bisect_raw_ptr<const Module> fModule = nullptr;

    // This is the current symbol table of the code we are processing, and therefore changes during
    // compilation.
    bisect_raw_ptr<SymbolTable> fSymbolTable = nullptr;
};

}  // namespace SkSL

#endif
