/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkM44.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/codegen/SkSLVMCodeGenerator.h"
#include "src/sksl/codegen/SkVMDebugTrace.h"
#include "src/sksl/tracing/SkVMDebugTracePlayer.h"

#include "tests/Test.h"

static sk_sp<SkSL::SkVMDebugTrace> make_trace(skiatest::Reporter* r, SkSL::String src) {
    SkSL::ShaderCaps caps;
    SkSL::Compiler compiler(&caps);
    SkSL::Program::Settings settings;
    settings.fOptimize = false;

    skvm::Builder b;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::ProgramKind::kGeneric,
                                                                     src, settings);
    REPORTER_ASSERT(r, program);

    const SkSL::FunctionDefinition* main = SkSL::Program_GetFunction(*program, "main");
    auto debugTrace = sk_make_sp<SkSL::SkVMDebugTrace>();
    SkSL::ProgramToSkVM(*program, *main, &b, debugTrace.get(), /*uniforms=*/{});
    skvm::Program p = b.done();
    REPORTER_ASSERT(r, p.nargs() == 1);

    int result;
    p.eval(1, &result);

    return debugTrace;
}

static std::string make_stack_string(const SkSL::SkVMDebugTrace& trace,
                                     const SkSL::SkVMDebugTracePlayer& player) {
    std::vector<int> callStack = player.getCallStack();
    std::string text;
    const char* separator = "";
    for (int frame : callStack) {
        text += separator;
        separator = " -> ";

        if (frame >= 0 && (size_t)frame < trace.fFuncInfo.size()) {
            text += trace.fFuncInfo[frame].name;
        } else {
            text += "???";
        }
    }

    return text;
}

static std::string make_vars_string(
        const SkSL::SkVMDebugTrace& trace,
        const std::vector<SkSL::SkVMDebugTracePlayer::VariableData>& vars) {
    std::string text;
    const char* separator = "";
    for (const SkSL::SkVMDebugTracePlayer::VariableData& var : vars) {
        text += separator;
        separator = ", ";

        if (var.fSlotIndex >= 0 && (size_t)var.fSlotIndex < trace.fSlotInfo.size()) {
            text += trace.fSlotInfo[var.fSlotIndex].name;
        } else {
            text += "???";
        }
        text += " = ";
        text += std::to_string(var.fValue);
    }

    return text;
}

static std::string make_local_vars_string(const SkSL::SkVMDebugTrace& trace,
                                          const SkSL::SkVMDebugTracePlayer& player,
                                          int frame) {
    return make_vars_string(trace, player.getLocalVariables(frame));
}

static std::string make_global_vars_string(const SkSL::SkVMDebugTrace& trace,
                                           const SkSL::SkVMDebugTracePlayer& player) {
    return make_vars_string(trace, player.getGlobalVariables());
}

DEF_TEST(SkSLTracePlayerHelloWorld, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                // Line 1
int main() {       // Line 2
    return 2 + 2;  // Line 3
}                  // Line 4
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);

    // We have not started tracing yet.
    REPORTER_ASSERT(r, player.cursor() == 0);
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, !player.hasTraceCompleted());
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());

    player.step();

    // We should now be inside main.
    REPORTER_ASSERT(r, player.cursor() > 0);
    REPORTER_ASSERT(r, !player.hasTraceCompleted());
    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());
    REPORTER_ASSERT(r, player.getLocalVariables(0).empty());

    player.step();

    // We have now exited main.
    REPORTER_ASSERT(r, player.cursor() > 0);
    REPORTER_ASSERT(r, !player.hasTraceCompleted());
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "[main].result = 4");

    player.step();

    // We should now have completed execution.
    REPORTER_ASSERT(r, player.hasTraceCompleted());
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, player.getCallStack().empty());
}

DEF_TEST(SkSLTracePlayerReset, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                // Line 1
int main() {       // Line 2
    return 2 + 2;  // Line 3
}                  // Line 4
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);

    // We have not started tracing yet.
    REPORTER_ASSERT(r, player.cursor() == 0);
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, !player.hasTraceCompleted());
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());

    player.step();

    // We should now be inside main.
    REPORTER_ASSERT(r, player.cursor() > 0);
    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());
    REPORTER_ASSERT(r, player.getLocalVariables(0).empty());

    player.reset(trace);

    // We should be back to square one.
    REPORTER_ASSERT(r, player.cursor() == 0);
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, !player.hasTraceCompleted());
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());
}

DEF_TEST(SkSLTracePlayerFunctions, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                             // Line 1
int fnB() {                     // Line 2
    return 2 + 2;               // Line 3
}                               // Line 4
int fnA() {                     // Line 5
    return fnB();               // Line 6
}                               // Line 7
int main() {                    // Line 8
    return fnA();               // Line 9
}                               // Line 10
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);

    // We have not started tracing yet.
    REPORTER_ASSERT(r, player.cursor() == 0);
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, !player.hasTraceCompleted());
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());

    player.step();

    // We should now be inside main.
    REPORTER_ASSERT(r, !player.hasTraceCompleted());
    REPORTER_ASSERT(r, player.getCurrentLine() == 9);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());
    REPORTER_ASSERT(r, player.getLocalVariables(0).empty());

    player.stepOver();

    // We should now be at the exit of main.
    REPORTER_ASSERT(r, !player.hasTraceCompleted());
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "[main].result = 4");

    player.step();

    // We should now have completed execution.
    REPORTER_ASSERT(r, player.hasTraceCompleted());

    // Watch the stack grow and shrink as single-step.
    player.reset(trace);
    player.step();
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    player.step();
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fnA()");
    player.step();
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fnA() -> int fnB()");
    player.step();
    player.step();
    player.step();
    player.step();
    player.step();
    player.step();
}
