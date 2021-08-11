/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTEXT
#define SKSL_CONTEXT

#include <memory>

#include "include/sksl/DSLErrorHandling.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

struct ProgramConfig;

namespace dsl {

class ErrorHandler;

} // namespace dsl

/**
 * Contains compiler-wide objects, which currently means the core types.
 */
class Context {
public:
    Context(dsl::ErrorHandler& errors, const ShaderCapsClass& caps);

    ~Context() {
        SkASSERT(!Pool::IsAttached());
    }

    // Returns the current error handler
    dsl::ErrorHandler& errors() const;

    // The Context holds all of the built-in types.
    BuiltinTypes fTypes;

    // The Context holds a reference to our shader caps bits.
    const ShaderCapsClass& fCaps;

    // The Context holds a pointer to our pool of modifiers.
    ModifiersPool* fModifiersPool = nullptr;

    // The Context holds a pointer to the configuration of the program being compiled.
    ProgramConfig* fConfig = nullptr;

private:
    // The default error handler to use outside of DSL code (between Start() and End(), the DSL
    // error handler is used instead)
    dsl::ErrorHandler& fErrors;
};

}  // namespace SkSL

#endif
