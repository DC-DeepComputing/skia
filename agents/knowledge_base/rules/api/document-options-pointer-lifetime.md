---
name: skia/api/document-options-pointer-lifetime
description: When storing non-owning raw pointers in public options or configuration structs, explicitly document client lifetime requirements and precise operational scope
rubric: readability
severity: should_fix
---

**Rule.** When storing non-owning raw pointers, references, or buffers in public
options, descriptor, or configuration structs, explicitly document caller
lifetime requirements and precise operational scope in the field comment. State
clearly that the client must ensure the referenced object remains valid
throughout the lifetime of the receiving context(s) or engine, and describe
operational persistence precisely (such as 'across Context lifetimes') rather
than using ambiguous phrases like 'between runs'.

**Why.** Public options structs use non-owning raw pointers when callers manage
resource lifecycles or share objects across systems. Without explicit
documentation specifying that the referenced object must outlive the receiving
engine or context, API clients risk prematurely destroying resources and causing
use-after-free crashes. Clarifying operational scope prevents ambiguity about
persistence across context instances.

## Examples

Bad:

```cpp
struct ContextOptions {
    /**
     * Optional executor to handle threaded work within Graphite.
     */
    SkExecutor* fExecutor = nullptr;

    /**
     * Cache in which to store backend-specific data between runs.
     */
    PersistentCache* fPersistentCache = nullptr;
};
```

Good:

```cpp
struct ContextOptions {
    /**
     * Optional executor to handle threaded work within Graphite. It is up to the
     * client to ensure the SkExecutor remains valid throughout the lifetime of the Context.
     */
    SkExecutor* fExecutor = nullptr;

    /**
     * Allows Graphite to store Pipeline data across Context lifetimes. It is up to the
     * client to ensure the PersistentCache object remains valid throughout the lifetime
     * of the Context(s).
     */
    PersistentCache* fPersistentCache = nullptr;
};
```
