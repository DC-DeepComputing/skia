---
name: skia/prefer-debug-asserts-for-prevalidated-inputs
description: In internal helpers, constructors, and routines, assert caller-validated preconditions, non-null pointers, and descriptor consistency using debug SkASSERT
rubric: correctness
severity: should_fix
---

**Rule.** In internal constructors, resource creators, and helper methods
handling pre-validated inputs, descriptor flags, or geometry, assert all
preconditions (such as matrix invertibility, non-degeneracy, or non-null
pointers) and configuration consistency using debug SkASSERT rather than
omitting checks or asserting a partial subset. When accessing optional,
backend-specific, or conditionally initialized configuration fields or pointers,
assert that required capabilities are non-null (e.g., SkASSERT(caps.field !=
nullptr)) or explicitly guard against the unsupported state. When accepting
pre-queried tokens, filters, or descriptor flags, assert that arguments match
validator queries and agree with backend capabilities. Do not rely solely on
upstream validation, and do not use SkASSERT_RELEASE or redundant runtime checks
when inputs are already sanitized.

**Why.** Internal constructors and callee helpers can gain new call sites over
time that bypass factory checks, pass mismatched tokens, or provide degenerate
inputs; callee-side debug assertions document class contracts, prevent
regressions, and catch invalid states early during development. Because boundary
entry points already validate untrusted input, debug assertions (SkASSERT)
enforce invariants and capability consistency uniformly without incurring
production overhead or runtime bloat.

## Examples

Bad:

```cpp
// Bad: Relies on 'm' being invertible without asserting caller-enforced precondition
bool SkRectPriv::QuadContainsRect(const SkM44& m, const SkRect& a, const SkRect& b) {
    if (a.isEmpty()) { return false; }
    // ...
}

// Bad: Accessing conditionally initialized backend caps fields without asserting preconditions
const Symbol* sym = context.fSymbolTable->find(shaderCaps.fFloatBufferArrayName);
SkASSERT(sym->is<FieldSymbol>());

// Bad: Omitting precondition checks or capability consistency on descriptor flags
bool DrawContext::recordUpload(Recorder* recorder, const UploadSource& source) {
    return fPendingUploads->recordUpload(recorder, source);
}

// Bad: Using release asserts in internal helpers when input is pre-validated
SkASSERT_RELEASE((unsigned)verb < sizeof(gPtsPerVerb));
```

Good:

```cpp
// Good: Explicitly assert caller-guaranteed precondition such as matrix invertibility
bool SkRectPriv::QuadContainsRect(const SkM44& m, const SkRect& a, const SkRect& b) {
    SkDEBUGCODE(SkM44 inverse;)
    SkASSERT(m.invert(&inverse));
    if (a.isEmpty()) { return false; }
    // ...
}

// Good: Assert backend caps precondition before dereferencing or querying
SkASSERT(shaderCaps.fFloatBufferArrayName != nullptr);
const Symbol* sym = context.fSymbolTable->find(shaderCaps.fFloatBufferArrayName);
SkASSERT(sym->is<FieldSymbol>());

// Good: Callee debug assertions guard preconditions and matching descriptor flags
bool DrawContext::recordUpload(Recorder* recorder, const UploadSource& source) {
    SkASSERT(source.isValid());
    return fPendingUploads->recordUpload(recorder, source);
}

// Good: Debug assertion documents invariant without production overhead
SkASSERT((unsigned)verb < sizeof(gPtsPerVerb));
```
