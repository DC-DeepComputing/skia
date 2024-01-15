/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sksl/ShaderCapsFactory.h"

#include "src/core/SkSLTypeShared.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/ir/SkSLType.h"

#include <string>

namespace SkSL::tools {

// TODO: Once Graphite has its own GPU-caps system, SK_GRAPHITE should get its own mode.
// At the moment, it either mimics what GrShaderCaps reports, or it uses these hard-coded values
// depending on the build.
#if defined(SKSL_STANDALONE) || !defined(SK_GANESH)
std::unique_ptr<ShaderCaps> ShaderCapsFactory::MakeShaderCaps() {
    std::unique_ptr<ShaderCaps> standalone = std::make_unique<ShaderCaps>();
    standalone->fShaderDerivativeSupport = true;
    standalone->fExplicitTextureLodSupport = true;
    standalone->fFlatInterpolationSupport = true;
    standalone->fNoPerspectiveInterpolationSupport = true;
    standalone->fSampleMaskSupport = true;
    standalone->fExternalTextureSupport = true;
    return standalone;
}
#else
std::unique_ptr<ShaderCaps> ShaderCapsFactory::MakeShaderCaps() {
    return std::make_unique<ShaderCaps>();
}
#endif  // defined(SKSL_STANDALONE) || !defined(SK_GANESH)

}  // namespace SkSL::tools
