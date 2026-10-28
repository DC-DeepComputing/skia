---
name: skia/api/qualify-base-id-accessors
description: When defining accessor methods for identifiers originating from shared base abstractions, qualify the method with the conceptual domain rather than generic getId()
rubric: readability
severity: should_fix
---

**Rule.** When defining accessor methods that return an identifier originating
from a shared base abstraction (such as SkPixelStorage), qualify the method name
with the conceptual domain or base entity (e.g., getPixelStorageId()) rather
than a bare or generic getId(). Propagate this qualified naming across derived
classes, wrapper proxies, and forwarding accessors.

**Why.** Generic accessor names like getId() on derived classes or wrapper
proxies mislead readers into assuming the identifier uniquely identifies the
concrete subtype (such as a TextureProxy or Bitmap) rather than the underlying
storage. Explicit domain qualification clarifies the scope of uniqueness and
prevents identifier collisions or misinterpretations across subsystems sharing
the base abstraction.

## Examples

Bad:

```cpp
// In SkPixelStorage and derived classes / proxies:
uint32_t getId() const { return fID; }

// At call sites / wrappers:
uint32_t Surface::getPixelStorageID() const {
    return this->target().proxy()->getId();
}
```

Good:

```cpp
// In SkPixelStorage and derived classes / proxies:
uint32_t getPixelStorageId() const { return fID; }

// At call sites / wrappers:
uint32_t Surface::getPixelStorageID() const {
    return this->target().proxy()->getPixelStorageId();
}
```
