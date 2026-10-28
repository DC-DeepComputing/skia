---
name: skia/avoid-cached-prefix-for-fixed-properties
description: When declaring member variables for fixed or invariant object properties, avoid Cached or fCached prefixes; reserve Cached naming for mutable or lazily evaluated caches
rubric: readability
severity: nit
---

**Rule.** Do not use Cached or fCached... prefixes when declaring member
variables for fixed configuration, construction-time properties, or invariant
state stored for comparison or dynamic emission. Reserve Cached naming strictly
for mutable, memoized, or lazily evaluated caches that update over an object's
lifetime; name fixed properties directly after their semantic attributes (e.g.,
fPrimitiveType) or qualify them by their dynamic role.

**Why.** In Skia conventions, prefixing a member with Cached communicates that
the field is a mutable or lazily populated cache that updates over the object's
lifecycle. Using Cached for fixed properties misleadingly suggests the object
mutates or manages an invalidatable cache, obscuring object lifecycle and
thread-safety invariants.

## Examples

Bad:

```cpp
// Fixed pipeline configuration stored for dynamic state diffing:
PrimitiveType fCachedPrimitiveType;
DepthStencilSettings fCachedDepthStencilSettings;
```

Good:

```cpp
// Name fixed properties directly after their semantic attributes:
PrimitiveType fPrimitiveType;
DepthStencilSettings fDepthStencilSettings;
```
