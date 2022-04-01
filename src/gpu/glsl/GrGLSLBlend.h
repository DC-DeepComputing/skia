/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLBlend_DEFINED
#define GrGLBlend_DEFINED

#include "include/core/SkBlendMode.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"

class GrGLSLShaderBuilder;
class GrGLSLUniformHandler;
class GrProcessor;

namespace GrGLSLBlend {

/**
 * Returns the name of the built in blend function for a SkBlendMode.
 */
const char* BlendFuncName(SkBlendMode mode);

/**
 * Appends GLSL code to fsBuilder that assigns a specified blend of the srcColor and dstColor
 * variables to the outColor variable.
 */
void AppendMode(GrGLSLShaderBuilder* fsBuilder,
                const char* srcColor,
                const char* dstColor,
                const char* outColor,
                SkBlendMode mode);

/**
 * Returns an SkSL expression that blends the passed-in srcColor and dstColor values.
 * A matching call to SetBlendModeUniform must be made from onSetData.
 */
std::string BlendExpression(const GrProcessor* processor,
                            GrGLSLUniformHandler* uniformHandler,
                            GrGLSLProgramDataManager::UniformHandle* uniform,
                            const char* srcColor,
                            const char* dstColor,
                            SkBlendMode mode);

/**
 * Sets up uniforms for a BlendExpression as needed. Pass the same uniform and blend mode that were
 * passed to BlendExpression.
 */
void SetBlendModeUniform(const GrGLSLProgramDataManager& pdman,
                         GrGLSLProgramDataManager::UniformHandle uniform,
                         SkBlendMode mode);

}  // namespace GrGLSLBlend

#endif
