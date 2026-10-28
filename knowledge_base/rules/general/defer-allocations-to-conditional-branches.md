---
name: skia/defer-allocations-to-conditional-branches
description: When allocating heap objects, fallback buffers, or debug tracers needed only for rare or conditional paths, defer instantiation inside the branch that requires them
rubric: safety
severity: should_fix
---

**Rule.** Do not unconditionally allocate heap objects, fallback buffers, or
diagnostic tracing instances outside conditional blocks when they are only
needed along rare, optional, or debugging paths. Declare an empty handle or
pointer upfront if needed across scopes, and defer instantiation inside the
conditional branch that actually requires the object.

**Why.** Eagerly allocating heap objects outside conditional checks imposes
unnecessary allocator overhead and cache pollution on common or
performance-critical paths, even when debugging, tracing, or fallback branches
are inactive. Instantiating objects lazily inside the branches that consume them
preserves fast-path performance.

## Examples

Bad:

```cpp
// Bad: Eagerly allocating a heap tracing object outside conditional checks
sk_sp<SkSL::DebugTracePriv> tempDebugTrace = SkSL::MakeDebugTrace();
if (debugTrace) {
    program = compile(programToUse, debugTrace, /*writeTraceOps=*/true);
} else if (kRPEnableLiveTrace) {
    debugTrace = tempDebugTrace.get();
    program = compile(programToUse, debugTrace, /*writeTraceOps=*/false);
}
```

Good:

```cpp
// Good: Defer allocation until the specific branch is taken
sk_sp<SkSL::DebugTracePriv> tempDebugTrace = nullptr;
if (debugTrace) {
    program = compile(programToUse, debugTrace, /*writeTraceOps=*/true);
} else {
#if SKSL_ENABLE_LIVE_TRACE
    tempDebugTrace = SkSL::DebugTraceImpl::Make();
    debugTrace = tempDebugTrace.get();
#endif
    program = compile(programToUse, debugTrace, /*writeTraceOps=*/false);
}
```
