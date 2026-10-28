---
name: skia/api/avoid-active-verbs-for-declaration-macros
description: When defining macros that declare or initialize variables rather than starting execution, use _INIT or _SETUP with SK_ prefix instead of START_ or BEGIN_
rubric: readability
severity: should_fix
---

**Rule.** Do not name preprocessor macros with active runtime verbs (such as
START_* or BEGIN_*) when they only declare externs or initialize static
variables rather than initiating runtime execution. Use descriptive noun or
state suffixes such as *_INIT or *_SETUP instead, and prefix non-file-scoped
macros with SK_ (or GR_).

**Why.** Action verbs like START_ or BEGIN_ imply initiating an active
operation, timer, or execution scope, misleading callers about when runtime
actions take place and what the macro executes. Suffixes like _INIT or _SETUP
accurately communicate passive declaration or one-time initialization, while the
SK_ prefix avoids collisions across header boundaries.

## Examples

Bad:

```cpp
// Bad: Implies starting an active runtime session, but only declares variables.
#define START_BLITTER_TRACE \
    extern SkBlitterTrace gSkVMBlitterTrace; \
    static SkBlitterTrace gSkVMBlitterTraceCapture("VM", false);
```

Good:

```cpp
// Good: Accurately reflects passive declaration/initialization and uses SK_ prefix.
#define SK_BLITTER_TRACE_INIT \
    extern SkBlitterTrace gSkVMBlitterTrace; \
    static SkBlitterTrace gSkVMBlitterTraceCapture("VM", false);
```
