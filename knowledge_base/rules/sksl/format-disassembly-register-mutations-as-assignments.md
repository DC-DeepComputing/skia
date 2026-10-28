---
name: skia/sksl/format-disassembly-register-mutations-as-assignments
description: When formatting instruction disassembly or IR dumps, format register-mutating operations as explicit pseudo-code assignments rather than bare function calls
rubric: readability
severity: nit
---

**Rule.** In IR and instruction disassemblers or dumpers, format operations that
mutate implicit registers or scratch slots as explicit pseudo-code assignments
(e.g., `src.rgba = op(src.rgba)`) rather than bare function calls (e.g.,
`op()`). Explicitly display the target and source registers or memory slots to
make dataflow and register effects transparent.

**Why.** Bare function names in disassembly dumps conceal implicit inputs and
outputs (such as `src.rgba` or execution masks), making program state changes
difficult to track. Explicit pseudo-code assignments preserve visual continuity
across the dumper and ensure dataflow is immediately understandable in test
expectations and debug dumps.

## Examples

Bad:

```cpp
opText = "toLinearSrgb()";
```

Good:

```cpp
opText = opArg1 + " = toLinearSrgb(" + opArg1 + ")";
```
