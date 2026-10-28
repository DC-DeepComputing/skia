---
name: skia/guard-debug-only-methods-with-sk-debug
description: When declaring or storing debug-only helpers or member fields, guard them with SkDEBUGCODE or cast unconditionally stored assertion-only members via (void) in constructors
rubric: correctness
severity: should_fix
---

**Rule.** Wrap helper methods, accessors, member variables, and tracking state
that exist solely to support debug assertions (such as SkASSERT or
SKGPU_ASSERT_SINGLE_OWNER) in SkDEBUGCODE(...) or #if defined(SK_DEBUG) rather
than exposing them unconditionally in release builds. For single-line debug-only
declarations, member variables, parameters, or statements, prefer the compact
SkDEBUGCODE(...) macro instead of multi-line #if defined(SK_DEBUG) blocks. If an
assertion-only member variable (such as SingleOwner* fSingleOwner) is declared
unconditionally or initialized in constructors, cast it via (void)fMember; in
the constructor body (or annotate with [[maybe_unused]]) to prevent
release-build unused-private-field errors.

**Why.** Exposing debug-only helper methods and member fields unconditionally
increases class layout size and memory overhead in release builds, causes
compiler warnings or -Werror build breaks for unused private fields when
assertion macros expand to no-ops, clutters class interfaces with dead code, and
risks production callers mistakenly depending on debug mechanisms. Wrapping
declarations in SkDEBUGCODE(...) elides them in release builds, while casting
unconditional members via (void) silences unused-field diagnostics without
preprocessor guards in constructor initializer lists.

## Examples

Bad:

```cpp
// Bad: debug-only validation helper compiled unconditionally in release builds
class DawnCommandBuffer {
    bool hasActivePassEncoder() const {
        return fActiveRenderPassEncoder || fActiveComputePassEncoder;
    }
};

// Bad: debug-only tracking fields bloat object layout size in release builds
class DrawWriter {
    BindBufferInfo fStatic;
    BindBufferInfo fIndices;
};

// Bad: assertion-only member causes unused-private-field compilation error in release builds
DawnResourceProvider::DawnResourceProvider(SharedContext* sc, SingleOwner* so)
        : ResourceProvider(sc, so), fSingleOwner(so) {
    fManager = std::make_unique<Manager>(this);
}
```

Good:

```cpp
// Good: multi-line debug helper conditionally compiled under SK_DEBUG
class DawnCommandBuffer {
#if defined(SK_DEBUG)
    bool hasActivePassEncoder() const {
        return fActiveRenderPassEncoder || fActiveComputePassEncoder;
    }
#endif
};

// Good: single-line debug member variables elided with SkDEBUGCODE
class DrawWriter {
    SkDEBUGCODE(BindBufferInfo fStatic;)
    SkDEBUGCODE(BindBufferInfo fIndices;)
};

// Good: suppress release-build unused-field warnings with (void) in constructor
DawnResourceProvider::DawnResourceProvider(SharedContext* sc, SingleOwner* so)
        : ResourceProvider(sc, so), fSingleOwner(so) {
    fManager = std::make_unique<Manager>(this);
    // Only used for debug asserts so this avoids compile errors.
    (void)fSingleOwner;
}
```
