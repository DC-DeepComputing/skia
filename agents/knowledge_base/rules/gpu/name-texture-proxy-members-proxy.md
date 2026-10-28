---
name: skia/gpu/name-texture-proxy-members-proxy
description: When declaring member variables holding TextureProxy or GrTextureProxy in GPU code, use a Proxy suffix rather than naming them fTexture
rubric: readability
severity: should_fix
---

**Rule.** When declaring struct or class member variables that hold a
TextureProxy (or GrTextureProxy), include a Proxy suffix in the identifier name
(such as fProxy, fTextureProxy, or f<Role>Proxy). Do not name proxy members
fTexture or f<Role>Texture, which misleadingly implies an instantiated GPU
texture resource rather than a deferred proxy handle.

**Why.** In Graphite and Ganesh, TextureProxy is a deferred proxy handle whose
backing GPU Texture resource may not yet be instantiated or allocated. Naming a
proxy member fTexture obscures the object's type and instantiation lifecycle,
creating confusion with concrete Texture and GrTexture classes.

## Examples

Bad:

```cpp
struct AtlasPage {
    // Bad: 'fTexture' misleadingly implies an instantiated Texture resource.
    sk_sp<TextureProxy> fTexture;
};
```

Good:

```cpp
struct AtlasPage {
    // Good: Proxy suffix clearly identifies the deferred proxy handle.
    sk_sp<TextureProxy> fTextureProxy;
    // Or role-specific: sk_sp<TextureProxy> fPageProxy;
};
```
