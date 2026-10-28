---
name: skia/use-symmetric-names-for-bracketed-operations
description: When naming bracketed or paired operations (such as macros, functions, or lifecycle hooks), use symmetric counterpart names like SETUP/TEARDOWN rather than mismatched action verbs
rubric: readability
severity: should_fix
---

**Rule.** When defining complementary or bracketed operations—such as macros,
functions, or lifecycle hooks that prepare state before an action and clean up
or record state afterward—give them symmetric, paired counterpart names (e.g.,
SETUP/TEARDOWN, BEGIN/END, ENTER/EXIT, SAVE/RESTORE). Do not pair mismatched or
asymmetric verbs (such as pairing RESTORE with SAVE, or SETUP with FINISH) that
obscure their coupling and make lifecycle usage error-prone.

**Why.** Complementary operations acting as a bracket around an action form a
strict lifecycle contract. Asymmetric or mismatched names obscure their
coupling, confuse call sites about the expected sequence, and increase the risk
of unbalanced or improperly paired invocations.

## Examples

Bad:

```cpp
// Asymmetric naming obscures that these macros form a paired bracket.
#define RESTORE_BLITTER_TRACE \
    SKVM_BLITTER_TRACE_NAME.reset(); \
    RP_BLITTER_TRACE_NAME.reset()

#define SAVE_BLITTER_CAPTURE \
    SKVM_BLITTER_TRACE_CAPTURE = SKVM_BLITTER_TRACE_NAME; \
    RP_BLITTER_TRACE_CAPTURE = RP_BLITTER_TRACE_NAME

// Call site:
RESTORE_BLITTER_TRACE;
bench->draw(loops, canvas);
SAVE_BLITTER_CAPTURE;
```

Good:

```cpp
// Symmetric SETUP and TEARDOWN pair clearly signals bracketed lifecycle usage.
#define SK_BLITTER_TRACE_LOCAL_SETUP \
    gSkVMBlitterTrace.reset(); \
    gSkRPBlitterTrace.reset()

#define SK_BLITTER_TRACE_LOCAL_TEARDOWN \
    gSkVMBlitterTraceCapture = gSkVMBlitterTrace; \
    gSkRPBlitterTraceCapture = gSkRPBlitterTrace

// Call site:
SK_BLITTER_TRACE_LOCAL_SETUP;
bench->draw(loops, canvas);
SK_BLITTER_TRACE_LOCAL_TEARDOWN;
```
