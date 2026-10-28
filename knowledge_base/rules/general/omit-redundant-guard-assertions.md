---
name: skia/omit-redundant-guard-assertions
description: When asserting invariants with SkASSERT or SkASSERT_RELEASE, omit redundant checks already guaranteed by enclosing guards, prior early exits, preceding helper postconditions, initializers, or assignments
rubric: readability
severity: should_fix
---

**Rule.** Do not add redundant SkASSERT, SkASSERTF, or SkASSERT_RELEASE checks
for conditions already guaranteed by enclosing if statements, prior runtime
guards with early returns, immediately preceding helper function calls or
postconditions, unconditional member initializer dereferences in constructors,
or unconditional assignments within the same block. Furthermore, do not
duplicate SkASSERT with a redundant SkASSERT_RELEASE on the exact same
condition; retain only the appropriate assertion level, and replace temporary
debugging TODO comments on permanently retained release asserts with explanatory
comments.

**Why.** Asserting conditions already established by preceding early exits,
enclosing guards, preceding helper calls, unconditional assignments, prior
execution, or companion assertions adds visual clutter and dead checks without
providing safety or debugging value, and can mislead readers into questioning
whether intervening logic compromised the invariant. Because SkASSERT_RELEASE
already executes in debug builds, duplicating it with SkASSERT on the exact same
condition is redundant, while temporary debugging TODOs obscure why a release
assertion is intentionally retained in production.

## Examples

Bad:

```cpp
// Redundant assertion inside enclosing guard:
if (supportCpuUpload) {
    if (state.fUseTransferBuffer) {
        SkASSERT(supportCpuUpload);
        doUpload();
    }
}

// Redundant assertion following early-return check:
if (!stream) {
    *result = kInvalidInput;
    return nullptr;
}
SkASSERT_RELEASE(stream);

// Redundant assertion after helper postcondition or assignment:
injectMoveToIfNeeded();
if (!this->getLastPt(&lastPt) || lastPt != mappedPts[0]) {
    SkASSERT(fLastMoveToIndex >= 0);
    this->lineTo(mappedPts[0]);
}
fContextOptions.fOptionsPriv = &fOptionsPriv;
SkASSERT(fContextOptions.fOptionsPriv == &fOptionsPriv);

// Redundant assertion after member initializer dereference:
AtlasManager::AtlasManager(Recorder* recorder)
        : fRecorder(recorder), fAtlas(recorder->priv().caps()) {
    SkASSERT(recorder);
}

// Redundant duplicate assertion with temporary debugging TODO:
SkASSERT(step->performsShading());
// TODO(b/372912880): Release assert debugging for illegal instruction occurring in the wild.
SkASSERTF_RELEASE(step->performsShading(), "step: %s", step->name());
```

Good:

```cpp
// Rely on enclosing guard:
if (supportCpuUpload) {
    if (state.fUseTransferBuffer) {
        doUpload();
    }
}

// Rely on prior early-return check:
if (!stream) {
    *result = kInvalidInput;
    return nullptr;
}

// Rely on preceding helper postcondition or explicit assignment:
injectMoveToIfNeeded();
if (!this->getLastPt(&lastPt) || lastPt != mappedPts[0]) {
    this->lineTo(mappedPts[0]);
}
fContextOptions.fOptionsPriv = &fOptionsPriv;

// Omit assertion when already dereferenced in initializer list:
AtlasManager::AtlasManager(Recorder* recorder)
        : fRecorder(recorder), fAtlas(recorder->priv().caps()) {}

// Keep only the necessary assertion; document permanently retained release asserts:
SkASSERT(step->performsShading());
// Check for unexpected corruption occurring in the wild.
SkASSERTF_RELEASE(fRoots.size() >= 2, "root size = %zu", fRoots.size());
```
