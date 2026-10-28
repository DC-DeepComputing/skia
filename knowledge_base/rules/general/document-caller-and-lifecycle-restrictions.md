---
name: skia/document-caller-and-lifecycle-restrictions
description: When declaring or refactoring restricted or friend-accessible methods, document caller constraints concisely and keep them synchronized with actual caller patterns
rubric: readability
severity: should_fix
---

**Rule.** When declaring or refactoring protected, friend-accessible, or
internal methods whose invocation is restricted to specific callers or lifecycle
phases, document caller invariants directly and concisely (e.g., 'This should
only ever be called by/from X'). Avoid speculating on hypothetical future
callers or manual wrapper bypasses. When refactoring caller patterns, keep
caller constraint comments synchronized and remove references to obsolete call
sites or internal invocations that no longer occur.

**Why.** C++ access specifiers cannot prevent all invalid invocations across
subclasses or friend declarations. Stating strict caller invariants directly
clarifies design intent and prevents state corruption, while keeping caller
comments synchronized when refactoring prevents misleading maintainers about
active encapsulation boundaries and internal call sites.

## Examples

Bad:

```cpp
// Update backend label to match fLabel and mark dirty as false.
void synchronizeBackendLabel();

// Note: Resource does not currently invoke this directly, but if modified
// to do so, it will need to manually set fBackendLabelDirty to false.
virtual void setBackendLabel(char const* label) {}

// Stale comment: internal calls were migrated to constructors.
// This method should only be called internally or by the ResourceCache.
void setLabel(std::string_view label);
```

Good:

```cpp
// Update backend label to match fLabel. This should only ever be called by
// ResourceCache or Resource subclass constructors.
void synchronizeBackendLabel();

// Enforces thread safety. This should only ever be called from within
// synchronizeBackendLabel().
virtual void setBackendLabel(char const* label) {}

// Updated to reflect active callers after internal invocations were refactored.
// This method is only expected to be called when returning a non-shareable
// resource from the ResourceCache.
void setLabel(std::string_view label);
```
