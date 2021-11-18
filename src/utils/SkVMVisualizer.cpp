/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/utils/SkVMVisualizer.h"
#include <sstream>
#include "src/core/SkStreamPriv.h"

namespace skvm {

void SkVMVisualizer::parseDisassembler(SkWStream* output, const char* code) {
    fOutput = output == nullptr ? new SkDebugfStream() : output;
    if (code == nullptr) {
        fAsmLine = 0;
        return;
    }
        // Read the disassembled code
    SkTArray<SkString> commands;
    SkStrSplit(code, "\n", kStrict_SkStrSplitMode, &commands);
    for (auto& line : commands) {
        ++fAsmLine;
        if (line.find("<_skvm_jit>") >= 0) {
            break;
        }
    }

    if (fAsmLine < commands.size()) {
        auto& line = commands[fAsmLine];
        SkTArray<SkString> tokens;
        SkStrSplit(line.c_str(), ":#", kStrict_SkStrSplitMode, &tokens);
        if (tokens.size() >= 2) {
            std::istringstream ss(tokens[0].c_str());
            ss >> std::hex >> fAsmStart;
        }
    }
    auto asmEnd = fAsmStart + fInstructions.back().endCode;
    for (auto i = fAsmLine; i < commands.size(); ++i) {
        auto& line = commands[i];
        SkTArray<SkString> tokens;
        SkStrSplit(line.c_str(), ":#", kStrict_SkStrSplitMode, &tokens);
        size_t addr = 0;
        if (tokens.size() >= 2) {
            std::istringstream ss(tokens[0].c_str());
            ss >> std::hex >> addr;
        }
        if (addr > asmEnd) {
            break;
        }
        addr -= fAsmStart;
        if (!fAsm.empty()) {
            auto& prev = fAsm.back();
            int len = addr - prev.address;
            prev.command.remove(0, 1 + len * 3);
            if (prev.command.isEmpty()) {
                prev.command.printf("{ align %d bytes }", len);
            }
        }
        fAsm.push_back({addr,
                        tokens[0], tokens[1], tokens.size() == 3 ? tokens[2] : SkString()});
    }
    if (!fAsm.empty()) {

        auto& prev = fAsm.back();
        int len = fInstructions.back().endCode - prev.address;
        prev.command.remove(0, 1 + len * 3);
        if (prev.command.isEmpty()) {
            prev.command.printf("{ align %d bytes }", len);
        }
    }
    fAsmLine = 0;
}

void SkVMVisualizer::dump(SkWStream* output, const char* code) {
    parseDisassembler(output, code);
    dumpHead();
    for (auto id = 0ul; id < fInstructions.size(); ++id) {
        dumpInstruction(id);
    }
    dumpTail();
}
void SkVMVisualizer::markAsDeadCode(std::vector<bool>& live, const std::vector<int>& newIds) {
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
            instruction.kind
                    = static_cast<InstructionFlags>(instruction.kind | InstructionFlags::kDead);
            fToDisassembler[instruction.instructionIndex] = -1;
            instruction.instructionIndex = -2;
        }
    }
}

void SkVMVisualizer::addMachineCommands(int id, size_t start, size_t end) {
    auto found = fToDisassembler[id];
    auto& instruction = fInstructions[found];
    instruction.startCode = start;
    instruction.endCode = end;
}

void SkVMVisualizer::formatVV(const char* op, int v1, int v2) const {
    writeText("%s %s, %s", op, V(v1).c_str(), V(v2).c_str());
}
void SkVMVisualizer::formatPV(const char* op, int imm, int v1) const {
    writeText("%s Ptr%d, %s", op, imm, V(v1).c_str());
}
void SkVMVisualizer::formatPVV(const char* op, int imm, int v1, int v2) const {
    writeText("%s Ptr%d, %s, %s", op, imm, V(v1).c_str(), V(v2).c_str());
}
void SkVMVisualizer::formatPVVVV(const char* op, int imm, int v1, int v2, int v3, int v4) const {
    writeText("%s Ptr%d, %s, %s, %s, %s",
              op, imm, V(v1).c_str(), V(v2).c_str(), V(v3).c_str(), V(v4).c_str());
}
void SkVMVisualizer::formatA_(int id, const char* op) const {
    writeText("%s = %s", V(id).c_str(), op);
}
void SkVMVisualizer::formatA_P(int id, const char* op, int imm) const {
    writeText("%s = %s Ptr%d", V(id).c_str(), op, imm);
}
void SkVMVisualizer::formatA_PH(int id, const char* op, int immA, int immB) const {
    writeText("%s = %s Ptr%d, %x", V(id).c_str(), op, immA, immB);
}
void SkVMVisualizer::formatA_PHH(int id, const char* op, int immA, int immB, int immC) const {
    writeText("%s = %s Ptr%d, %x, %x", V(id).c_str(), op, immA, immB, immC);
}
void SkVMVisualizer::formatA_PHV(int id, const char* op, int immA, int immB, int v) const {
    writeText("%s = %s Ptr%d, %x, V%d", V(id).c_str(), op, immA, immB, V(v).c_str());
}
void SkVMVisualizer::formatA_S(int id, const char* op, int imm) const {
    float f;
    memcpy(&f, &imm, 4);
    char buffer[kSkStrAppendScalar_MaxSize];
    char* stop = SkStrAppendScalar(buffer, f);
    writeText("%s = %s %x (", V(id).c_str(), op, imm);
    fOutput->write(buffer, stop - buffer);
    writeText(")");
}
void SkVMVisualizer::formatA_V(int id, const char* op, int v) const {
    writeText("%s = %s %s", V(id).c_str(), op, V(v).c_str());
}
void SkVMVisualizer::formatA_VV(int id, const char* op, int v1, int v2) const {
    writeText("%s = %s %s, %s", V(id).c_str(), op, V(v1).c_str(), V(v2).c_str());
}
void SkVMVisualizer::formatA_VVV(int id, const char* op,  int v1, int v2, int v3) const {
    writeText("%s = %s %s, %s, %s", V(id).c_str(), op, V(v1).c_str(), V(v2).c_str(), V(v3).c_str());
}
void SkVMVisualizer::formatA_VC(int id, const char* op,  int v, int imm) const {
    writeText("%s = %s %s, %d", V(id).c_str(), op, V(v).c_str(), imm);
}
void SkVMVisualizer::dumpInstruction(int id0) const {
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
        SkASSERT(fDebugInfo != nullptr);
        SkASSERT(immA > 0 && immA <= (int)fDebugInfo->fSource.size());
        writeText("<tr class='source'><td class='mask'></td><td colspan=2>// %s</td></tr>\n",
                  fDebugInfo->fSource[immA - 1].c_str());
        return;
    } else if (instruction.op == SkVMOp::trace_var) {
        return;
    } else if (instruction.op == SkVMOp::trace_enter) {
        SkASSERT(fDebugInfo != nullptr);
        SkASSERT(immA >= 0 && immA <= (int)fDebugInfo->fFuncInfo.size());
        std::string& func = fDebugInfo->fFuncInfo[immA].name;
        SkString mask;
        mask.printf(immC == 1 ? "%s(-1)" : "%s", V(x).c_str());
        writeText("<tr class='source'><td class='mask'>&#8618;%s</td><td colspan=2>%s</td></tr>\n",
                  mask.c_str(), func.c_str());
        return;
    } else if (instruction.op == SkVMOp::trace_exit) {
        SkASSERT(fDebugInfo != nullptr);
        SkASSERT(immA >= 0 && immA <= (int)fDebugInfo->fFuncInfo.size());
        std::string& func = fDebugInfo->fFuncInfo[immA].name;        SkString mask;
        mask.printf(immC == 1 ? "%s(-1)" : "%s", V(x).c_str());
        writeText("<tr class='source'><td class='mask'>&#8617;%s</td><td colspan=2>%s</td></tr>\n",
                  mask.c_str(), func.c_str());
        return;
    }
    // No label, to the operation
    SkString label("");
    if ((instruction.kind & InstructionFlags::kHoisted) != 0) {
        label.set("&#8593;&#8593;&#8593; ");
    }
    if (instruction.duplicates > 0) {
        SkString dups;
        dups.printf("*%d", instruction.duplicates + 1);
        label.append(dups);
    } else {
        label.append("");
    }
    auto classes = instruction.classes();
    writeText("<tr class='%s'><td>%s</td><td>", classes.c_str(), label.c_str());
    // Operation
    switch (instruction.op) {
        case SkVMOp::no_op:                                                         break;
        case SkVMOp::trace_line:
        case SkVMOp::trace_var:
        case SkVMOp::trace_enter:
        case SkVMOp::trace_exit:    SkASSERT(false);                                break;
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
        default: SkASSERT(false);
    }
    // Generation
    if ((instruction.kind & (InstructionFlags::kDead | InstructionFlags::kHoisted)) == 0) {
        while (fAsmLine < fAsm.size() && fAsm[fAsmLine].address != instruction.startCode) {
            ++fAsmLine;
        }
        while (fAsmLine < fAsm.size() && fAsm[fAsmLine].address < instruction.endCode) {
            auto line = fAsm[fAsmLine];
            writeText("</td></tr>\n<tr class='machine'><td>%s</td><td colspan='2'>%s",
                      line.label.c_str(), line.command.c_str());
            ++fAsmLine;
        }
    }
    writeText("</td></tr>\n");
}

void SkVMVisualizer::dumpHead() const {
writeText(
    "<HTML>\n"
    "<HEAD>\n"
    "   <TITLE>SkVM Disassembler Output</TITLE>\n"
    "   <STYLE>\n"
    "   button { border-style: none; font-size: 10px; background-color: lightpink; }\n"
    "   table { text-align: left; }\n"
    "   table th { background-color: lightgray; }\n"
    "   .dead, .dead1 { color: lightgray; text-decoration: line-through; }\n"
    "   .normal, .normal1 { }\n"
    "   .deduped, .deduped1 { font-weight: bold; }\n"
    "   .source, .source1 { color: darkblue; }\n"
    "   .mask, .mask1 { color: green; }\n"
    "   .comments, .comments1 { }\n"
    "   .machine, .machine1 { color: lightblue; }\n"
    "   </STYLE>\n"
    "    <script>\n"
    "    function initializeButton(className) {\n"
    "      var btn = document.getElementById(className);\n"
    "      var elems = document.getElementsByClassName(className);\n"
    "      if (elems == undefined || elems.length == 0) {\n"
    "        btn.disabled = true;\n"
    "        btn.innerText = \"None\";\n"
    "        btn.style.background = \"lightgray\";\n"
    "        return;\n"
    "      }\n"
    "    };\n"
    "    function initialize() {\n"
    "      initializeButton('normal');\n"
    "      initializeButton('source');\n"
    "      initializeButton('dead');\n"
    "      initializeButton('machine');\n"
    "    };\n"
    "  </script>\n"
    "</HEAD>\n"
    "<BODY onload='initialize();'>\n"
    "    <script>\n"
    "    function toggle(btn, className) {\n"
    "      var elems = document.getElementsByClassName(className);\n"
    "      for (var i = 0; i < elems.length; i++) {\n"
    "        var elem = elems.item(i);\n"
    "        if (elem.style.display === \"\") {\n"
    "            elem.style.display = \"none\";\n"
    "            btn.innerText = \"Show\";\n"
    "            btn.style.background = \"lightcyan\";\n"
    "        } else {\n"
    "            elem.style.display = \"\";\n"
    "            btn.innerText = \"Hide\";\n"
    "            btn.style.background = \"lightpink\";\n"
    "        }\n"
    "      }\n"
    "    };\n"
    "    </script>"
    "    <table border=\"0\" style='font-family:\"monospace\"; font-size: 10px;'>\n"
    "     <caption style='font-family:Roboto; font-size:15px; text-align:left;'>Legend</caption>\n"
    "     <tr>\n"
    "        <th style=\"min-width:100px;\"><u>Kind</u></th>\n"
    "        <th style=\"width:35%;\"><u>Example</u></th>\n"
    "        <th style=\"width: 5%; min-width:50px;\"><u></u></th>\n"
    "        <th style=\"width:60%;\"><u>Description</u></th>\n"
    "     </tr>\n"
    "      <tr class='normal1'>"
            "<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>"
            "<td>v1 = load32 Ptr1</td>"
            "<td><button id='normal' onclick=\"toggle(this, 'normal')\">Hide</button></td>"
            "<td>A regular SkVM command</td></tr>\n"
    "      <tr class='normal1 deduped'><td>*{N}</td>"
            "<td>v9 = gt_f32 v0, v1</td><td></td>"
            "<td>A {N} times deduped SkVM command</td></tr>\n"
    "      <tr class='normal1'><td>&#8593;&#8593;&#8593; &nbsp;&nbsp;&nbsp;</td>"
            "<td>v22 = splat 3f800000 (1)</td><td></td>"
            "<td>A hoisted SkVM command</td></tr>\n"
    "      <tr class='source1'><td class='mask'>mask&#8618;v{N}(-1)</td>"
            "<td>// C++ source line</td><td></td>"
            "<td>Enter into the procedure with mask v{N} (which has a constant value -1)"
            "</td></tr>\n"
    "      <tr class='source1'><td class='mask'>mask&#8617;v{N}</td>"
            "<td>// C++ source line</td><td>"
            "</td><td>Exit the procedure with mask v{N}</td></tr>\n"
    "      <tr class='source1'><td class='mask'></td><td>// C++ source line</td>"
            "<td><button id='source' onclick=\"toggle(this, 'source')\">Hide</button></td>"
            "<td>Line trace back to C++ code</td></tr>\n"
    "      <tr class='dead1'><td></td><td>{dead code} = mul_f32 v1, v18</td>"
            "<td><button id='dead' onclick=\"toggle(this, 'dead')\">Hide</button></td>"
            "<td>An eliminated \"dead code\" SkVM command</td></tr>\n"
    "      <tr class='machine1'><td>{address}</td><td>vmovups (%rsi),%ymm0</td>"
            "<td><button id='machine' onclick=\"toggle(this, 'machine')\">Hide</button></td>"
            "<td>A disassembled machine command generated by SkVM command</td></tr>\n"
    "    </table>\n"
    "    <table border = \"0\"style='font-family:\"monospace\"; font-size: 10px;'>\n"
    "     <caption style='font-family:Roboto;font-size:15px;text-align:left;'>SkVM Code</caption>\n"
    "     <tr>\n"
    "        <th style=\"min-width:100px;\"><u>Kind</u></th>\n"
    "        <th style=\"width:40%;min-width:100px;\"><u>Command</u></th>\n"
    "        <th style=\"width:60%;\"><u>Comments</u></th>\n"
    "     </tr>");
}
void SkVMVisualizer::dumpTail() const {
    writeText(
    "      </table>\n"
            "</BODY>\n"
            "</HTML>"
    );
}
} // namespace skvm
