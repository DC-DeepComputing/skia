/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkVMVisualizer_DEFINED
#define SkVMVisualizer_DEFINED
#include <unordered_map>
#include <vector>
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkBitmaskEnum.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkTHash.h"
#include "src/core/SkOpts.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/tracing/SkVMDebugTrace.h"

namespace skvm {
    enum InstructionFlags : uint8_t {
        kNormal = 0x00,
        kHoisted = 0x01,
        kDead = 0x02,
    };

    struct MachineCommand {
        size_t address;
        SkString label;
        SkString command;
        SkString extra;
    };

    struct SkVMInstruction {
        InstructionFlags kind = InstructionFlags::kNormal;
        skvm::Op op;
        // Machine commands range (for dissasembling):
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
                   this->startCode == o.startCode &&
                   this->endCode == o.endCode &&
                   this->instructionIndex == o.instructionIndex &&
                   this->x == o.x && this->y == o.y && this->z == o.z && this->w == o.w &&
                   this->immA == o.immA && this->immB == o.immB && this->immC == o.immC;
        }
        SkString classes() const {

            SkString result((kind & InstructionFlags::kDead) ? "dead" : "normal");
            if (duplicates > 0) result += " origin";
            if (duplicates < 0) result += " deduped";
            return result;
        }
    };
    struct SkVMInstructionHash {
        uint32_t operator()(const SkVMInstruction& i) const {
            uint32_t hash = 0;
            hash = SkOpts::hash_fn(&i.kind, sizeof(i.kind), hash);
            hash = SkOpts::hash_fn(&i.op, sizeof(i.op), hash);
            hash = SkOpts::hash_fn(&i.instructionIndex, sizeof(i.instructionIndex), hash);
            hash = SkOpts::hash_fn(&i.x, sizeof(i.x), hash);
            hash = SkOpts::hash_fn(&i.y, sizeof(i.y), hash);
            hash = SkOpts::hash_fn(&i.z, sizeof(i.z), hash);
            hash = SkOpts::hash_fn(&i.immA, sizeof(i.immA), hash);
            hash = SkOpts::hash_fn(&i.immB, sizeof(i.immB), hash);
            hash = SkOpts::hash_fn(&i.immC, sizeof(i.immC), hash);
            return hash;
        }
    };
    class SkVMVisualizer {
    public:
        explicit SkVMVisualizer(SkSL::SkVMDebugTrace* debugInfo)
                : fDebugInfo(debugInfo), fOutput(nullptr) {}
        ~SkVMVisualizer() = default;
        void dump(SkWStream* output, const char* code);
        void markAsDeadCode(std::vector<bool>& live, const std::vector<int>& newIds);
        void markAsHoisted(int id) {
            auto found = fToDisassembler[id];
            auto& instruction = fInstructions[found];
            instruction.kind =
                    static_cast<InstructionFlags>(instruction.kind | InstructionFlags::kHoisted);
        }
        void markAsDuplicate(int origin, int id) {
            ++fInstructions[origin].duplicates;
        }
        void addInstruction(SkVMInstruction skvm) {
            if (!touches_varying_memory(skvm.op)) {
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
        SkString V(int reg) const {
            if (reg == -1) {
                return SkString("{optimized}");
            } else if (reg == -2) {
                return SkString("{dead code}");
            } else {
                return SkStringPrintf("v%d", reg);
            }
        }
    private:
        void parseDisassembler(SkWStream* output, const char* code);
        void dumpInstruction(int id) const;
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

        void writeText(const char* format, ...) const {
            SkString message;
            va_list argp;
            va_start(argp, format);
            message.appendVAList(format, argp);
            va_end(argp);
            fOutput->writeText(message.c_str());
        }

        SkSL::SkVMDebugTrace* fDebugInfo;
        SkTHashMap<SkVMInstruction, size_t, SkVMInstructionHash> fIndex;
        std::vector<SkVMInstruction> fInstructions;
        SkWStream* fOutput;
        std::unordered_map<int, size_t> fToDisassembler;
        SkTArray<MachineCommand> fAsm;
        mutable size_t fAsmLine = 0;
        size_t fAsmStart = 0;
        size_t fAsmEnd = 0;
    };
} // namespace skvm

namespace sknonstd {
template <> struct is_bitmask_enum<skvm::InstructionFlags> : std::true_type {};
}  // namespace sknonstd

#endif // SkVMVisualizer_DEFINED
