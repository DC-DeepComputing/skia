/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleBinding_DEFINED
#define SkParticleBinding_DEFINED

#include "include/core/SkString.h"
#include "modules/particles/include/SkReflected.h"
#include "src/sksl/ir/SkSLExternalFunction.h"

#include <memory>

class SkArenaAlloc;
class SkParticleEffect;
class SkParticleEffectParams;

namespace skresources {
    class ResourceProvider;
}  // namespace skresources

namespace SkSL {
    class Compiler;
}  // namespace SkSL

namespace skvm {
    struct Uniforms;
}  // namespace skvm

class SkParticleExternalFunction : public SkSL::ExternalFunction {
public:
    SkParticleExternalFunction(const char* name,
                               SkSL::Compiler& compiler,
                               const SkSL::Type& type,
                               skvm::Uniforms* uniforms,
                               SkArenaAlloc* alloc)
            : SkSL::ExternalFunction(name, type)
            , fCompiler(compiler)
            , fUniforms(uniforms)
            , fAlloc(alloc) {}

protected:
    SkSL::Compiler& fCompiler;
    skvm::Uniforms* fUniforms;
    SkArenaAlloc*   fAlloc;
};

class SkParticleBinding : public SkReflected {
public:
    SkParticleBinding(const char* name = "name") : fName(name) {}

    REFLECTED_ABSTRACT(SkParticleBinding, SkReflected)

    void visitFields(SkFieldVisitor* v) override;

    /*
     * All SkParticleBinding objects expose a particular native object to an effect's SkSL code.
     * In all cases, the 'name' is the symbol that will be used to access the object from the SkSL.
     * Each binding is a callable object, so the SkSL name behaves like a function.
     */
    virtual std::unique_ptr<SkParticleExternalFunction> toFunction(SkSL::Compiler&,
                                                                   skvm::Uniforms*,
                                                                   SkArenaAlloc*) = 0;
    virtual void prepare(const skresources::ResourceProvider*) = 0;
    virtual void update(double t) {}
    static void RegisterBindingTypes();

protected:
    SkString fName;
};

#endif // SkParticleBinding_DEFINED
