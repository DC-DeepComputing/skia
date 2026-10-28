---
name: skia/document-input-structure-in-parsing-routines
description: When implementing parsing or string-scanning routines using sentinel markers or delimiters, document the expected input structure and skipped sections rather than restating code
rubric: readability
severity: nit
---

**Rule.** When implementing parsing or string-scanning routines that rely on
sentinel markers, delimiters, or section headers, document the expected input
format and explain why specific markers are sought. Clarify which sections (such
as headers, preludes, or tail loops) are skipped or processed, rather than
merely writing comments that restate the code operation.

**Why.** Ad-hoc token and string parsing logic scanning for sentinel markers is
fragile and opaque to future maintainers unless the input structure and parser
assumptions are explicit. Documenting why specific delimiters are targeted and
what sections are skipped prevents maintainers from breaking parsing logic when
input formats change or during refactoring.

## Examples

Bad:

```cpp
// Read the disassembled code
SkTArray<SkString> commands;
SkStrSplit(code, "\n", kStrict_SkStrSplitMode, &commands);
for (const SkString& line : commands) {
    ++fAsmLine;
    if (line.find("<_skvm_jit>") >= 0) {
        break;
    }
}
```

Good:

```cpp
// Read the disassembled code from <_skvm_jit> until the last command
// attached to the byte code. We skip the prelude (main loop organization),
// process the vector loop, and skip the trailing scalar loop.
SkTArray<SkString> commands;
SkStrSplit(code, "\n", kStrict_SkStrSplitMode, &commands);
for (const SkString& line : commands) {
    ++fAsmLine;
    if (line.find("<_skvm_jit>") >= 0) {
        break;
    }
}
```
