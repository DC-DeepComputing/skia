---
name: skia/document-in-band-sentinel-values
description: When assigning or returning in-band numeric sentinels like 0, -1, or negative numbers to signal absence, termination, or invalid/dead states, document their meaning with an explanatory comment
rubric: readability
severity: should_fix
---

**Rule.** When an in-band numeric value (such as 0, -1, or a negative value
assigned to an offset, index, or out-parameter) is used as a sentinel to
indicate termination, absence, or special states (such as dead or duplicate
entries), document the sentinel value and its meaning in an explanatory comment.
Explain why negative values are skipped or what state each sentinel designates
so readers and callers do not confuse them with valid 0-based positions or
quantities.

**Why.** In-band numeric values such as 0, -1, or negative numbers are easily
mistaken for valid 0-based indices, offsets, or sizes, or appear arbitrary and
mysterious without context. Adding a clarifying comment documents the protocol
or invariant, making it immediately clear to readers and callers why sentinel
values are assigned or skipped rather than representing valid targets.

## Examples

Bad:

```cpp
// Absence / termination sentinel:
if (allowTruncated) {
    *outNumEntries = dataSize / kSizeEntry;
    *outNextIfdOffset = 0;
    return true;
}

// Special / dead state sentinel:
instruction.kind = static_cast<InstructionFlags>(instruction.kind | InstructionFlags::kDead);
fToDisassembler[instruction.instructionIndex] = -1;
instruction.instructionIndex = -2;
```

Good:

```cpp
// Absence / termination sentinel:
if (allowTruncated) {
    *outNumEntries = dataSize / kSizeEntry;
    // Set next offset to 0 to indicate there is no next IFD.
    *outNextIfdOffset = 0;
    return true;
}

// Special / dead state sentinel:
instruction.kind = static_cast<InstructionFlags>(instruction.kind | InstructionFlags::kDead);
fToDisassembler[instruction.instructionIndex] = -1;
// Negative index indicates the instruction is duplicate or dead.
instruction.instructionIndex = -2;
```
