/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/tracing/SkVMDebugTracePlayer.h"

namespace SkSL {

void SkVMDebugTracePlayer::reset(sk_sp<SkVMDebugTrace> debugTrace) {
    fDebugTrace = debugTrace;
    fCursor = 0;
    fSlots.clear();
    fSlots.resize(debugTrace->fSlotInfo.size());
    fStack.clear();
    fStack.push_back({/*fFunction=*/-1,
                      /*fLine=*/-1,
                      /*fDisplayMask=*/SkBitSet(fDebugTrace->fSlotInfo.size())});
}

void SkVMDebugTracePlayer::step() {
    while (!this->hasTraceCompleted()) {
        if (this->execute(fCursor++)) {
            break;
        }
    }
}

void SkVMDebugTracePlayer::stepOver() {
    size_t initialStackDepth = fStack.size();
    while (!this->hasTraceCompleted()) {
        if (this->execute(fCursor++) && fStack.size() <= initialStackDepth) {
            break;
        }
    }
}

bool SkVMDebugTracePlayer::hasTraceCompleted() const {
    return !fDebugTrace || fCursor >= fDebugTrace->fTraceInfo.size();
}

int32_t SkVMDebugTracePlayer::getCurrentLine() const {
    SkASSERT(!fStack.empty());
    return fStack.back().fLine;
}

std::vector<int> SkVMDebugTracePlayer::getCallStack() const {
    if (fStack.empty()) {
        return {};
    }
    std::vector<int> funcs;
    funcs.reserve(fStack.size() - 1);
    for (size_t index = 1; index < fStack.size(); ++index) {
        funcs.push_back(fStack[index].fFunction);
    }
    return funcs;
}

std::vector<SkVMDebugTracePlayer::VariableData> SkVMDebugTracePlayer::getVariablesForDisplayMask(
        const SkBitSet& bits) const {
    SkASSERT(bits.size() == fSlots.size());

    std::vector<VariableData> vars;
    bits.forEachSetIndex([&](int slot) {
        vars.push_back({slot, fSlots[slot]});
    });
    return vars;
}

std::vector<SkVMDebugTracePlayer::VariableData> SkVMDebugTracePlayer::getLocalVariables(
        int stackFrameIndex) const {
    // The first entry on the stack is the "global" frame before we enter main, so offset our index
    // by one to account for it.
    ++stackFrameIndex;
    if (stackFrameIndex <= 0 || (size_t)stackFrameIndex >= fStack.size()) {
        SkDEBUGFAILF("stack frame %d doesn't exist", stackFrameIndex - 1);
        return {};
    }
    return this->getVariablesForDisplayMask(fStack[stackFrameIndex].fDisplayMask);
}

std::vector<SkVMDebugTracePlayer::VariableData> SkVMDebugTracePlayer::getGlobalVariables() const {
    if (fStack.empty()) {
        return {};
    }
    return this->getVariablesForDisplayMask(fStack.front().fDisplayMask);
}

bool SkVMDebugTracePlayer::execute(size_t position) {
    if (position >= fDebugTrace->fTraceInfo.size()) {
        SkDEBUGFAILF("position %zu out of range", position);
        return true;
    }

    const SkVMTraceInfo& trace = fDebugTrace->fTraceInfo[position];
    switch (trace.op) {
        case SkVMTraceInfo::Op::kLine:  // data: line number, (unused)
            SkASSERT(!fStack.empty());
            SkASSERT(trace.data[0] >= 0);
            SkASSERT((size_t)trace.data[0] < fDebugTrace->fSource.size());
            fStack.back().fLine = trace.data[0];
            return true;

        case SkVMTraceInfo::Op::kVar:  // data: slot, value
            SkASSERT(trace.data[0] >= 0);
            SkASSERT((size_t)trace.data[0] < fDebugTrace->fSlotInfo.size());
            fSlots[trace.data[0]] = trace.data[1];
            fStack.back().fDisplayMask.set(trace.data[0]);
            break;

        case SkVMTraceInfo::Op::kEnter:  // data: function index, (unused)
            SkASSERT(trace.data[0] >= 0);
            SkASSERT((size_t)trace.data[0] < fDebugTrace->fFuncInfo.size());
            fStack.push_back({/*fFunction=*/trace.data[0],
                              /*fLine=*/-1,
                              /*fDisplayMask=*/SkBitSet(fDebugTrace->fSlotInfo.size())});
            break;

        case SkVMTraceInfo::Op::kExit:  // data: function index, (unused)
            SkASSERT(!fStack.empty());
            SkASSERT(fStack.back().fFunction == trace.data[0]);
            fStack.pop_back();
            return true;
    }

    return false;
}

}  // namespace SkSL
