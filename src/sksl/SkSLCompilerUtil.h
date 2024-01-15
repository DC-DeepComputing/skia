/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkAssert.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <cstdint>
#include <memory>
#include <string_view>

namespace SkSL {

class AutoShaderCaps {
public:
    AutoShaderCaps(std::shared_ptr<Context>& context, const ShaderCaps* caps)
            : fContext(context.get()), fOldCaps(fContext->fCaps) {
        fContext->fCaps = caps;
    }

    ~AutoShaderCaps() { fContext->fCaps = fOldCaps; }

    Context* fContext;
    const ShaderCaps* fOldCaps;
};

class AutoSource {
public:
    AutoSource(Compiler* compiler, std::string_view source) : fCompiler(compiler) {
        SkASSERT(!fCompiler->errorReporter().source().data());
        fCompiler->errorReporter().setSource(source);
    }

    ~AutoSource() { fCompiler->errorReporter().setSource(std::string_view()); }

    Compiler* fCompiler;
};

}  // namespace SkSL
