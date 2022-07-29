/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkHalf.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkThreadID.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCpu.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkOpts.h"
#include "src/core/SkStreamPriv.h"
#include "src/core/SkVM.h"
#include <algorithm>
#include <atomic>
#include <queue>

#if !defined(SK_BUILD_FOR_WIN)
#include <unistd.h>
#endif

#if defined(SKSL_STANDALONE)
    // skslc needs to link against this module (for the VM code generator). This module pulls in
    // color-space code, but attempting to add those transitive dependencies to skslc gets out of
    // hand. So we terminate the chain here with stub functions. Note that skslc's usage of SkVM
    // never cares about color management.
    skvm::F32 sk_program_transfer_fn(
        skvm::F32 v, TFKind tf_kind,
        skvm::F32 G, skvm::F32 A, skvm::F32 B, skvm::F32 C, skvm::F32 D, skvm::F32 E, skvm::F32 F) {
            return v;
    }

    const skcms_TransferFunction* skcms_sRGB_TransferFunction() { return nullptr; }
    const skcms_TransferFunction* skcms_sRGB_Inverse_TransferFunction() { return nullptr; }
#endif

namespace skvm {

    static Features detect_features() {
        static const bool fma =
        #if defined(SK_CPU_X86)
            SkCpu::Supports(SkCpu::HSW);
        #elif defined(SK_CPU_ARM64)
            true;
        #else
            false;
        #endif

        static const bool fp16 = false;  // TODO

        return { fma, fp16 };
    }

    Builder::Builder(bool createDuplicates)
        : fFeatures(detect_features()), fCreateDuplicates(createDuplicates) {}
    Builder::Builder(Features features, bool createDuplicates)
        : fFeatures(features         ), fCreateDuplicates(createDuplicates) {}

    struct Program::Impl {
        std::vector<InterpreterInstruction> instructions;
        int regs = 0;
        int loop = 0;
        std::vector<int> strides;
        std::vector<TraceHook*> traceHooks;
    };

    // Debugging tools, mostly for printing various data structures out to a stream.

    namespace {
        struct V { Val id; };
        struct R { Reg id; };
        struct Shift       { int bits; };
        struct Splat       { int bits; };
        struct Hex         { int bits; };
        struct TraceHookID { int bits; };
        // For op `trace_line`
        struct Line  { int bits; };
        // For op `trace_var`
        struct VarSlot { int bits; };
        // For op `trace_enter`/`trace_exit`
        struct FnIdx { int bits; };

        static void write(SkWStream* o, const char* s) {
            o->writeText(s);
        }

        static const char* name(Op op) {
            switch (op) {
            #define M(x) case Op::x: return #x;
                SKVM_OPS(M)
            #undef M
            }
            return "unknown op";
        }

        static void write(SkWStream* o, Op op) {
            o->writeText(name(op));
        }
        static void write(SkWStream* o, Ptr p) {
            write(o, "ptr");
            o->writeDecAsText(p.ix);
        }
        static void write(SkWStream* o, V v) {
            write(o, "v");
            o->writeDecAsText(v.id);
        }
        static void write(SkWStream* o, R r) {
            write(o, "r");
            o->writeDecAsText(r.id);
        }
        static void write(SkWStream* o, Shift s) {
            o->writeDecAsText(s.bits);
        }
        static void write(SkWStream* o, Splat s) {
            float f;
            memcpy(&f, &s.bits, 4);
            o->writeHexAsText(s.bits);
            write(o, " (");
            o->writeScalarAsText(f);
            write(o, ")");
        }
        static void write(SkWStream* o, Hex h) {
            o->writeHexAsText(h.bits);
        }
        static void write(SkWStream* o, TraceHookID h) {
            o->writeDecAsText(h.bits);
        }
        static void write(SkWStream* o, Line d) {
            write(o, "L");
            o->writeDecAsText(d.bits);
        }
        static void write(SkWStream* o, VarSlot s) {
            write(o, "$");
            o->writeDecAsText(s.bits);
        }
        static void write(SkWStream* o, FnIdx s) {
            write(o, "F");
            o->writeDecAsText(s.bits);
        }
        template <typename T, typename... Ts>
        static void write(SkWStream* o, T first, Ts... rest) {
            write(o, first);
            write(o, " ");
            write(o, rest...);
        }
    }  // namespace

    static void write_one_instruction(Val id, const OptimizedInstruction& inst, SkWStream* o) {
        Op  op = inst.op;
        Val  x = inst.x,
             y = inst.y,
             z = inst.z,
             w = inst.w;
        int immA = inst.immA,
            immB = inst.immB,
            immC = inst.immC;
        switch (op) {
            case Op::assert_true: write(o, op, V{x}, V{y}); break;

            case Op::trace_line:  write(o, op, TraceHookID{immA}, V{x}, V{y}, Line{immB}); break;
            case Op::trace_var:   write(o, op, TraceHookID{immA}, V{x}, V{y},
                                                                  VarSlot{immB}, "=", V{z}); break;
            case Op::trace_enter: write(o, op, TraceHookID{immA}, V{x}, V{y}, FnIdx{immB}); break;
            case Op::trace_exit:  write(o, op, TraceHookID{immA}, V{x}, V{y}, FnIdx{immB}); break;
            case Op::trace_scope: write(o, op, TraceHookID{immA}, V{x}, V{y}, Shift{immB}); break;

            case Op::store8:   write(o, op, Ptr{immA}, V{x}               ); break;
            case Op::store16:  write(o, op, Ptr{immA}, V{x}               ); break;
            case Op::store32:  write(o, op, Ptr{immA}, V{x}               ); break;
            case Op::store64:  write(o, op, Ptr{immA}, V{x},V{y}          ); break;
            case Op::store128: write(o, op, Ptr{immA}, V{x},V{y},V{z},V{w}); break;

            case Op::index: write(o, V{id}, "=", op); break;

            case Op::load8:   write(o, V{id}, "=", op, Ptr{immA}); break;
            case Op::load16:  write(o, V{id}, "=", op, Ptr{immA}); break;
            case Op::load32:  write(o, V{id}, "=", op, Ptr{immA}); break;
            case Op::load64:  write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}); break;
            case Op::load128: write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}); break;

            case Op::gather8:  write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}, V{x}); break;
            case Op::gather16: write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}, V{x}); break;
            case Op::gather32: write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}, V{x}); break;

            case Op::uniform32: write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}); break;
            case Op::array32:   write(o, V{id}, "=", op, Ptr{immA}, Hex{immB}, Hex{immC}); break;

            case Op::splat: write(o, V{id}, "=", op, Splat{immA}); break;

            case Op:: add_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: sub_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: mul_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: div_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: min_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: max_f32: write(o, V{id}, "=", op, V{x}, V{y}      ); break;
            case Op:: fma_f32: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;
            case Op:: fms_f32: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;
            case Op::fnma_f32: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;


            case Op::sqrt_f32: write(o, V{id}, "=", op, V{x}); break;

            case Op:: eq_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::neq_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op:: gt_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::gte_f32: write(o, V{id}, "=", op, V{x}, V{y}); break;


            case Op::add_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::sub_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::mul_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;

            case Op::shl_i32: write(o, V{id}, "=", op, V{x}, Shift{immA}); break;
            case Op::shr_i32: write(o, V{id}, "=", op, V{x}, Shift{immA}); break;
            case Op::sra_i32: write(o, V{id}, "=", op, V{x}, Shift{immA}); break;

            case Op::eq_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::gt_i32: write(o, V{id}, "=", op, V{x}, V{y}); break;


            case Op::bit_and  : write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::bit_or   : write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::bit_xor  : write(o, V{id}, "=", op, V{x}, V{y}); break;
            case Op::bit_clear: write(o, V{id}, "=", op, V{x}, V{y}); break;

            case Op::select: write(o, V{id}, "=", op, V{x}, V{y}, V{z}); break;

            case Op::ceil:      write(o, V{id}, "=", op, V{x}); break;
            case Op::floor:     write(o, V{id}, "=", op, V{x}); break;
            case Op::to_f32:    write(o, V{id}, "=", op, V{x}); break;
            case Op::to_fp16:   write(o, V{id}, "=", op, V{x}); break;
            case Op::from_fp16: write(o, V{id}, "=", op, V{x}); break;
            case Op::trunc:     write(o, V{id}, "=", op, V{x}); break;
            case Op::round:     write(o, V{id}, "=", op, V{x}); break;

            case Op::duplicate: write(o, V{id}, "=", op, Hex{immA}); break;
        }

        write(o, "\n");
    }

    void Builder::dump(SkWStream* o) const {
        SkDebugfStream debug;
        if (!o) { o = &debug; }

        std::vector<OptimizedInstruction> optimized = this->optimize();
        o->writeDecAsText(optimized.size());
        o->writeText(" values (originally ");
        o->writeDecAsText(fProgram.size());
        o->writeText("):\n");
        for (Val id = 0; id < (Val)optimized.size(); id++) {
            const OptimizedInstruction& inst = optimized[id];
            write(o, inst.can_hoist ? "↑ " : "  ");
            write_one_instruction(id, inst, o);
        }
    }

    void Program::dump(SkWStream* o) const {
        SkDebugfStream debug;
        if (!o) { o = &debug; }

        o->writeDecAsText(fImpl->regs);
        o->writeText(" registers, ");
        o->writeDecAsText(fImpl->instructions.size());
        o->writeText(" instructions:\n");
        for (Val i = 0; i < (Val)fImpl->instructions.size(); i++) {
            if (i == fImpl->loop) { write(o, "loop:\n"); }
            o->writeDecAsText(i);
            o->writeText("\t");
            if (i >= fImpl->loop) { write(o, "    "); }
            const InterpreterInstruction& inst = fImpl->instructions[i];
            Op   op = inst.op;
            Reg   d = inst.d,
                  x = inst.x,
                  y = inst.y,
                  z = inst.z,
                  w = inst.w;
            int immA = inst.immA,
                immB = inst.immB,
                immC = inst.immC;
            switch (op) {
                case Op::assert_true: write(o, op, R{x}, R{y}); break;

                case Op::trace_line:  write(o, op, TraceHookID{immA},
                                                   R{x}, R{y}, Line{immB}); break;
                case Op::trace_var:   write(o, op, TraceHookID{immA}, R{x}, R{y},
                                                   VarSlot{immB}, "=", R{z}); break;
                case Op::trace_enter: write(o, op, TraceHookID{immA},
                                                   R{x}, R{y}, FnIdx{immB}); break;
                case Op::trace_exit:  write(o, op, TraceHookID{immA},
                                                   R{x}, R{y}, FnIdx{immB}); break;
                case Op::trace_scope: write(o, op, TraceHookID{immA},
                                                   R{x}, R{y}, Shift{immB}); break;

                case Op::store8:   write(o, op, Ptr{immA}, R{x}                  ); break;
                case Op::store16:  write(o, op, Ptr{immA}, R{x}                  ); break;
                case Op::store32:  write(o, op, Ptr{immA}, R{x}                  ); break;
                case Op::store64:  write(o, op, Ptr{immA}, R{x}, R{y}            ); break;
                case Op::store128: write(o, op, Ptr{immA}, R{x}, R{y}, R{z}, R{w}); break;

                case Op::index: write(o, R{d}, "=", op); break;

                case Op::load8:   write(o, R{d}, "=", op, Ptr{immA}); break;
                case Op::load16:  write(o, R{d}, "=", op, Ptr{immA}); break;
                case Op::load32:  write(o, R{d}, "=", op, Ptr{immA}); break;
                case Op::load64:  write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}); break;
                case Op::load128: write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}); break;

                case Op::gather8:  write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}, R{x}); break;
                case Op::gather16: write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}, R{x}); break;
                case Op::gather32: write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}, R{x}); break;

                case Op::uniform32: write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}); break;
                case Op::array32:   write(o, R{d}, "=", op, Ptr{immA}, Hex{immB}, Hex{immC}); break;

                case Op::splat:     write(o, R{d}, "=", op, Splat{immA}); break;

                case Op::add_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::sub_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::mul_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::div_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::min_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::max_f32: write(o, R{d}, "=", op, R{x}, R{y}      ); break;
                case Op::fma_f32: write(o, R{d}, "=", op, R{x}, R{y}, R{z}); break;
                case Op::fms_f32: write(o, R{d}, "=", op, R{x}, R{y}, R{z}); break;
                case Op::fnma_f32: write(o, R{d}, "=", op, R{x}, R{y}, R{z}); break;

                case Op::sqrt_f32: write(o, R{d}, "=", op, R{x}); break;

                case Op:: eq_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::neq_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op:: gt_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::gte_f32: write(o, R{d}, "=", op, R{x}, R{y}); break;


                case Op::add_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::sub_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::mul_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;

                case Op::shl_i32: write(o, R{d}, "=", op, R{x}, Shift{immA}); break;
                case Op::shr_i32: write(o, R{d}, "=", op, R{x}, Shift{immA}); break;
                case Op::sra_i32: write(o, R{d}, "=", op, R{x}, Shift{immA}); break;

                case Op::eq_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::gt_i32: write(o, R{d}, "=", op, R{x}, R{y}); break;

                case Op::bit_and  : write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::bit_or   : write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::bit_xor  : write(o, R{d}, "=", op, R{x}, R{y}); break;
                case Op::bit_clear: write(o, R{d}, "=", op, R{x}, R{y}); break;

                case Op::select: write(o, R{d}, "=", op, R{x}, R{y}, R{z}); break;

                case Op::ceil:      write(o, R{d}, "=", op, R{x}); break;
                case Op::floor:     write(o, R{d}, "=", op, R{x}); break;
                case Op::to_f32:    write(o, R{d}, "=", op, R{x}); break;
                case Op::to_fp16:   write(o, R{d}, "=", op, R{x}); break;
                case Op::from_fp16: write(o, R{d}, "=", op, R{x}); break;
                case Op::trunc:     write(o, R{d}, "=", op, R{x}); break;
                case Op::round:     write(o, R{d}, "=", op, R{x}); break;

                case Op::duplicate: write(o, R{d}, "=", op, Hex{immA}); break;
            }
            write(o, "\n");
        }
    }
    std::vector<Instruction> eliminate_dead_code(std::vector<Instruction> program) {
        // Determine which Instructions are live by working back from side effects.
        std::vector<bool> live(program.size(), false);
        for (Val id = program.size(); id--;) {
            if (live[id] || has_side_effect(program[id].op)) {
                live[id] = true;
                const Instruction& inst = program[id];
                for (Val arg : {inst.x, inst.y, inst.z, inst.w}) {
                    if (arg != NA) { live[arg] = true; }
                }
            }
        }

        // Rewrite the program with only live Instructions:
        //   - remap IDs in live Instructions to what they'll be once dead Instructions are removed;
        //   - then actually remove the dead Instructions.
        std::vector<Val> new_id(program.size(), NA);
        for (Val id = 0, next = 0; id < (Val)program.size(); id++) {
            if (live[id]) {
                Instruction& inst = program[id];
                for (Val* arg : {&inst.x, &inst.y, &inst.z, &inst.w}) {
                    if (*arg != NA) {
                        *arg = new_id[*arg];
                        SkASSERT(*arg != NA);
                    }
                }
                new_id[id] = next++;
            }
        }

        // Eliminate any non-live ops.
        auto it = std::remove_if(program.begin(), program.end(), [&](const Instruction& inst) {
            Val id = (Val)(&inst - program.data());
            return !live[id];
        });
        program.erase(it, program.end());

        return program;
    }

    std::vector<OptimizedInstruction> finalize(const std::vector<Instruction> program) {
        std::vector<OptimizedInstruction> optimized(program.size());
        for (Val id = 0; id < (Val)program.size(); id++) {
            Instruction inst = program[id];
            optimized[id] = {inst.op, inst.x,inst.y,inst.z,inst.w,
                             inst.immA,inst.immB,inst.immC,
                             /*death=*/id, /*can_hoist=*/true};
        }

        // Each Instruction's inputs need to live at least until that Instruction issues.
        for (Val id = 0; id < (Val)optimized.size(); id++) {
            OptimizedInstruction& inst = optimized[id];
            for (Val arg : {inst.x, inst.y, inst.z, inst.w}) {
                // (We're walking in order, so this is the same as max()ing with the existing Val.)
                if (arg != NA) { optimized[arg].death = id; }
            }
        }

        // Mark which values don't depend on the loop and can be hoisted.
        for (OptimizedInstruction& inst : optimized) {
            // Varying loads (and gathers) and stores cannot be hoisted out of the loop.
            if (is_always_varying(inst.op) || is_trace(inst.op)) {
                inst.can_hoist = false;
            }

            // If any of an instruction's inputs can't be hoisted, it can't be hoisted itself.
            if (inst.can_hoist) {
                for (Val arg : {inst.x, inst.y, inst.z, inst.w}) {
                    if (arg != NA) { inst.can_hoist &= optimized[arg].can_hoist; }
                }
            }
        }

        // Extend the lifetime of any hoisted value that's used in the loop to infinity.
        for (OptimizedInstruction& inst : optimized) {
            if (!inst.can_hoist /*i.e. we're in the loop, so the arguments are used-in-loop*/) {
                for (Val arg : {inst.x, inst.y, inst.z, inst.w}) {
                    if (arg != NA && optimized[arg].can_hoist) {
                        optimized[arg].death = (Val)program.size();
                    }
                }
            }
        }

        return optimized;
    }

    std::vector<OptimizedInstruction> Builder::optimize() const {
        std::vector<Instruction> program = this->program();
        program = eliminate_dead_code(std::move(program));
        return    finalize           (std::move(program));
    }

    Program Builder::done() const {
        auto optimized = this->optimize();
        return {optimized, fStrides, fTraceHooks};
    }

    uint64_t Builder::hash() const {
        uint32_t lo = SkOpts::hash(fProgram.data(), fProgram.size() * sizeof(Instruction), 0),
                 hi = SkOpts::hash(fProgram.data(), fProgram.size() * sizeof(Instruction), 1);
        return (uint64_t)lo | (uint64_t)hi << 32;
    }

    bool operator!=(Ptr a, Ptr b) { return a.ix != b.ix; }

    bool operator==(const Instruction& a, const Instruction& b) {
        return a.op   == b.op
            && a.x    == b.x
            && a.y    == b.y
            && a.z    == b.z
            && a.w    == b.w
            && a.immA == b.immA
            && a.immB == b.immB
            && a.immC == b.immC;
    }

    uint32_t InstructionHash::operator()(const Instruction& inst, uint32_t seed) const {
        return SkOpts::hash(&inst, sizeof(inst), seed);
    }


    // Most instructions produce a value and return it by ID,
    // the value-producing instruction's own index in the program vector.
    Val Builder::push(Instruction inst) {
        // Basic common subexpression elimination:
        // if we've already seen this exact Instruction, use it instead of creating a new one.
        //
        // But we never dedup loads or stores: an intervening store could change that memory.
        // Uniforms and gathers touch only uniform memory, so they're fine to dedup,
        // and index is varying but doesn't touch memory, so it's fine to dedup too.
        if (!touches_varying_memory(inst.op) && !is_trace(inst.op)) {
            if (Val* id = fIndex.find(inst)) {
                if (fCreateDuplicates) {
                    inst.op = Op::duplicate;
                    inst.immA = *id;
                    fProgram.push_back(inst);
                }
                return *id;
            }
        }

        Val id = static_cast<Val>(fProgram.size());
        fProgram.push_back(inst);
        fIndex.set(inst, id);
        return id;
    }

    Ptr Builder::arg(int stride) {
        int ix = (int)fStrides.size();
        fStrides.push_back(stride);
        return {ix};
    }

    void Builder::assert_true(I32 cond, I32 debug) {
    #ifdef SK_DEBUG
        int imm;
        if (this->allImm(cond.id,&imm)) { SkASSERT(imm); return; }
        (void)push(Op::assert_true, cond.id, debug.id);
    #endif
    }

    int Builder::attachTraceHook(TraceHook* hook) {
        int traceHookID = (int)fTraceHooks.size();
        fTraceHooks.push_back(hook);
        return traceHookID;
    }

    bool Builder::mergeMasks(I32& mask, I32& traceMask) {
        if (this->isImm(mask.id,      0)) { return false; }
        if (this->isImm(traceMask.id, 0)) { return false; }
        if (this->isImm(mask.id,     ~0)) { mask = traceMask; }
        if (this->isImm(traceMask.id,~0)) { traceMask = mask; }
        return true;
    }

    void Builder::trace_line(int traceHookID, I32 mask, I32 traceMask, int line) {
        SkASSERT(traceHookID >= 0);
        SkASSERT(traceHookID < (int)fTraceHooks.size());
        if (!this->mergeMasks(mask, traceMask)) { return; }
        (void)push(Op::trace_line, mask.id,traceMask.id,NA,NA, traceHookID, line);
    }
    void Builder::trace_var(int traceHookID, I32 mask, I32 traceMask, int slot, I32 val) {
        SkASSERT(traceHookID >= 0);
        SkASSERT(traceHookID < (int)fTraceHooks.size());
        if (!this->mergeMasks(mask, traceMask)) { return; }
        (void)push(Op::trace_var, mask.id,traceMask.id,val.id,NA, traceHookID, slot);
    }
    void Builder::trace_enter(int traceHookID, I32 mask, I32 traceMask, int fnIdx) {
        SkASSERT(traceHookID >= 0);
        SkASSERT(traceHookID < (int)fTraceHooks.size());
        if (!this->mergeMasks(mask, traceMask)) { return; }
        (void)push(Op::trace_enter, mask.id,traceMask.id,NA,NA, traceHookID, fnIdx);
    }
    void Builder::trace_exit(int traceHookID, I32 mask, I32 traceMask, int fnIdx) {
        SkASSERT(traceHookID >= 0);
        SkASSERT(traceHookID < (int)fTraceHooks.size());
        if (!this->mergeMasks(mask, traceMask)) { return; }
        (void)push(Op::trace_exit, mask.id,traceMask.id,NA,NA, traceHookID, fnIdx);
    }
    void Builder::trace_scope(int traceHookID, I32 mask, I32 traceMask, int delta) {
        SkASSERT(traceHookID >= 0);
        SkASSERT(traceHookID < (int)fTraceHooks.size());
        if (!this->mergeMasks(mask, traceMask)) { return; }
        (void)push(Op::trace_scope, mask.id,traceMask.id,NA,NA, traceHookID, delta);
    }

    void Builder::store8 (Ptr ptr, I32 val) { (void)push(Op::store8 , val.id,NA,NA,NA, ptr.ix); }
    void Builder::store16(Ptr ptr, I32 val) { (void)push(Op::store16, val.id,NA,NA,NA, ptr.ix); }
    void Builder::store32(Ptr ptr, I32 val) { (void)push(Op::store32, val.id,NA,NA,NA, ptr.ix); }
    void Builder::store64(Ptr ptr, I32 lo, I32 hi) {
        (void)push(Op::store64, lo.id,hi.id,NA,NA, ptr.ix);
    }
    void Builder::store128(Ptr ptr, I32 x, I32 y, I32 z, I32 w) {
        (void)push(Op::store128, x.id,y.id,z.id,w.id, ptr.ix);
    }

    I32 Builder::index() { return {this, push(Op::index)}; }

    I32 Builder::load8 (Ptr ptr) { return {this, push(Op::load8 , NA,NA,NA,NA, ptr.ix) }; }
    I32 Builder::load16(Ptr ptr) { return {this, push(Op::load16, NA,NA,NA,NA, ptr.ix) }; }
    I32 Builder::load32(Ptr ptr) { return {this, push(Op::load32, NA,NA,NA,NA, ptr.ix) }; }
    I32 Builder::load64(Ptr ptr, int lane) {
        return {this, push(Op::load64 , NA,NA,NA,NA, ptr.ix,lane) };
    }
    I32 Builder::load128(Ptr ptr, int lane) {
        return {this, push(Op::load128, NA,NA,NA,NA, ptr.ix,lane) };
    }

    I32 Builder::gather8 (UPtr ptr, int offset, I32 index) {
        return {this, push(Op::gather8 , index.id,NA,NA,NA, ptr.ix,offset)};
    }
    I32 Builder::gather16(UPtr ptr, int offset, I32 index) {
        return {this, push(Op::gather16, index.id,NA,NA,NA, ptr.ix,offset)};
    }
    I32 Builder::gather32(UPtr ptr, int offset, I32 index) {
        return {this, push(Op::gather32, index.id,NA,NA,NA, ptr.ix,offset)};
    }

    I32 Builder::uniform32(UPtr ptr, int offset) {
        return {this, push(Op::uniform32, NA,NA,NA,NA, ptr.ix, offset)};
    }

    // Note: this converts the array index into a byte offset for the op.
    I32 Builder::array32  (UPtr ptr, int offset, int index) {
        return {this, push(Op::array32, NA,NA,NA,NA, ptr.ix, offset, index * sizeof(int))};
    }

    I32 Builder::splat(int n) { return {this, push(Op::splat, NA,NA,NA,NA, n) }; }

    template <typename F32_or_I32>
    void Builder::canonicalizeIdOrder(F32_or_I32& x, F32_or_I32& y) {
        bool immX = fProgram[x.id].op == Op::splat;
        bool immY = fProgram[y.id].op == Op::splat;
        if (immX != immY) {
            if (immX) {
                // Prefer (val, imm) over (imm, val).
                std::swap(x, y);
            }
            return;
        }
        if (x.id > y.id) {
            // Prefer (lower-ID, higher-ID) over (higher-ID, lower-ID).
            std::swap(x, y);
        }
    }

    // Be careful peepholing float math!  Transformations you might expect to
    // be legal can fail in the face of NaN/Inf, e.g. 0*x is not always 0.
    // Float peepholes must pass this equivalence test for all ~4B floats:
    //
    //     bool equiv(float x, float y) { return (x == y) || (isnanf(x) && isnanf(y)); }
    //
    //     unsigned bits = 0;
    //     do {
    //        float f;
    //        memcpy(&f, &bits, 4);
    //        if (!equiv(f, ...)) {
    //           abort();
    //        }
    //     } while (++bits != 0);

    F32 Builder::add(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X+Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0.0f)) { return x; }   // x+0 == x

        if (fFeatures.fma) {
            if (fProgram[x.id].op == Op::mul_f32) {
                return {this, this->push(Op::fma_f32, fProgram[x.id].x, fProgram[x.id].y, y.id)};
            }
            if (fProgram[y.id].op == Op::mul_f32) {
                return {this, this->push(Op::fma_f32, fProgram[y.id].x, fProgram[y.id].y, x.id)};
            }
        }
        return {this, this->push(Op::add_f32, x.id, y.id)};
    }

    F32 Builder::sub(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X-Y); }
        if (this->isImm(y.id, 0.0f)) { return x; }   // x-0 == x
        if (fFeatures.fma) {
            if (fProgram[x.id].op == Op::mul_f32) {
                return {this, this->push(Op::fms_f32, fProgram[x.id].x, fProgram[x.id].y, y.id)};
            }
            if (fProgram[y.id].op == Op::mul_f32) {
                return {this, this->push(Op::fnma_f32, fProgram[y.id].x, fProgram[y.id].y, x.id)};
            }
        }
        return {this, this->push(Op::sub_f32, x.id, y.id)};
    }

    F32 Builder::mul(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X*Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 1.0f)) { return x; }  // x*1 == x
        return {this, this->push(Op::mul_f32, x.id, y.id)};
    }

    F32 Builder::fast_mul(F32 x, F32 y) {
        if (this->isImm(x.id, 0.0f) || this->isImm(y.id, 0.0f)) { return splat(0.0f); }
        return mul(x,y);
    }

    F32 Builder::div(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(sk_ieee_float_divide(X,Y)); }
        if (this->isImm(y.id, 1.0f)) { return x; }  // x/1 == x
        return {this, this->push(Op::div_f32, x.id, y.id)};
    }

    F32 Builder::sqrt(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat(std::sqrt(X)); }
        return {this, this->push(Op::sqrt_f32, x.id)};
    }

    // See http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html.
    F32 Builder::approx_log2(F32 x) {
        // e - 127 is a fair approximation of log2(x) in its own right...
        F32 e = mul(to_F32(pun_to_I32(x)), splat(1.0f / (1<<23)));

        // ... but using the mantissa to refine its error is _much_ better.
        F32 m = pun_to_F32(bit_or(bit_and(pun_to_I32(x), 0x007fffff),
                                0x3f000000));
        F32 approx = sub(e,        124.225514990f);
            approx = sub(approx, mul(1.498030302f, m));
            approx = sub(approx, div(1.725879990f, add(0.3520887068f, m)));

        return approx;
    }

    F32 Builder::approx_pow2(F32 x) {
        constexpr float kInfinityBits = 0x7f800000;

        F32 f = fract(x);
        F32 approx = add(x,         121.274057500f);
            approx = sub(approx, mul( 1.490129070f, f));
            approx = add(approx, div(27.728023300f, sub(4.84252568f, f)));
            approx = mul(1.0f * (1<<23), approx);
            approx = clamp(approx, 0, kInfinityBits);  // guard against underflow/overflow

        return pun_to_F32(round(approx));
    }

    F32 Builder::approx_powf(F32 x, F32 y) {
        // TODO: assert this instead?  Sometimes x is very slightly negative.  See skia:10210.
        x = max(0.0f, x);

        if (this->isImm(x.id, 1.0f)) { return x; }                    // 1^y is one
        if (this->isImm(x.id, 2.0f)) { return this->approx_pow2(y); } // 2^y is pow2(y)
        if (this->isImm(y.id, 0.5f)) { return this->sqrt(x); }        // x^0.5 is sqrt(x)
        if (this->isImm(y.id, 1.0f)) { return x; }                    // x^1 is x
        if (this->isImm(y.id, 2.0f)) { return x * x; }                // x^2 is x*x

        auto is_x = bit_or(eq(x, 0.0f),
                           eq(x, 1.0f));
        return select(is_x, x, approx_pow2(mul(approx_log2(x), y)));
    }

    // Bhaskara I's sine approximation
    // 16x(pi - x) / (5*pi^2 - 4x(pi - x)
    // ... divide by 4
    // 4x(pi - x) / 5*pi^2/4 - x(pi - x)
    //
    // This is a good approximation only for 0 <= x <= pi, so we use symmetries to get
    // radians into that range first.
    //
    F32 Builder::approx_sin(F32 radians) {
        constexpr float Pi = SK_ScalarPI;
        // x = radians mod 2pi
        F32 x = fract(radians * (0.5f/Pi)) * (2*Pi);
        I32 neg = x > Pi;   // are we pi < x < 2pi --> need to negate result
        x = select(neg, x - Pi, x);

        F32 pair = x * (Pi - x);
        x = 4.0f * pair / ((5*Pi*Pi/4) - pair);
        x = select(neg, -x, x);
        return x;
    }

    /*  "GENERATING ACCURATE VALUES FOR THE TANGENT FUNCTION"
         https://mae.ufl.edu/~uhk/ACCURATE-TANGENT.pdf

        approx = x + (1/3)x^3 + (2/15)x^5 + (17/315)x^7 + (62/2835)x^9

        Some simplifications:
        1. tan(x) is periodic, -PI/2 < x < PI/2
        2. tan(x) is odd, so tan(-x) = -tan(x)
        3. Our polynomial approximation is best near zero, so we use the following identity
                        tan(x) + tan(y)
           tan(x + y) = -----------------
                       1 - tan(x)*tan(y)
           tan(PI/4) = 1

           So for x > PI/8, we do the following refactor:
           x' = x - PI/4

                    1 + tan(x')
           tan(x) = ------------
                    1 - tan(x')
     */
    F32 Builder::approx_tan(F32 x) {
        constexpr float Pi = SK_ScalarPI;
        // periodic between -pi/2 ... pi/2
        // shift to 0...Pi, scale 1/Pi to get into 0...1, then fract, scale-up, shift-back
        x = fract((1/Pi)*x + 0.5f) * Pi - (Pi/2);

        I32 neg = (x < 0.0f);
        x = select(neg, -x, x);

        // minimize total error by shifting if x > pi/8
        I32 use_quotient = (x > (Pi/8));
        x = select(use_quotient, x - (Pi/4), x);

        // 9th order poly = 4th order(x^2) * x
        x = poly(x*x, 62/2835.0f, 17/315.0f, 2/15.0f, 1/3.0f, 1.0f) * x;
        x = select(use_quotient, (1+x)/(1-x), x);
        x = select(neg, -x, x);
        return x;
    }

     // http://mathforum.org/library/drmath/view/54137.html
     // referencing Handbook of Mathematical Functions,
     //             by Milton Abramowitz and Irene Stegun
     F32 Builder::approx_asin(F32 x) {
         I32 neg = (x < 0.0f);
         x = select(neg, -x, x);
         x = SK_ScalarPI/2 - sqrt(1-x) * poly(x, -0.0187293f, 0.0742610f, -0.2121144f, 1.5707288f);
         x = select(neg, -x, x);
         return x;
     }

    /*  Use 4th order polynomial approximation from https://arachnoid.com/polysolve/
     *      with 129 values of x,atan(x) for x:[0...1]
     *  This only works for 0 <= x <= 1
     */
    static F32 approx_atan_unit(F32 x) {
        // for now we might be given NaN... let that through
        x->assert_true((x != x) | ((x >= 0) & (x <= 1)));
        return poly(x, 0.14130025741326729f,
                      -0.34312835980675116f,
                      -0.016172900528248768f,
                       1.0037696976200385f,
                      -0.00014758242182738969f);
    }

    /*  Use identity atan(x) = pi/2 - atan(1/x) for x > 1
     */
    F32 Builder::approx_atan(F32 x) {
        I32 neg = (x < 0.0f);
        x = select(neg, -x, x);
        I32 flip = (x > 1.0f);
        x = select(flip, 1/x, x);
        x = approx_atan_unit(x);
        x = select(flip, SK_ScalarPI/2 - x, x);
        x = select(neg, -x, x);
        return x;
    }

    /*  Use identity atan(x) = pi/2 - atan(1/x) for x > 1
     *  By swapping y,x to ensure the ratio is <= 1, we can safely call atan_unit()
     *  which avoids a 2nd divide instruction if we had instead called atan().
     */
    F32 Builder::approx_atan2(F32 y0, F32 x0) {

        I32 flip = (abs(y0) > abs(x0));
        F32 y = select(flip, x0, y0);
        F32 x = select(flip, y0, x0);
        F32 arg = y/x;

        I32 neg = (arg < 0.0f);
        arg = select(neg, -arg, arg);

        F32 r = approx_atan_unit(arg);
        r = select(flip, SK_ScalarPI/2 - r, r);
        r = select(neg, -r, r);

        // handle quadrant distinctions
        r = select((y0 >= 0) & (x0  < 0), r + SK_ScalarPI, r);
        r = select((y0  < 0) & (x0 <= 0), r - SK_ScalarPI, r);
        // Note: we don't try to handle 0,0 or infinities (yet)
        return r;
    }

    F32 Builder::min(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(std::min(X,Y)); }
        return {this, this->push(Op::min_f32, x.id, y.id)};
    }
    F32 Builder::max(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(std::max(X,Y)); }
        return {this, this->push(Op::max_f32, x.id, y.id)};
    }

    SK_ATTRIBUTE(no_sanitize("signed-integer-overflow"))
    I32 Builder::add(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X+Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0)) { return x; }  // x+0 == x
        return {this, this->push(Op::add_i32, x.id, y.id)};
    }
    SK_ATTRIBUTE(no_sanitize("signed-integer-overflow"))
    I32 Builder::sub(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X-Y); }
        if (this->isImm(y.id, 0)) { return x; }
        return {this, this->push(Op::sub_i32, x.id, y.id)};
    }
    SK_ATTRIBUTE(no_sanitize("signed-integer-overflow"))
    I32 Builder::mul(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X*Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0)) { return splat(0); }  // x*0 == 0
        if (this->isImm(y.id, 1)) { return x; }         // x*1 == x
        return {this, this->push(Op::mul_i32, x.id, y.id)};
    }

    SK_ATTRIBUTE(no_sanitize("shift"))
    I32 Builder::shl(I32 x, int bits) {
        if (bits == 0) { return x; }
        if (int X; this->allImm(x.id,&X)) { return splat(X << bits); }
        return {this, this->push(Op::shl_i32, x.id,NA,NA,NA, bits)};
    }
    I32 Builder::shr(I32 x, int bits) {
        if (bits == 0) { return x; }
        if (int X; this->allImm(x.id,&X)) { return splat(unsigned(X) >> bits); }
        return {this, this->push(Op::shr_i32, x.id,NA,NA,NA, bits)};
    }
    I32 Builder::sra(I32 x, int bits) {
        if (bits == 0) { return x; }
        if (int X; this->allImm(x.id,&X)) { return splat(X >> bits); }
        return {this, this->push(Op::sra_i32, x.id,NA,NA,NA, bits)};
    }

    I32 Builder:: eq(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X==Y ? ~0 : 0); }
        this->canonicalizeIdOrder(x, y);
        return {this, this->push(Op::eq_f32, x.id, y.id)};
    }
    I32 Builder::neq(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X!=Y ? ~0 : 0); }
        this->canonicalizeIdOrder(x, y);
        return {this, this->push(Op::neq_f32, x.id, y.id)};
    }
    I32 Builder::lt(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(Y> X ? ~0 : 0); }
        return {this, this->push(Op::gt_f32, y.id, x.id)};
    }
    I32 Builder::lte(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(Y>=X ? ~0 : 0); }
        return {this, this->push(Op::gte_f32, y.id, x.id)};
    }
    I32 Builder::gt(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X> Y ? ~0 : 0); }
        return {this, this->push(Op::gt_f32, x.id, y.id)};
    }
    I32 Builder::gte(F32 x, F32 y) {
        if (float X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X>=Y ? ~0 : 0); }
        return {this, this->push(Op::gte_f32, x.id, y.id)};
    }

    I32 Builder:: eq(I32 x, I32 y) {
        if (x.id == y.id) { return splat(~0); }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X==Y ? ~0 : 0); }
        this->canonicalizeIdOrder(x, y);
        return {this, this->push(Op:: eq_i32, x.id, y.id)};
    }
    I32 Builder::neq(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X!=Y ? ~0 : 0); }
        return ~(x == y);
    }
    I32 Builder:: gt(I32 x, I32 y) {
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X> Y ? ~0 : 0); }
        return {this, this->push(Op:: gt_i32, x.id, y.id)};
    }
    I32 Builder::gte(I32 x, I32 y) {
        if (x.id == y.id) { return splat(~0); }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X>=Y ? ~0 : 0); }
        return ~(x < y);
    }
    I32 Builder:: lt(I32 x, I32 y) { return y>x; }
    I32 Builder::lte(I32 x, I32 y) { return y>=x; }

    Val Builder::holdsBitNot(Val id) {
        // We represent `~x` as `x ^ ~0`.
        if (fProgram[id].op == Op::bit_xor && this->isImm(fProgram[id].y, ~0)) {
            return fProgram[id].x;
        }
        return NA;
    }

    I32 Builder::bit_and(I32 x, I32 y) {
        if (x.id == y.id) { return x; }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X&Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0)) { return splat(0); }         // (x & false) == false
        if (this->isImm(y.id,~0)) { return x; }                // (x & true) == x
        if (Val notX = this->holdsBitNot(x.id); notX != NA) {  // (~x & y) == bit_clear(y, ~x)
            return bit_clear(y, {this, notX});
        }
        if (Val notY = this->holdsBitNot(y.id); notY != NA) {  // (x & ~y) == bit_clear(x, ~y)
            return bit_clear(x, {this, notY});
        }
        return {this, this->push(Op::bit_and, x.id, y.id)};
    }
    I32 Builder::bit_or(I32 x, I32 y) {
        if (x.id == y.id) { return x; }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X|Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0)) { return x; }           // (x | false) == x
        if (this->isImm(y.id,~0)) { return splat(~0); }   // (x | true) == true
        return {this, this->push(Op::bit_or, x.id, y.id)};
    }
    I32 Builder::bit_xor(I32 x, I32 y) {
        if (x.id == y.id) { return splat(0); }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X^Y); }
        this->canonicalizeIdOrder(x, y);
        if (this->isImm(y.id, 0)) { return x; }   // (x ^ false) == x
        return {this, this->push(Op::bit_xor, x.id, y.id)};
    }

    I32 Builder::bit_clear(I32 x, I32 y) {
        if (x.id == y.id) { return splat(0); }
        if (int X,Y; this->allImm(x.id,&X, y.id,&Y)) { return splat(X&~Y); }
        if (this->isImm(y.id, 0)) { return x; }          // (x & ~false) == x
        if (this->isImm(y.id,~0)) { return splat(0); }   // (x & ~true) == false
        if (this->isImm(x.id, 0)) { return splat(0); }   // (false & ~y) == false
        return {this, this->push(Op::bit_clear, x.id, y.id)};
    }

    I32 Builder::select(I32 x, I32 y, I32 z) {
        if (y.id == z.id) { return y; }
        if (int X,Y,Z; this->allImm(x.id,&X, y.id,&Y, z.id,&Z)) { return splat(X?Y:Z); }
        if (this->isImm(x.id,~0)) { return y; }                // (true  ? y : z) == y
        if (this->isImm(x.id, 0)) { return z; }                // (false ? y : z) == z
        if (this->isImm(y.id, 0)) { return bit_clear(z,x); }   //     (x ? 0 : z) == ~x&z
        if (this->isImm(z.id, 0)) { return bit_and  (y,x); }   //     (x ? y : 0) ==  x&y
        if (Val notX = this->holdsBitNot(x.id); notX != NA) {  //    (!x ? y : z) == (x ? z : y)
            x.id = notX;
            std::swap(y, z);
        }
        return {this, this->push(Op::select, x.id, y.id, z.id)};
    }

    I32 Builder::extract(I32 x, int bits, I32 z) {
        if (unsigned Z; this->allImm(z.id,&Z) && (~0u>>bits) == Z) { return shr(x, bits); }
        return bit_and(z, shr(x, bits));
    }

    I32 Builder::pack(I32 x, I32 y, int bits) {
        return bit_or(x, shl(y, bits));
    }

    F32 Builder::ceil(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat(ceilf(X)); }
        return {this, this->push(Op::ceil, x.id)};
    }
    F32 Builder::floor(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat(floorf(X)); }
        return {this, this->push(Op::floor, x.id)};
    }
    F32 Builder::to_F32(I32 x) {
        if (int X; this->allImm(x.id,&X)) { return splat((float)X); }
        return {this, this->push(Op::to_f32, x.id)};
    }
    I32 Builder::trunc(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat((int)X); }
        return {this, this->push(Op::trunc, x.id)};
    }
    I32 Builder::round(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat((int)lrintf(X)); }
        return {this, this->push(Op::round, x.id)};
    }

    I32 Builder::to_fp16(F32 x) {
        if (float X; this->allImm(x.id,&X)) { return splat((int)SkFloatToHalf(X)); }
        return {this, this->push(Op::to_fp16, x.id)};
    }
    F32 Builder::from_fp16(I32 x) {
        if (int X; this->allImm(x.id,&X)) { return splat(SkHalfToFloat(X)); }
        return {this, this->push(Op::from_fp16, x.id)};
    }

    F32 Builder::from_unorm(int bits, I32 x) {
        F32 limit = splat(1 / ((1<<bits)-1.0f));
        return mul(to_F32(x), limit);
    }
    I32 Builder::to_unorm(int bits, F32 x) {
        F32 limit = splat((1<<bits)-1.0f);
        return round(mul(x, limit));
    }

    PixelFormat SkColorType_to_PixelFormat(SkColorType ct) {
        auto UNORM = PixelFormat::UNORM,
             SRGB  = PixelFormat::SRGB,
             FLOAT = PixelFormat::FLOAT;
        switch (ct) {
            case kUnknown_SkColorType: break;

            case kRGBA_F32_SkColorType: return {FLOAT,32,32,32,32, 0,32,64,96};

            case kRGBA_F16Norm_SkColorType:       return {FLOAT,16,16,16,16, 0,16,32,48};
            case kRGBA_F16_SkColorType:           return {FLOAT,16,16,16,16, 0,16,32,48};
            case kR16G16B16A16_unorm_SkColorType: return {UNORM,16,16,16,16, 0,16,32,48};

            case kA16_float_SkColorType:    return {FLOAT,  0, 0,0,16, 0, 0,0,0};
            case kR16G16_float_SkColorType: return {FLOAT, 16,16,0, 0, 0,16,0,0};

            case kAlpha_8_SkColorType:  return {UNORM, 0,0,0,8, 0,0,0,0};
            case kGray_8_SkColorType:   return {UNORM, 8,8,8,0, 0,0,0,0};  // Subtle.
            case kR8_unorm_SkColorType: return {UNORM, 8,0,0,0, 0,0,0,0};

            case kRGB_565_SkColorType:   return {UNORM, 5,6,5,0, 11,5,0,0};  // (BGR)
            case kARGB_4444_SkColorType: return {UNORM, 4,4,4,4, 12,8,4,0};  // (ABGR)

            case kRGBA_8888_SkColorType:  return {UNORM, 8,8,8,8,  0,8,16,24};
            case kRGB_888x_SkColorType:   return {UNORM, 8,8,8,0,  0,8,16,32};  // 32-bit
            case kBGRA_8888_SkColorType:  return {UNORM, 8,8,8,8, 16,8, 0,24};
            case kSRGBA_8888_SkColorType: return { SRGB, 8,8,8,8,  0,8,16,24};

            case kRGBA_1010102_SkColorType: return {UNORM, 10,10,10,2,  0,10,20,30};
            case kBGRA_1010102_SkColorType: return {UNORM, 10,10,10,2, 20,10, 0,30};
            case kRGB_101010x_SkColorType:  return {UNORM, 10,10,10,0,  0,10,20, 0};
            case kBGR_101010x_SkColorType:  return {UNORM, 10,10,10,0, 20,10, 0, 0};

            case kR8G8_unorm_SkColorType:   return {UNORM,  8, 8,0, 0, 0, 8,0,0};
            case kR16G16_unorm_SkColorType: return {UNORM, 16,16,0, 0, 0,16,0,0};
            case kA16_unorm_SkColorType:    return {UNORM,  0, 0,0,16, 0, 0,0,0};
        }
        SkASSERT(false);
        return {UNORM, 0,0,0,0, 0,0,0,0};
    }

    static int byte_size(PixelFormat f) {
        // What's the highest bit we read?
        int bits = std::max(f.r_bits + f.r_shift,
                   std::max(f.g_bits + f.g_shift,
                   std::max(f.b_bits + f.b_shift,
                            f.a_bits + f.a_shift)));
        // Round up to bytes.
        return (bits + 7) / 8;
    }

    static Color unpack(PixelFormat f, I32 x) {
        SkASSERT(byte_size(f) <= 4);

        auto from_srgb = [](int bits, I32 channel) -> F32 {
            const skcms_TransferFunction* tf = skcms_sRGB_TransferFunction();
            F32 v = from_unorm(bits, channel);
            return sk_program_transfer_fn(v, sRGBish_TF,
                                          v->splat(tf->g),
                                          v->splat(tf->a),
                                          v->splat(tf->b),
                                          v->splat(tf->c),
                                          v->splat(tf->d),
                                          v->splat(tf->e),
                                          v->splat(tf->f));
        };

        auto unpack_rgb = [=](int bits, int shift) -> F32 {
            I32 channel = extract(x, shift, (1<<bits)-1);
            switch (f.encoding) {
                case PixelFormat::UNORM: return from_unorm(bits, channel);
                case PixelFormat:: SRGB: return from_srgb (bits, channel);
                case PixelFormat::FLOAT: return from_fp16 (      channel);
            }
            SkUNREACHABLE;
        };
        auto unpack_alpha = [=](int bits, int shift) -> F32 {
            I32 channel = extract(x, shift, (1<<bits)-1);
            switch (f.encoding) {
                case PixelFormat::UNORM:
                case PixelFormat:: SRGB: return from_unorm(bits, channel);
                case PixelFormat::FLOAT: return from_fp16 (      channel);
            }
            SkUNREACHABLE;
        };
        return {
            f.r_bits ? unpack_rgb  (f.r_bits, f.r_shift) : x->splat(0.0f),
            f.g_bits ? unpack_rgb  (f.g_bits, f.g_shift) : x->splat(0.0f),
            f.b_bits ? unpack_rgb  (f.b_bits, f.b_shift) : x->splat(0.0f),
            f.a_bits ? unpack_alpha(f.a_bits, f.a_shift) : x->splat(1.0f),
        };
    }

    static void split_disjoint_8byte_format(PixelFormat f, PixelFormat* lo, PixelFormat* hi) {
        SkASSERT(byte_size(f) == 8);
        // We assume some of the channels are in the low 32 bits, some in the high 32 bits.
        // The assert on byte_size(lo) will trigger if this assumption is violated.
        *lo = f;
        if (f.r_shift >= 32) { lo->r_bits = 0; lo->r_shift = 32; }
        if (f.g_shift >= 32) { lo->g_bits = 0; lo->g_shift = 32; }
        if (f.b_shift >= 32) { lo->b_bits = 0; lo->b_shift = 32; }
        if (f.a_shift >= 32) { lo->a_bits = 0; lo->a_shift = 32; }
        SkASSERT(byte_size(*lo) == 4);

        *hi = f;
        if (f.r_shift < 32) { hi->r_bits = 0; hi->r_shift = 32; } else { hi->r_shift -= 32; }
        if (f.g_shift < 32) { hi->g_bits = 0; hi->g_shift = 32; } else { hi->g_shift -= 32; }
        if (f.b_shift < 32) { hi->b_bits = 0; hi->b_shift = 32; } else { hi->b_shift -= 32; }
        if (f.a_shift < 32) { hi->a_bits = 0; hi->a_shift = 32; } else { hi->a_shift -= 32; }
        SkASSERT(byte_size(*hi) == 4);
    }

    // The only 16-byte format we support today is RGBA F32,
    // though, TODO, we could generalize that to any swizzle, and to allow UNORM too.
    static void assert_16byte_is_rgba_f32(PixelFormat f) {
    #if defined(SK_DEBUG)
        SkASSERT(byte_size(f) == 16);
        PixelFormat rgba_f32 = SkColorType_to_PixelFormat(kRGBA_F32_SkColorType);

        SkASSERT(f.encoding == rgba_f32.encoding);

        SkASSERT(f.r_bits == rgba_f32.r_bits);
        SkASSERT(f.g_bits == rgba_f32.g_bits);
        SkASSERT(f.b_bits == rgba_f32.b_bits);
        SkASSERT(f.a_bits == rgba_f32.a_bits);

        SkASSERT(f.r_shift == rgba_f32.r_shift);
        SkASSERT(f.g_shift == rgba_f32.g_shift);
        SkASSERT(f.b_shift == rgba_f32.b_shift);
        SkASSERT(f.a_shift == rgba_f32.a_shift);
    #endif
    }

    Color Builder::load(PixelFormat f, Ptr ptr) {
        switch (byte_size(f)) {
            case 1: return unpack(f, load8 (ptr));
            case 2: return unpack(f, load16(ptr));
            case 4: return unpack(f, load32(ptr));
            case 8: {
                PixelFormat lo,hi;
                split_disjoint_8byte_format(f, &lo,&hi);
                Color l = unpack(lo, load64(ptr, 0)),
                      h = unpack(hi, load64(ptr, 1));
                return {
                    lo.r_bits ? l.r : h.r,
                    lo.g_bits ? l.g : h.g,
                    lo.b_bits ? l.b : h.b,
                    lo.a_bits ? l.a : h.a,
                };
            }
            case 16: {
                assert_16byte_is_rgba_f32(f);
                return {
                    pun_to_F32(load128(ptr, 0)),
                    pun_to_F32(load128(ptr, 1)),
                    pun_to_F32(load128(ptr, 2)),
                    pun_to_F32(load128(ptr, 3)),
                };
            }
            default: SkUNREACHABLE;
        }
        return {};
    }

    Color Builder::gather(PixelFormat f, UPtr ptr, int offset, I32 index) {
        switch (byte_size(f)) {
            case 1: return unpack(f, gather8 (ptr, offset, index));
            case 2: return unpack(f, gather16(ptr, offset, index));
            case 4: return unpack(f, gather32(ptr, offset, index));
            case 8: {
                PixelFormat lo,hi;
                split_disjoint_8byte_format(f, &lo,&hi);
                Color l = unpack(lo, gather32(ptr, offset, (index<<1)+0)),
                      h = unpack(hi, gather32(ptr, offset, (index<<1)+1));
                return {
                    lo.r_bits ? l.r : h.r,
                    lo.g_bits ? l.g : h.g,
                    lo.b_bits ? l.b : h.b,
                    lo.a_bits ? l.a : h.a,
                };
            }
            case 16: {
                assert_16byte_is_rgba_f32(f);
                return {
                    gatherF(ptr, offset, (index<<2)+0),
                    gatherF(ptr, offset, (index<<2)+1),
                    gatherF(ptr, offset, (index<<2)+2),
                    gatherF(ptr, offset, (index<<2)+3),
                };
            }
            default: SkUNREACHABLE;
        }
        return {};
    }

    static I32 pack32(PixelFormat f, Color c) {
        SkASSERT(byte_size(f) <= 4);

        auto to_srgb = [](int bits, F32 v) {
            const skcms_TransferFunction* tf = skcms_sRGB_Inverse_TransferFunction();
            return to_unorm(bits, sk_program_transfer_fn(v, sRGBish_TF,
                                                         v->splat(tf->g),
                                                         v->splat(tf->a),
                                                         v->splat(tf->b),
                                                         v->splat(tf->c),
                                                         v->splat(tf->d),
                                                         v->splat(tf->e),
                                                         v->splat(tf->f)));
        };

        I32 packed = c->splat(0);
        auto pack_rgb = [&](F32 channel, int bits, int shift) {
            I32 encoded;
            switch (f.encoding) {
                case PixelFormat::UNORM: encoded = to_unorm(bits, channel); break;
                case PixelFormat:: SRGB: encoded = to_srgb (bits, channel); break;
                case PixelFormat::FLOAT: encoded = to_fp16 (      channel); break;
            }
            packed = pack(packed, encoded, shift);
        };
        auto pack_alpha = [&](F32 channel, int bits, int shift) {
            I32 encoded;
            switch (f.encoding) {
                case PixelFormat::UNORM:
                case PixelFormat:: SRGB: encoded = to_unorm(bits, channel); break;
                case PixelFormat::FLOAT: encoded = to_fp16 (      channel); break;
            }
            packed = pack(packed, encoded, shift);
        };
        if (f.r_bits) { pack_rgb  (c.r, f.r_bits, f.r_shift); }
        if (f.g_bits) { pack_rgb  (c.g, f.g_bits, f.g_shift); }
        if (f.b_bits) { pack_rgb  (c.b, f.b_bits, f.b_shift); }
        if (f.a_bits) { pack_alpha(c.a, f.a_bits, f.a_shift); }
        return packed;
    }

    void Builder::store(PixelFormat f, Ptr ptr, Color c) {
        // Detect a grayscale PixelFormat: r,g,b bit counts and shifts all equal.
        if (f.r_bits  == f.g_bits  && f.g_bits  == f.b_bits &&
            f.r_shift == f.g_shift && f.g_shift == f.b_shift) {

            // TODO: pull these coefficients from an SkColorSpace?  This is sRGB luma/luminance.
            c.r = c.r * 0.2126f
                + c.g * 0.7152f
                + c.b * 0.0722f;
            f.g_bits = f.b_bits = 0;
        }

        switch (byte_size(f)) {
            case 1: store8 (ptr, pack32(f,c)); break;
            case 2: store16(ptr, pack32(f,c)); break;
            case 4: store32(ptr, pack32(f,c)); break;
            case 8: {
                PixelFormat lo,hi;
                split_disjoint_8byte_format(f, &lo,&hi);
                store64(ptr, pack32(lo,c)
                           , pack32(hi,c));
                break;
            }
            case 16: {
                assert_16byte_is_rgba_f32(f);
                store128(ptr, pun_to_I32(c.r), pun_to_I32(c.g), pun_to_I32(c.b), pun_to_I32(c.a));
                break;
            }
            default: SkUNREACHABLE;
        }
    }

    void Builder::unpremul(F32* r, F32* g, F32* b, F32 a) {
        skvm::F32 invA = 1.0f / a,
                  inf  = pun_to_F32(splat(0x7f800000));
        // If a is 0, so are *r,*g,*b, so set invA to 0 to avoid 0*inf=NaN (instead 0*0 = 0).
        invA = select(invA < inf, invA
                                , 0.0f);
        *r *= invA;
        *g *= invA;
        *b *= invA;
    }

    void Builder::premul(F32* r, F32* g, F32* b, F32 a) {
        *r *= a;
        *g *= a;
        *b *= a;
    }

    Color Builder::uniformColor(SkColor4f color, Uniforms* uniforms) {
        auto [r,g,b,a] = color;
        return {
            uniformF(uniforms->pushF(r)),
            uniformF(uniforms->pushF(g)),
            uniformF(uniforms->pushF(b)),
            uniformF(uniforms->pushF(a)),
        };
    }

    F32 Builder::lerp(F32 lo, F32 hi, F32 t) {
        if (this->isImm(t.id, 0.0f)) { return lo; }
        if (this->isImm(t.id, 1.0f)) { return hi; }
        return mad(sub(hi, lo), t, lo);
    }

    Color Builder::lerp(Color lo, Color hi, F32 t) {
        return {
            lerp(lo.r, hi.r, t),
            lerp(lo.g, hi.g, t),
            lerp(lo.b, hi.b, t),
            lerp(lo.a, hi.a, t),
        };
    }

    HSLA Builder::to_hsla(Color c) {
        F32 mx = max(max(c.r,c.g),c.b),
            mn = min(min(c.r,c.g),c.b),
             d = mx - mn,
          invd = 1.0f / d,
        g_lt_b = select(c.g < c.b, splat(6.0f)
                                 , splat(0.0f));

        F32 h = (1/6.0f) * select(mx == mn,  0.0f,
                           select(mx == c.r, invd * (c.g - c.b) + g_lt_b,
                           select(mx == c.g, invd * (c.b - c.r) + 2.0f
                                           , invd * (c.r - c.g) + 4.0f)));

        F32 sum = mx + mn,
              l = sum * 0.5f,
              s = select(mx == mn, 0.0f
                                 , d / select(l > 0.5f, 2.0f - sum
                                                      , sum));
        return {h, s, l, c.a};
    }

    Color Builder::to_rgba(HSLA c) {
        // See GrRGBToHSLFilterEffect.fp

        auto [h,s,l,a] = c;
        F32 x = s * (1.0f - abs(l + l - 1.0f));

        auto hue_to_rgb = [&,l=l](auto hue) {
            auto q = abs(6.0f * fract(hue) - 3.0f) - 1.0f;
            return x * (clamp01(q) - 0.5f) + l;
        };

        return {
            hue_to_rgb(h + 0/3.0f),
            hue_to_rgb(h + 2/3.0f),
            hue_to_rgb(h + 1/3.0f),
            c.a,
        };
    }

    // We're basing our implementation of non-separable blend modes on
    //   https://www.w3.org/TR/compositing-1/#blendingnonseparable.
    // and
    //   https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
    // They're equivalent, but ES' math has been better simplified.
    //
    // Anything extra we add beyond that is to make the math work with premul inputs.

    static skvm::F32 saturation(skvm::F32 r, skvm::F32 g, skvm::F32 b) {
        return max(r, max(g, b))
             - min(r, min(g, b));
    }

    static skvm::F32 luminance(skvm::F32 r, skvm::F32 g, skvm::F32 b) {
        return r*0.30f + g*0.59f + b*0.11f;
    }

    static void set_sat(skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 s) {
        F32 mn  = min(*r, min(*g, *b)),
            mx  = max(*r, max(*g, *b)),
            sat = mx - mn;

        // Map min channel to 0, max channel to s, and scale the middle proportionally.
        auto scale = [&](skvm::F32 c) {
            auto scaled = ((c - mn) * s) / sat;
            return select(is_finite(scaled), scaled, 0.0f);
        };
        *r = scale(*r);
        *g = scale(*g);
        *b = scale(*b);
    }

    static void set_lum(skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 lu) {
        auto diff = lu - luminance(*r, *g, *b);
        *r += diff;
        *g += diff;
        *b += diff;
    }

    static void clip_color(skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32 a) {
        F32 mn  = min(*r, min(*g, *b)),
            mx  = max(*r, max(*g, *b)),
            lu = luminance(*r, *g, *b);

        auto clip = [&](auto c) {
            c = select(mn < 0 & lu != mn, lu + ((c-lu)*(  lu)) / (lu-mn), c);
            c = select(mx > a & lu != mx, lu + ((c-lu)*(a-lu)) / (mx-lu), c);
            return clamp01(c);  // May be a little negative, or worse, NaN.
        };
        *r = clip(*r);
        *g = clip(*g);
        *b = clip(*b);
    }

    Color Builder::blend(SkBlendMode mode, Color src, Color dst) {
        auto mma = [](skvm::F32 x, skvm::F32 y, skvm::F32 z, skvm::F32 w) {
            return x*y + z*w;
        };

        auto two = [](skvm::F32 x) { return x+x; };

        auto apply_rgba = [&](auto fn) {
            return Color {
                fn(src.r, dst.r),
                fn(src.g, dst.g),
                fn(src.b, dst.b),
                fn(src.a, dst.a),
            };
        };

        auto apply_rgb_srcover_a = [&](auto fn) {
            return Color {
                fn(src.r, dst.r),
                fn(src.g, dst.g),
                fn(src.b, dst.b),
                mad(dst.a, 1-src.a, src.a),   // srcover for alpha
            };
        };

        auto non_sep = [&](auto R, auto G, auto B) {
            return Color{
                R + mma(src.r, 1-dst.a,  dst.r, 1-src.a),
                G + mma(src.g, 1-dst.a,  dst.g, 1-src.a),
                B + mma(src.b, 1-dst.a,  dst.b, 1-src.a),
                mad(dst.a, 1-src.a, src.a),   // srcover for alpha
            };
        };

        switch (mode) {
            default:
                SkASSERT(false);
                [[fallthrough]]; /*but also, for safety, fallthrough*/

            case SkBlendMode::kClear: return { splat(0.0f), splat(0.0f), splat(0.0f), splat(0.0f) };

            case SkBlendMode::kSrc: return src;
            case SkBlendMode::kDst: return dst;

            case SkBlendMode::kDstOver: std::swap(src, dst); [[fallthrough]];
            case SkBlendMode::kSrcOver:
                return apply_rgba([&](auto s, auto d) {
                    return mad(d,1-src.a, s);
                });

            case SkBlendMode::kDstIn: std::swap(src, dst); [[fallthrough]];
            case SkBlendMode::kSrcIn:
                return apply_rgba([&](auto s, auto d) {
                    return s * dst.a;
                });

            case SkBlendMode::kDstOut: std::swap(src, dst); [[fallthrough]];

            case SkBlendMode::kSrcOut:
                return apply_rgba([&](auto s, auto d) {
                    return s * (1-dst.a);
                });

            case SkBlendMode::kDstATop: std::swap(src, dst); [[fallthrough]];
            case SkBlendMode::kSrcATop:
                return apply_rgba([&](auto s, auto d) {
                    return mma(s, dst.a,  d, 1-src.a);
                });

            case SkBlendMode::kXor:
                return apply_rgba([&](auto s, auto d) {
                    return mma(s, 1-dst.a,  d, 1-src.a);
                });

            case SkBlendMode::kPlus:
                return apply_rgba([&](auto s, auto d) {
                    return min(s+d, 1.0f);
                });

            case SkBlendMode::kModulate:
                return apply_rgba([&](auto s, auto d) {
                    return s * d;
                });

            case SkBlendMode::kScreen:
                // (s+d)-(s*d) gave us trouble with our "r,g,b <= after blending" asserts.
                // It's kind of plausible that s + (d - sd) keeps more precision?
                return apply_rgba([&](auto s, auto d) {
                    return s + (d - s*d);
                });

            case SkBlendMode::kDarken:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return s + (d - max(s * dst.a,
                                        d * src.a));
                });

            case SkBlendMode::kLighten:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return s + (d - min(s * dst.a,
                                        d * src.a));
                });

            case SkBlendMode::kDifference:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return s + (d - two(min(s * dst.a,
                                            d * src.a)));
                });

            case SkBlendMode::kExclusion:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return s + (d - two(s * d));
                });

            case SkBlendMode::kColorBurn:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    auto mn   = min(dst.a,
                                    src.a * (dst.a - d) / s),
                         burn = src.a * (dst.a - mn) + mma(s, 1-dst.a, d, 1-src.a);
                    return select(d == dst.a     , s * (1-dst.a) + d,
                           select(is_finite(burn), burn
                                                 , d * (1-src.a) + s));
                });

            case SkBlendMode::kColorDodge:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    auto dodge = src.a * min(dst.a,
                                             d * src.a / (src.a - s))
                                       + mma(s, 1-dst.a, d, 1-src.a);
                    return select(d == 0.0f       , s * (1-dst.a) + d,
                           select(is_finite(dodge), dodge
                                                  , d * (1-src.a) + s));
                });

            case SkBlendMode::kHardLight:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return mma(s, 1-dst.a, d, 1-src.a) +
                           select(two(s) <= src.a,
                                  two(s * d),
                                  src.a * dst.a - two((dst.a - d) * (src.a - s)));
                });

            case SkBlendMode::kOverlay:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    return mma(s, 1-dst.a, d, 1-src.a) +
                           select(two(d) <= dst.a,
                                  two(s * d),
                                  src.a * dst.a - two((dst.a - d) * (src.a - s)));
                });

            case SkBlendMode::kMultiply:
                return apply_rgba([&](auto s, auto d) {
                    return mma(s, 1-dst.a, d, 1-src.a) + s * d;
                });

            case SkBlendMode::kSoftLight:
                return apply_rgb_srcover_a([&](auto s, auto d) {
                    auto  m = select(dst.a > 0.0f, d / dst.a
                                                 , 0.0f),
                         s2 = two(s),
                         m4 = 4*m;

                         // The logic forks three ways:
                         //    1. dark src?
                         //    2. light src, dark dst?
                         //    3. light src, light dst?

                         // Used in case 1
                    auto darkSrc = d * ((s2-src.a) * (1-m) + src.a),
                         // Used in case 2
                         darkDst = (m4 * m4 + m4) * (m-1) + 7*m,
                         // Used in case 3.
                         liteDst = sqrt(m) - m,
                         // Used in 2 or 3?
                         liteSrc = dst.a * (s2 - src.a) * select(4*d <= dst.a, darkDst
                                                                             , liteDst)
                                   + d * src.a;
                    return s * (1-dst.a) + d * (1-src.a) + select(s2 <= src.a, darkSrc
                                                                             , liteSrc);
                });

            case SkBlendMode::kHue: {
                skvm::F32 R = src.r * src.a,
                          G = src.g * src.a,
                          B = src.b * src.a;

                set_sat   (&R, &G, &B, src.a * saturation(dst.r, dst.g, dst.b));
                set_lum   (&R, &G, &B, src.a * luminance (dst.r, dst.g, dst.b));
                clip_color(&R, &G, &B, src.a * dst.a);

                return non_sep(R, G, B);
            }

            case SkBlendMode::kSaturation: {
                skvm::F32 R = dst.r * src.a,
                          G = dst.g * src.a,
                          B = dst.b * src.a;

                set_sat   (&R, &G, &B, dst.a * saturation(src.r, src.g, src.b));
                set_lum   (&R, &G, &B, src.a * luminance (dst.r, dst.g, dst.b));
                clip_color(&R, &G, &B, src.a * dst.a);

                return non_sep(R, G, B);
            }

            case SkBlendMode::kColor: {
                skvm::F32 R = src.r * dst.a,
                          G = src.g * dst.a,
                          B = src.b * dst.a;

                set_lum   (&R, &G, &B, src.a * luminance(dst.r, dst.g, dst.b));
                clip_color(&R, &G, &B, src.a * dst.a);

                return non_sep(R, G, B);
            }

            case SkBlendMode::kLuminosity: {
                skvm::F32 R = dst.r * src.a,
                          G = dst.g * src.a,
                          B = dst.b * src.a;

                set_lum   (&R, &G, &B, dst.a * luminance(src.r, src.g, src.b));
                clip_color(&R, &G, &B, dst.a * src.a);

                return non_sep(R, G, B);
            }
        }
    }

    // ~~~~ Program::eval() and co. ~~~~ //

    void Program::eval(int n, void* args[]) const {
        SkOpts::interpret_skvm(fImpl->instructions.data(), (int)fImpl->instructions.size(),
                               this->nregs(), this->loop(), fImpl->strides.data(),
                               fImpl->traceHooks.data(), fImpl->traceHooks.size(),
                               this->nargs(), n, args);
    }

    bool Program::hasTraceHooks() const {
        // Identifies a program which has been instrumented for debugging.
        return !fImpl->traceHooks.empty();
    }

    Program::Program() : fImpl(std::make_unique<Impl>()) {}

    Program::~Program() {}

    Program::Program(Program&& other) : fImpl(std::move(other.fImpl)) {}

    Program& Program::operator=(Program&& other) {
        fImpl = std::move(other.fImpl);
        return *this;
    }

    Program::Program(const std::vector<OptimizedInstruction>& instructions,
                     const std::vector<int>& strides,
                     const std::vector<TraceHook*>& traceHooks) : Program() {
        fImpl->strides = strides;
        fImpl->traceHooks = traceHooks;

        this->setupInterpreter(instructions);
    }

    std::vector<InterpreterInstruction> Program::instructions() const { return fImpl->instructions; }
    int  Program::nargs() const { return (int)fImpl->strides.size(); }
    int  Program::nregs() const { return fImpl->regs; }
    int  Program::loop () const { return fImpl->loop; }
    bool Program::empty() const { return fImpl->instructions.empty(); }

    // Translate OptimizedInstructions to InterpreterInstructions.
    void Program::setupInterpreter(const std::vector<OptimizedInstruction>& instructions) {
        // Register each instruction is assigned to.
        std::vector<Reg> reg(instructions.size());

        // This next bit is a bit more complicated than strictly necessary;
        // we could just assign every instruction to its own register.
        //
        // But recycling registers is fairly cheap, and good practice for the
        // JITs where minimizing register pressure really is important.
        //
        // We have effectively infinite registers, so we hoist any value we can.
        // (The JIT may choose a more complex policy to reduce register pressure.)

        fImpl->regs = 0;
        std::vector<Reg> avail;

        // Assign this value to a register, recycling them where we can.
        auto assign_register = [&](Val id) {
            const OptimizedInstruction& inst = instructions[id];

            // If this is a real input and it's lifetime ends at this instruction,
            // we can recycle the register it's occupying.
            auto maybe_recycle_register = [&](Val input) {
                if (input != NA && instructions[input].death == id) {
                    avail.push_back(reg[input]);
                }
            };

            // Take care to not recycle the same register twice.
            const Val x = inst.x, y = inst.y, z = inst.z, w = inst.w;
            if (true                      ) { maybe_recycle_register(x); }
            if (y != x                    ) { maybe_recycle_register(y); }
            if (z != x && z != y          ) { maybe_recycle_register(z); }
            if (w != x && w != y && w != z) { maybe_recycle_register(w); }

            // Instructions that die at themselves (stores) don't need a register.
            if (inst.death != id) {
                // Allocate a register if we have to, preferring to reuse anything available.
                if (avail.empty()) {
                    reg[id] = fImpl->regs++;
                } else {
                    reg[id] = avail.back();
                    avail.pop_back();
                }
            }
        };

        // Assign a register to each hoisted instruction, then each non-hoisted loop instruction.
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            if ( instructions[id].can_hoist) { assign_register(id); }
        }
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            if (!instructions[id].can_hoist) { assign_register(id); }
        }

        // Translate OptimizedInstructions to InterpreterIstructions by mapping values to
        // registers.  This will be two passes, first hoisted instructions, then inside the loop.

        // The loop begins at the fImpl->loop'th Instruction.
        fImpl->loop = 0;
        fImpl->instructions.reserve(instructions.size());

        // Add a mapping for the N/A sentinel Val to any arbitrary register
        // so lookups don't have to know which arguments are used by which Ops.
        auto lookup_register = [&](Val id) {
            return id == NA ? (Reg)0
                            : reg[id];
        };

        auto push_instruction = [&](Val id, const OptimizedInstruction& inst) {
            InterpreterInstruction pinst{
                inst.op,
                lookup_register(id),
                lookup_register(inst.x),
                lookup_register(inst.y),
                lookup_register(inst.z),
                lookup_register(inst.w),
                inst.immA,
                inst.immB,
                inst.immC,
            };
            fImpl->instructions.push_back(pinst);
        };

        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const OptimizedInstruction& inst = instructions[id];
            if (inst.can_hoist) {
                push_instruction(id, inst);
                fImpl->loop++;
            }
        }
        for (Val id = 0; id < (Val)instructions.size(); id++) {
            const OptimizedInstruction& inst = instructions[id];
            if (!inst.can_hoist) {
                push_instruction(id, inst);
            }
        }
    }

}  // namespace skvm
