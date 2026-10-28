---
name: skia/gpu/document-copy-on-write-query-side-effects
description: When implementing GPU proxy or surface copy-on-write query methods with internal side effects, document state transitions, copy suppression, and ordering constraints
rubric: readability
severity: should_fix
---

**Rule.** In GPU proxy and surface copy-on-write query methods (such as
`surfaceMustCopyOnWrite`), document any non-obvious side effects, state
mutations, or ordering constraints in an explanatory comment. Explicitly
document conditions where returning false suppresses standard copy-on-write
mechanisms in favor of internal subsystem copy tasks, explaining why early state
transitions are required.

**Why.** Query and predicate methods are generally expected to be
side-effect-free inspections. When a query method mutates internal proxy state
or suppresses generic copy-on-write mechanisms to avoid duplicate copies,
undocumented behavior obscures lifecycle contracts and leads to texture sampling
conflicts or regressions.

## Examples

Bad:

```cpp
inline bool SkImage_Ganesh::ProxyChooser::surfaceMustCopyOnWrite(GrSurfaceProxy* surfaceProxy) {
    SkAutoSpinlock hold(fLock);
    auto surfaceID = surfaceProxy->underlyingUniqueID();

    if (fVolatileProxy && surfaceID == fVolatileProxy->underlyingUniqueID()) {
        fVolatileProxy.reset();
        fVolatileToStableCopyTask.reset();
        return false;
    }

    return surfaceID == fStableProxy->underlyingUniqueID();
}
```

Good:

```cpp
inline bool SkImage_Ganesh::ProxyChooser::surfaceMustCopyOnWrite(GrSurfaceProxy* surfaceProxy) {
    SkAutoSpinlock hold(fLock);
    auto surfaceID = surfaceProxy->underlyingUniqueID();

    // If we still have an fVolatileProxy and that proxy matches the surface we are drawing
    // into, then we will need to do a copy since we can't sample and draw into the same texture.
    // In this case we want to use the ProxyChooser's internal mechanisms for handling the copy
    // instead of generic copy-on-write systems in Skia to prevent duplicate copies.
    // Note: We can't depend on chooseProxy to trigger the copy since we may query the proxy
    // view before recording a new RenderTask to the surface.
    if (fVolatileProxy && surfaceID == fVolatileProxy->underlyingUniqueID()) {
        fVolatileProxy.reset();
        fVolatileToStableCopyTask.reset();
        return false;
    }

    return surfaceID == fStableProxy->underlyingUniqueID();
}
```
