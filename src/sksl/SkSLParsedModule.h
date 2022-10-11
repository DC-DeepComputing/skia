/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PARSEDMODULE
#define SKSL_PARSEDMODULE

#include "src/sksl/SkSLBuiltinMap.h"
#include <memory>

namespace SkSL {

class SymbolTable;

struct ParsedModule {
    const std::shared_ptr<SymbolTable> symbols() const { return fBuiltins->symbolTable(); }
    std::shared_ptr<const BuiltinMap> fBuiltins;
};

} //namespace SkSL

#endif
