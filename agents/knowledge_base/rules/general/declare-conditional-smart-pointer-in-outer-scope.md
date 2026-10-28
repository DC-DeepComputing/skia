---
name: skia/declare-conditional-smart-pointer-in-outer-scope
description: When borrowing raw pointers or references from conditionally created objects accessed downstream in outer scopes, declare owning smart pointers in the outer scope
rubric: safety
severity: should_fix
---

**Rule.** When an object is conditionally created or allocated within an `if`,
`else`, or nested block but borrowed raw pointers or references to it are
accessed downstream in the outer scope, declare the owning smart pointer
(`sk_sp`, `std::unique_ptr`) in the outer scope (initialized to `nullptr`) and
instantiate it inside the branch. Never declare the owning container or stack
object inside the branch when raw pointers borrowed from it escape the block.

**Why.** If an owning smart pointer or local object is declared inside a branch
or nested block, it is destroyed when exiting that block. Any raw pointers or
references borrowed from it and accessed later in the outer scope become
dangling, resulting in use-after-free crashes, memory corruption, or garbage
data.

## Examples

Bad:

```cpp
// Bad: tempDebugTrace is destroyed at the end of the else-if block,
// leaving debugTrace dangling when accessed downstream in dump().
if (debugTrace) {
    compileProgram(debugTrace, /*writeTraceOps=*/true);
} else if (kRPEnableLiveTrace) {
    sk_sp<SkSL::DebugTracePriv> tempDebugTrace = SkSL::DebugTraceImpl::Make();
    debugTrace = tempDebugTrace.get();
    compileProgram(debugTrace, /*writeTraceOps=*/false);
}
if (kRPEnableLiveTrace && fRPProgram) {
    fRPProgram->dump(&stream); // Use-after-free: debugTrace is dangling
}
```

Good:

```cpp
// Good: declare owning smart pointer in outer scope to extend its lifetime.
sk_sp<SkSL::DebugTracePriv> tempDebugTrace = nullptr;
if (debugTrace) {
    compileProgram(debugTrace, /*writeTraceOps=*/true);
} else if (kRPEnableLiveTrace) {
    tempDebugTrace = SkSL::DebugTraceImpl::Make();
    debugTrace = tempDebugTrace.get();
    compileProgram(debugTrace, /*writeTraceOps=*/false);
}
if (kRPEnableLiveTrace && fRPProgram) {
    fRPProgram->dump(&stream); // Safe: tempDebugTrace keeps object alive
}
```
