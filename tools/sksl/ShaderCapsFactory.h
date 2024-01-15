/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TOOLS_SKSL_SHADERCAPSFACTORY
#define TOOLS_SKSL_SHADERCAPSFACTORY

#include "src/sksl/SkSLUtil.h"

#include <memory>

namespace SkSL::tools {

// Various sets of caps for use in tests
class ShaderCapsFactory {
public:
    static const ShaderCaps* Default() {
        static const SkSL::ShaderCaps* sCaps = [] {
            std::unique_ptr<ShaderCaps> caps = MakeShaderCaps();
            caps->fVersionDeclString = "#version 400";
            caps->fShaderDerivativeSupport = true;
            return caps.release();
        }();
        return sCaps;
    }

    static const ShaderCaps* Standalone() {
        static const SkSL::ShaderCaps* sCaps = MakeShaderCaps().release();
        return sCaps;
    }

protected:
    static std::unique_ptr<ShaderCaps> MakeShaderCaps();
};

}  // namespace SkSL::tools

#endif
