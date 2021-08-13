/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineStateBuilder_DEFINED
#define GrMtlPipelineStateBuilder_DEFINED

#include "src/gpu/GrPipeline.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/mtl/GrMtlUniformHandler.h"
#include "src/gpu/mtl/GrMtlVaryingHandler.h"
#include "src/sksl/SkSLCompiler.h"

#import <Metal/Metal.h>

#include "include/ports/SkCFObject.h"

class GrProgramDesc;
class GrProgramInfo;
class GrMtlCaps;
class GrMtlGpu;
class GrMtlPipelineState;
class SkReadBuffer;

struct GrMtlPrecompiledLibraries {
    sk_cfp<id<MTLLibrary>> fVertexLibrary;
    sk_cfp<id<MTLLibrary>> fFragmentLibrary;
    bool fRTFlip = false;
};

class GrMtlPipelineStateBuilder : public GrGLSLProgramBuilder {
public:
    /** Generates a pipeline state.
     *
     * The returned GrMtlPipelineState implements the supplied GrProgramInfo.
     *
     * @return the created pipeline if generation was successful; nullptr otherwise
     */
    static GrMtlPipelineState* CreatePipelineState(
                                       GrMtlGpu*,
                                       const GrProgramDesc&,
                                       const GrProgramInfo&,
                                       const GrMtlPrecompiledLibraries* precompiledLibs = nullptr);

    static bool PrecompileShaders(GrMtlGpu*, const SkData&,
                                  GrMtlPrecompiledLibraries* precompiledLibs);

private:
    GrMtlPipelineStateBuilder(GrMtlGpu*, const GrProgramDesc&, const GrProgramInfo&);

    GrMtlPipelineState* finalize(const GrProgramDesc&, const GrProgramInfo&,
                                 const GrMtlPrecompiledLibraries* precompiledLibraries);

    const GrCaps* caps() const override;

    SkSL::Compiler* shaderCompiler() const override;

    void finalizeFragmentOutputColor(GrShaderVar& outputColor) override;

    void finalizeFragmentSecondaryColor(GrShaderVar& outputColor) override;

    sk_cfp<id<MTLLibrary>> compileMtlShaderLibrary(const SkSL::String& shader,
                                                   SkSL::Program::Inputs,
                                                   GrContextOptions::ShaderErrorHandler*);
    void storeShadersInCache(const SkSL::String shaders[], const SkSL::Program::Inputs inputs[],
                             SkSL::Program::Settings*, sk_sp<SkData>, bool isSkSL);

    GrGLSLUniformHandler* uniformHandler() override { return &fUniformHandler; }
    const GrGLSLUniformHandler* uniformHandler() const override { return &fUniformHandler; }
    GrGLSLVaryingHandler* varyingHandler() override { return &fVaryingHandler; }

    GrMtlGpu* fGpu;
    GrMtlUniformHandler fUniformHandler;
    GrMtlVaryingHandler fVaryingHandler;

    using INHERITED = GrGLSLProgramBuilder;
};
#endif
