---
name: skia/gpu/assert-graphite-surface-proxy-instantiated
description: When accessing backing texture proxies of user-created Graphite SkSurfaces, assert they are instantiated with SkASSERT rather than redundantly calling instantiate
rubric: readability
severity: should_fix
---

**Rule.** When accessing the backing TextureProxy of a user-created or
client-provided SkSurface in Graphite, assert that the proxy is already
instantiated via SkASSERT(proxy->isInstantiated()). Do not call instantiate() at
runtime with defensive error handling or bailout branches.

**Why.** In Graphite, user-created SkSurfaces (such as wrapped backend textures
or render targets from factory methods) are unbudgeted or wrapped, guaranteeing
their backing texture proxies are immediately instantiated upon creation.
Calling instantiate() on them is redundant and introduces dead error-handling
code.

## Examples

Bad:

```cpp
TextureProxy* surfaceTexture = targetSurface->backingTextureProxy();
if (!surfaceTexture->instantiate(resourceProvider)) {
    SKGPU_LOG_E("Could not instantiate target texture proxy.");
    return nullptr;
}
```

Good:

```cpp
TextureProxy* surfaceTexture = targetSurface->backingTextureProxy();
SkASSERT(surfaceTexture->isInstantiated());
```
