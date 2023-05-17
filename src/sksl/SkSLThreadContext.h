/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_THREADCONTEXT
#define SKSL_THREADCONTEXT

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace SkSL {

class Compiler;
class Context;
class ModifiersPool;
class Pool;
class Position;
class ProgramElement;
class Variable;
enum class ProgramKind : int8_t;
struct Module;

/**
 * Thread-safe class that tracks per-thread state associated with SkSL output.
 */
class ThreadContext {
public:
    ThreadContext(SkSL::Compiler* compiler,
                  SkSL::ProgramKind kind,
                  const SkSL::ProgramSettings& settings,
                  const SkSL::Module* module,
                  bool isModule);
    ~ThreadContext();

    /**
     * Returns the Compiler used by DSL operations in the current thread.
     */
    static SkSL::Compiler& Compiler() { return *Instance().fCompiler; }

    /**
     * Returns the Context used by DSL operations in the current thread.
     */
    static SkSL::Context& Context();

    /**
     * Returns the collection to which DSL program elements in this thread should be appended.
     */
    static std::vector<std::unique_ptr<SkSL::ProgramElement>>& ProgramElements() {
        return Instance().fProgramElements;
    }

    static std::vector<const ProgramElement*>& SharedElements() {
        return Instance().fSharedElements;
    }

    /**
     * Returns the current ProgramConfig.
     */
    static bool IsModule() { return Instance().fConfig->fIsBuiltinCode; }

    struct RTAdjustData {
        // Points to a standalone sk_RTAdjust variable, if one exists.
        const Variable* fVar = nullptr;
        // Points to the interface block containing an sk_RTAdjust field, if one exists.
        const Variable* fInterfaceBlock = nullptr;
        // If fInterfaceBlock is non-null, contains the index of the sk_RTAdjust field within it.
        int fFieldIndex = -1;
    };

    /**
     * Returns a struct containing information about the RTAdjust variable.
     */
    static RTAdjustData& RTAdjustState();

    static ThreadContext& Instance();

    static void SetInstance(std::unique_ptr<ThreadContext> instance);

private:
    class DefaultErrorReporter : public ErrorReporter {
        void handleError(std::string_view msg, Position pos) override;
    };

    void setupSymbolTable();

    std::unique_ptr<SkSL::ProgramConfig> fConfig;
    std::unique_ptr<SkSL::ModifiersPool> fModifiersPool;
    SkSL::Compiler* fCompiler;
    std::unique_ptr<Pool> fPool;
    SkSL::ProgramConfig* fOldConfig;
    SkSL::ModifiersPool* fOldModifiersPool;
    std::vector<std::unique_ptr<SkSL::ProgramElement>> fProgramElements;
    std::vector<const SkSL::ProgramElement*> fSharedElements;
    DefaultErrorReporter fDefaultErrorReporter;
    ErrorReporter& fOldErrorReporter;
    ProgramSettings fSettings;
    RTAdjustData fRTAdjust;
    Program::Interface fInterface;

    friend class Compiler;
};

} // namespace SkSL

#endif
