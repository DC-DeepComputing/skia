/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/disassembler/SkVMDisassembler.h"

namespace disassembler {
void Disassembler::dump() {

    dumpHead();

    for (auto id = 0ul; id < fInstructions.size(); ++id) {
        dumpInstruction(id);
    }

    dumpTail();
}

void Disassembler::markAsDeadCode(std::vector<bool>& live, const std::vector<int>& newIds) {

    for (size_t id = 0ul; id < fInstructions.size(); ++id) {
        auto& instruction = fInstructions[id];
        if (instruction.instructionIndex < 0) {
            continue;
        }
        SkASSERT(instruction.instructionIndex >= 0);
        SkASSERT(instruction.instructionIndex < (int)live.size());
        if (live[instruction.instructionIndex]) {
            for (int* arg : {&instruction.x, &instruction.y, &instruction.z, &instruction.w}) {
                if (*arg >= 0) {
                    *arg = newIds[*arg];
                    SkASSERT(*arg >= 0);
                }
            }
            instruction.instructionIndex = newIds[instruction.instructionIndex];
            fToDisassembler[instruction.instructionIndex] = id;
        } else {
            instruction.kind |= Kind::dead;
            fToDisassembler[instruction.instructionIndex] = -1;
            instruction.instructionIndex = -2;
        }
    }
}

void Disassembler::formatVV(const char* op, int v1, int v2) const {
    fOutput->writeText("%s %s, %s", op, V(v1).c_str(), V(v2).c_str());
}

void Disassembler::formatPV(const char* op, int imm, int v1) const {
    fOutput->writeText("%s Ptr%d, %s", op, imm, V(v1).c_str());
}
void Disassembler::formatPVV(const char* op, int imm, int v1, int v2) const {
    fOutput->writeText("%s Ptr%d, %s, %s", op, imm, V(v1).c_str(), V(v2).c_str());
}
void Disassembler::formatPVVVV(const char* op, int imm, int v1, int v2, int v3, int v4) const {
    fOutput->writeText("%s Ptr%d, %s, %s, %s, %s", op, imm, V(v1).c_str(), V(v2).c_str(), V(v3).c_str(), V(v4).c_str());
}

void Disassembler::formatA_(int id, const char* op) const {
    fOutput->writeText("%s = %s", V(id).c_str(), op);
}

void Disassembler::formatA_P(int id, const char* op, int imm) const {
    fOutput->writeText("%s = %s Ptr%d", V(id).c_str(), op, imm);
}

void Disassembler::formatA_PH(int id, const char* op, int immA, int immB) const {
    fOutput->writeText("%s = %s Ptr%d, %h", V(id).c_str(), op, immA, immB);
}

void Disassembler::formatA_PHH(int id, const char* op, int immA, int immB, int immC) const {
    fOutput->writeText("%s = %s Ptr%d, %h, %h", V(id).c_str(), op, immA, immB, immC);
}

void Disassembler::formatA_PHV(int id, const char* op, int immA, int immB, int v) const {
    fOutput->writeText("%s = %s Ptr%d, %h, V%d", V(id).c_str(), op, immA, immB, V(v).c_str());
}

void Disassembler::formatA_S(int id, const char* op, int imm) const {
    float f;
    memcpy(&f, &imm, 4);
    char buffer[kSkStrAppendScalar_MaxSize];
    char* stop = SkStrAppendScalar(buffer, f);

    fOutput->writeText("%s = %s %x (", V(id).c_str(), op, imm);

    fOutput->write(buffer, stop - buffer);
    fOutput->writeText(")");
}

void Disassembler::formatA_V(int id, const char* op, int v) const {
    fOutput->writeText("%s = %s %s", V(id).c_str(), op, V(v).c_str());
}

void Disassembler::formatA_VV(int id, const char* op, int v1, int v2) const {
    fOutput->writeText("%s = %s %s, %s", V(id).c_str(), op, V(v1).c_str(), V(v2).c_str());
}

void Disassembler::formatA_VVV(int id, const char* op,  int v1, int v2, int v3) const {
    fOutput->writeText("%s = %s %s, %s, %s", V(id).c_str(), op, V(v1).c_str(), V(v2).c_str(), V(v3).c_str());
}

void Disassembler::formatA_VC(int id, const char* op,  int v, int imm) const {
    fOutput->writeText("%s = %s %s, %d", V(id).c_str(), op, V(v).c_str(), imm);
}

void Disassembler::dumpInstruction(int id0) const {

    auto& instruction = fInstructions[id0];
    const int id = instruction.instructionIndex;
    const int x = instruction.x,
              y = instruction.y,
              z = instruction.z,
              w = instruction.w;
    const int immA = instruction.immA,
              immB = instruction.immB,
              immC = instruction.immC;

    if (instruction.op == SkVMOp::trace_line) {
        if (fLastLine == immA) return;
        fLastLine = immA;
        SkASSERT(immA > 0 && immA <= (int)fLines.size());
        SkString mask;
        mask.printf(immC == 1 ? "%s(-1)" : "%s", V(x).c_str());
        fOutput->writeText("<tr class='source'><td class='mask'>mask=%s</td><td colspan=2>// %s</td></tr>", mask.c_str(), fLines[immA - 1].c_str());
        return;
    } else if (instruction.op == SkVMOp::trace_var) {
        return;
    } else if (instruction.op == SkVMOp::trace_call) {
        SkASSERT(immA > 0 && immA <= (int)fLines.size());
        auto call =  immB == 1 ? "&#8618;" : "&#8617;";
        SkString mask;
        mask.printf(immC == 1 ? "%s(-1)" : "%s", V(x).c_str());
        fOutput->writeText("<tr class='source'><td class='mask'>mask%s%s</td><td colspan=2>// %s</td></tr>", call, mask.c_str(), fLines[immA - 1].c_str());
        return;
    }
    if ((instruction.kind & (Kind::dead | Kind::constant)) == 0) {
        fLastLine = -1;
    }
    // No label, to the operation
    SkString label("&nbsp;&nbsp;&nbsp;&nbsp;");
    if ((instruction.kind & Kind::hoisted) != 0) {
        label.set("&#8593;&#8593;&#8593; ");
    }
    if (instruction.duplicates > 0) {
        SkString dups;
        dups.printf("*%d", instruction.duplicates + 1);
        label.append(dups);
    } else {
        label.append("&nbsp;&nbsp;&nbsp;");
    }
    auto classes = instruction.classes();
    fOutput->writeText("<tr class='%s'><td>%s</td><td>", classes.c_str(), label.c_str());

    // Operation
    switch (instruction.op) {
        case SkVMOp::no_op:                                                         break;
        case SkVMOp::trace_line:
        case SkVMOp::trace_var:
        case SkVMOp::trace_call:    SkASSERT(false);                                break;
        case SkVMOp::assert_true:   formatVV("assert_true", x, y);                  break;
        case SkVMOp::store8:        formatPV("store8", immA, x);                    break;
        case SkVMOp::store16:       formatPV("store16", immA, x);                   break;
        case SkVMOp::store32:       formatPV("store32", immA, x);                   break;
        case SkVMOp::store64:       formatPVV("store64", immA, x, y);               break;
        case SkVMOp::store128:      formatPVVVV("store128", immA, x, y, z, w);      break;
        case SkVMOp::index:         formatA_(id, "index");                          break;
        case SkVMOp::load8:         formatA_P(id, "load8", immA);                   break;
        case SkVMOp::load16:        formatA_P(id, "load16", immA);                  break;
        case SkVMOp::load32:        formatA_P(id, "load32", immA);                  break;
        case SkVMOp::load64:        formatA_PH(id, "load64", immA, immB);           break;
        case SkVMOp::load128:       formatA_PH(id, "load128", immA, immB);          break;
        case SkVMOp::gather8:       formatA_PHV(id, "gather8", immA, immB, x);      break;
        case SkVMOp::gather16:      formatA_PHV(id, "gather16", immA, immB, x);     break;
        case SkVMOp::gather32:      formatA_PHV(id, "gather32", immA, immB, x);     break;
        case SkVMOp::uniform32:     formatA_PH(id, "uniform32", immA, immB);        break;
        case SkVMOp::array32:       formatA_PHH(id, "array32", immA, immB, immC);   break;
        case SkVMOp::splat:         formatA_S(id, "splat", immA);                   break;
        case SkVMOp:: add_f32:      formatA_VV(id, "add_f32", x, y);                break;
        case SkVMOp:: sub_f32:      formatA_VV(id, "sub_f32", x, y);                break;
        case SkVMOp:: mul_f32:      formatA_VV(id, "mul_f32", x, y);                break;
        case SkVMOp:: div_f32:      formatA_VV(id, "div_f32", x, y);                break;
        case SkVMOp:: min_f32:      formatA_VV(id, "min_f32", x, y);                break;
        case SkVMOp:: max_f32:      formatA_VV(id, "max_f32", x, y);                break;
        case SkVMOp:: fma_f32:      formatA_VVV(id, "fma_f32", x, y, z);            break;
        case SkVMOp:: fms_f32:      formatA_VVV(id, "fms_f32", x, y, z);            break;
        case SkVMOp::fnma_f32:      formatA_VVV(id, "fnma_f32", x, y, z);           break;
        case SkVMOp::sqrt_f32:      formatA_V(id, "sqrt_f32", x);                   break;
        case SkVMOp:: eq_f32:       formatA_VV(id, "eq_f32", x, y);                 break;
        case SkVMOp::neq_f32:       formatA_VV(id, "neq_f32", x, y);                break;
        case SkVMOp:: gt_f32:       formatA_VV(id, "gt_f32", x, y);                 break;
        case SkVMOp::gte_f32:       formatA_VV(id, "gte_f32", x, y);                break;
        case SkVMOp::add_i32:       formatA_VV(id, "add_i32", x, y);                break;
        case SkVMOp::sub_i32:       formatA_VV(id, "sub_i32", x, y);                break;
        case SkVMOp::mul_i32:       formatA_VV(id, "mul_i32", x, y);                break;
        case SkVMOp::shl_i32:       formatA_VC(id, "shl_i32", x, immA);             break;
        case SkVMOp::shr_i32:       formatA_VC(id, "shr_i32", x, immA);             break;
        case SkVMOp::sra_i32:       formatA_VC(id, "sra_i32", x, immA);             break;
        case SkVMOp::eq_i32:        formatA_VV(id, "eq_i32", x, y);                 break;
        case SkVMOp::gt_i32:        formatA_VV(id, "gt_i32", x, y);                 break;
        case SkVMOp::bit_and:       formatA_VV(id, "bit_and", x, y);                break;
        case SkVMOp::bit_or:        formatA_VV(id, "bit_or", x, y);                 break;
        case SkVMOp::bit_xor:       formatA_VV(id, "bit_xor", x, y);                break;
        case SkVMOp::bit_clear:     formatA_VV(id, "bit_clear", x, y);              break;
        case SkVMOp::select:        formatA_VVV(id, "select", x, y, z);             break;
        case SkVMOp::ceil:          formatA_V(id, "ceil", x);                       break;
        case SkVMOp::floor:         formatA_V(id, "floor", x);                      break;
        case SkVMOp::to_f32:        formatA_V(id, "to_f32", x);                     break;
        case SkVMOp::to_fp16:       formatA_V(id, "to_fp16", x);                    break;
        case SkVMOp::from_fp16:     formatA_V(id, "from_fp16", x);                  break;
        case SkVMOp::trunc:         formatA_V(id, "trunc", x);                      break;
        case SkVMOp::round:         formatA_V(id, "round", x);                      break;
        case SkVMOp::extract:      formatA_VC(id, "extract", x, immA);              break;
        default: SkASSERT(false);
    }
    // Comments
    if (instruction.comments.empty()) {
        fOutput->writeText("</td></tr>\n");
    } else {
        const char* prefix = "</td><td>";
        for (auto& comment : instruction.comments) {
            fOutput->writeText(prefix);
            fOutput->writeText(this->getText(comment));
            fOutput->writeText("</td></tr>\n");
            prefix = "<tr><td/><td/><td>";
        }
    }
    // Generation
    for (auto& machine : instruction.generation) {
        fOutput->writeText("<tr><td>%s</td><td>%s</td><td>%s</td></tr>\n", machine.label.c_str(), machine.command.c_str(), machine.operands.c_str());
    }
}
void dumpCommandBlock(const char* label, const char* comments = nullptr);
void dumpCommand(const char* command, const char* operands, const char* comments = nullptr);

void Disassembler::dumpHead() const {
fOutput->writeText(
    "<HTML>\n"
    "<HEAD>\n"
    "   <TITLE>SkVM Disassembler Output</TITLE>\n"
    "   <STYLE>\n"
    "   table {\n"
    "     text-align: left;\n"
    "   }\n"
    "   table th {\n"
    "      background-color: lightgray;\n"
    "   }\n"
    "   .dead {\n"
    "      color: lightgray;\n"
    "      text-decoration: line-through;\n"
    "   }\n"
    "   .constant {\n"
    "      color: lightgray;\n"
    "   }\n"
    "   .normal {\n"
    "   }\n"
    "   .deduped {\n"
    "      font-weight: bold;\n"
    "   }\n"
    "   .source {\n"
    "       color: darkblue;\n"
    "   }\n"
    "   .mask {\n"
    "       color: green;\n"
    "   }\n"
    "   .comments {\n"
    "   }\n"
    "   </STYLE>\n"
    "</HEAD>\n"
    "</BODY>\n"
    "      <table border = \"0\"  style='font-family:\"monospace\"; font-size: 10px;'>\n"
    "         <tr>\n"
    "            <th style=\"min-width:50px\"><u>Kind</u></th>\n"
    "            <th style=\"width:40%;\"><u>Command</u></th>\n"
    "            <th style=\"width:60%;\"><u>Comments</u></th>\n"
    "         </tr>");
}
void Disassembler::dumpTail() const {
    fOutput->writeText(
    "      </table>\n"
            "</BODY>\n"
            "</HTML>"
    );
}

} // namespace disassembler
