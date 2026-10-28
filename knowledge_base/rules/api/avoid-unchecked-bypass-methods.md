---
name: skia/api/avoid-unchecked-bypass-methods
description: When an accessor assertion proves too restrictive for valid use cases, relax the assertion and document caller invariants instead of adding parallel unchecked bypass methods
rubric: design
severity: should_fix
---

**Rule.** Do not introduce parallel `*_nocheck`, `*NoCheck`, or unchecked
accessor variants to bypass assertions that prove too restrictive for legitimate
use cases. Instead, remove or relax the assertion in the primary method and
clearly document safety expectations and threading constraints for callers in
header doc comments.

**Why.** Adding unchecked backdoor accessors bloats the public API, duplicates
maintenance surface, and fragments call sites across checked and unchecked
variants. If an assertion (such as single-ownership enforcement) prevents valid
multi-owner or concurrent workflows, relaxing the assertion directly and
documenting caller obligations preserves a single canonical interface.

## Examples

Bad:

```cpp
class SkData {
public:
    // Bad: Adding a parallel unchecked method to bypass a restrictive assert
    void* writable_data_nocheck() { return fSpan.data(); }

    void* writable_data() {
        SkASSERT(this->unique());
        return fSpan.data();
    }
};
```

Good:

```cpp
class SkData {
public:
    // Good: Relax the assert in the canonical method and document caller constraints
    /**
     *  USE WITH CAUTION.
     *  Be sure other 'owners' of this object are not accessing it in another thread.
     */
    void* writable_data() {
        return fSpan.data();
    }
};
```
