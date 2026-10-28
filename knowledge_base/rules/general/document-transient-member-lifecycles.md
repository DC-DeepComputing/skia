---
name: skia/document-transient-member-lifecycles
description: When declaring member variables, scratch containers, or staging buffers holding transient state, document their scope and clearing points instead of ambiguous phrases like 'volatile' or 'populated anew'
rubric: readability
severity: nit
---

**Rule.** When declaring struct or class member variables, CPU staging buffers,
dedicated caches, or scratch tracking containers holding transient intermediate
state rather than persistent object invariants—such as pass-specific bounds,
single-upload buffers, scope-keyed cache entries, or AST/codegen tracking
sets—document their lifecycle, scope boundary, and retention/clearing semantics
in an explanatory comment. Explicitly specify the exact operation or lifecycle
event that initializes, clears, or resets them (such as when an enclosing
record, pass, or helper function ends), state when their values are undefined or
reset, and describe short-lived entries precisely rather than using ambiguous
phrasing like 'volatile' (which implies mutating data) or 'populated anew'
(which leaves clearing timing ambiguous).

**Why.** Member variables, internal buffers, and caches are generally assumed to
maintain persistent invariants across operations. Documenting transient
lifecycles and explicit reset points prevents maintainers from assuming data
persists, reading stale state, or introducing reentrancy bugs. Furthermore,
using precise terminology rather than 'volatile' or 'populated anew' avoids
confusing short-lived state with dynamically mutating resources or leaving scope
boundaries and accumulation behavior unspecified.

## Examples

Bad:

```cpp
// Bounds of rendering operations in the render pass.
SkIRect fRenderAreaBounds;

// Separate atlas because SaveRecord keyed clips are far more volatile.
DrawAtlasMgr fSaveRecordKeyAtlasMgr;

// The contents of this set are expected to be populated anew for each out-param helper.
SkTHashSet<const Variable*> fOutParamArgVars;

struct TexturePage {
    sk_sp<TextureProxy> fTexture;
    SkTDArray<uint8_t> fAlphaBuffer;
};
```

Good:

```cpp
// Bounds of rendering operations in current pass; undefined outside addRenderPass().
SkIRect fRenderAreaBounds;

// Separate atlas because SaveRecord keyed clips are transient (never reused once popped).
DrawAtlasMgr fSaveRecordKeyAtlasMgr;

// Uniquely scoped for each out-param helper; cleared every time a helper function is emitted.
SkTHashSet<const Variable*> fOutParamArgVars;

// Backing struct; fAlphaBuffer only stages data for a single pending upload.
struct TexturePage {
    sk_sp<TextureProxy> fTexture;
    SkTDArray<uint8_t> fAlphaBuffer;
};
```
