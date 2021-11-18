/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkVMVisualizer_DEFINED
#define SkVMVisualizer_DEFINED
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkStream.h"
#include "src/core/SkOpts.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/SkTHash.h"
#include <vector>
#include <unordered_map>
class SkWStream;
namespace skvm {
    enum class SkVMOp {
        assert_true,
        trace_line,
        trace_var,
        trace_call,
        store8,
        store16,
        store32,
        store64,
        store128,
        load8,
        load16,
        load32,
        load64,
        load128,
        index,
        gather8,
        gather16,
        gather32,
        uniform32,
        array32,
        splat,
        add_f32,
        add_i32,
        sub_f32,
        sub_i32,
        mul_f32,
        mul_i32,
        div_f32,
        min_f32,
        max_f32,
        fma_f32,
        fms_f32,
        fnma_f32,
        sqrt_f32,
        shl_i32,
        shr_i32,
        sra_i32,
        ceil,
        floor,
        trunc,
        round,
        to_fp16,
        from_fp16,
        to_f32,
        neq_f32,
        eq_f32,
        eq_i32,
        gte_f32,
        gt_f32,
        gt_i32,
        bit_and,
        bit_or,
        bit_xor,
        bit_clear,
        select,
        neq_i32,
        gte_i32,
        extract,
        no_op,
    };
    enum class MachineOp {};
    struct pos {
        int line = -1;
        int pos = -1;
    };
    struct coord {
        pos start;
        pos end;
    };
    struct MachineCommand {
        size_t address;
        SkString label;
        SkString command;
        SkString extra;
    };
    enum Kind {
        normal = 0x00,
        hoisted = 0x01,
        constant = 0x02,
        dead = 0x04,
    };
} // disassembler
namespace sknonstd {
template <> struct is_bitmask_enum<skvm::Kind> : std::true_type {};
}  // namespace sknonstd

namespace skvm {
    struct SkVMInstruction {
        Kind kind = Kind::normal;
        SkVMOp op;
        coord source;
        size_t startCode = 0;
        size_t endCode = 0;
        int instructionIndex; // index in the actual instructions list
        int duplicates = 0;
        std::vector<int> comments;
        int x, y, z, w;
        int immA, immB, immC;
        bool operator == (const SkVMInstruction& o) const {
            return this->kind == o.kind &&
                   this->op == o.op &&
                   this->instructionIndex == o.instructionIndex &&
                   this->x == o.x && this->y == o.y && this->z == o.z && this->w == o.w &&
                   this->immA == o.immA && this->immB == o.immB && this->immC == o.immC;
        }
        SkString classes() const {
            SkString result("normal");
            if ((kind & Kind::dead) != 0) result = "dead";
            if ((kind & Kind::constant) != 0) result = "constant";
            if (duplicates > 0) result += " deduped";
            return result;
        }
    };
    struct SkVMInstructionHash {
        uint32_t mix(uint32_t hash, uint32_t data) const {
            hash += data;
            hash += (hash << 10);
            hash ^= (hash >> 6);
            return hash;
        }
        uint32_t operator()(const SkVMInstruction& i) const {
            uint32_t hash = 0;
            hash = mix(hash, SkGoodHash()(i.kind));
            hash = mix(hash, SkGoodHash()(i.op));
            hash = mix(hash, SkGoodHash()(i.instructionIndex));
            hash = mix(hash, SkGoodHash()(i.x));
            hash = mix(hash, SkGoodHash()(i.y));
            hash = mix(hash, SkGoodHash()(i.z));
            hash = mix(hash, SkGoodHash()(i.immA));
            hash = mix(hash, SkGoodHash()(i.immB));
            hash = mix(hash, SkGoodHash()(i.immC));
            return hash;
        }
    };
    class SkVMVisualizer {
        class SkDebugfStream final : public SkFILEWStream {
        public:
            SkDebugfStream(const char* path) : SkFILEWStream(path) { }
            bool writeText(const char* format, ...) {
                SkString message;
                va_list argp;
                va_start(argp, format);
                message.appendVAList(format, argp);
                va_end(argp);
                return SkFILEWStream::writeText(message.c_str());
            }
        };
    public:
        explicit SkVMVisualizer(const char* source) : fOutput(nullptr) {
            SkStrSplit(source, "\n", kStrict_SkStrSplitMode, &fLines);
        }
        ~SkVMVisualizer() = default;
        void dump(const char* path, const char* code);
        void markAsDeadCode(std::vector<bool>& live, const std::vector<int>& newIds);
        void markAsHoisted(int id) {
            auto found = fToDisassembler[id];
            auto& instruction = fInstructions[found];
            instruction.kind |= Kind::hoisted;
        }
        void addInstruction(SkVMInstruction skvm) {
            if (skvm.op < SkVMOp::store8 || skvm.op > SkVMOp::load128) {
                if (auto found = fIndex.find(skvm)) {
                    auto& instruction = fInstructions[*found];
                    ++(instruction.duplicates);
                    return;
                }
            }
            fIndex.set(skvm, fInstructions.size());
            fToDisassembler.emplace(skvm.instructionIndex, fInstructions.size());
            fInstructions.emplace_back(std::move(skvm));
        }
        void addMachineCommands(int id, size_t start, size_t end);
        void dumpComment(SkString comment);
        int addComment(SkString comment) {
            auto index = fComments.size();
            fComments.emplace_back(std::move(comment));
            return index;
        }
        void finishComment(size_t index, int comment) {
            for (auto i = index; i < fInstructions.size(); ++i) {
                auto& instruction = fInstructions[i];
                instruction.comments.emplace_back(comment);
            }
        }
        int lastInstruction() const { return fInstructions.size(); }
        SkString V(int reg) const {
            if (reg == -1) {
                return SkString("{optimized}");
            } else if (reg == -2) {
                return SkString("{dead code}");
            }
            SkString result;
            result.printf("v%d", reg);
            return result;
        }
    private:
        void dumpInstruction(int id) const;
        void dumpCommandBlock(const char* label, const char* comments = nullptr);
        void dumpCommand(const char* command, const char* operands, const char* comments = nullptr);
        void dumpHead() const;
        void dumpTail() const;
        void formatVV(const char* op, int v1, int v2) const;
        void formatPV(const char* op, int imm, int v1) const;
        void formatPVV(const char* op, int imm, int v1, int v2) const;
        void formatPVVVV(const char* op, int imm, int v1, int v2, int v3, int v4) const;
        void formatA_(int id, const char* op) const;
        void formatA_P(int id, const char* op, int imm) const;
        void formatA_PH(int id, const char* op, int immA, int immB) const;
        void formatA_PHH(int id, const char* op, int immA, int immB, int immC) const;
        void formatA_PHV(int id, const char* op, int immA, int immB, int v) const;
        void formatA_S(int id, const char* op, int imm) const;
        void formatA_V(int id, const char* op, int v) const;
        void formatA_VV(int id, const char* op, int v1, int v2) const;
        void formatA_VVV(int id, const char* op,  int v1, int v2, int v3) const;
        void formatA_VC(int id, const char* op,  int v, int imm) const;
        const char* getText(size_t comment) const {
            SkASSERT(comment < fComments.size());
            return fComments[comment].c_str();
        }
        SkTHashMap<SkVMInstruction, size_t, SkVMInstructionHash> fIndex;
        std::vector<SkVMInstruction> fInstructions;
        std::vector<SkString> fComments;
        SkDebugfStream* fOutput;
        std::unordered_map<int, size_t> fToDisassembler;
        SkTArray<SkString> fLines;
        mutable int fLastLine = -1;
        SkTArray<MachineCommand> fAsm;
        mutable size_t fAsmLine = 0;
        size_t fAsmStart = 0;
    };
} // namespace skvm
#endif // SkVMVisualizer_DEFINED
