---
name: skia/gpu/create-render-targets-with-texture-proxy-make
description: When creating render target proxies for DrawPass or test contexts, instantiate them via TextureProxy::Make with Renderable::kYes instead of CreateCachedProxy
rubric: correctness
severity: should_fix
---

**Rule.** When creating render target proxies (such as targets for a DrawPass or
mock rendering context), construct them using TextureProxy::Make with a
TextureInfo specifying Renderable::kYes from
Caps::getDefaultSampledTextureInfo(). Do not use
RecorderPriv::CreateCachedProxy() or ProxyCache to construct render target
proxies, as cached bitmap proxies are sample-only and not guaranteed to be
renderable.

**Why.** RecorderPriv::CreateCachedProxy() is intended for immutable sampled
bitmap textures and produces proxies marked as sample-only (Renderable::kNo).
Using a cached bitmap proxy as a DrawPass or render target triggers assertion
failures or validation errors when the backend pipeline attempts to bind the
texture as a color attachment.

## Examples

Bad:

```cpp
SkBitmap bmp;
bmp.tryAllocPixels(bmpInfo);
sk_sp<TextureProxy> targetProxy = RecorderPriv::CreateCachedProxy(recorder, bmp, "TextureProxy");
std::unique_ptr<DrawPass> drawPass = DrawPass::getTestDrawPass(targetProxy, ops, clearColor);
```

Good:

```cpp
auto textureInfo = recorder->priv().caps()->getDefaultSampledTextureInfo(
        colorType, skgpu::Mipmapped::kNo, skgpu::Protected::kNo, skgpu::Renderable::kYes);
sk_sp<TextureProxy> targetProxy = TextureProxy::Make(recorder->priv().caps(),
                                                     recorder->priv().resourceProvider(),
                                                     dimensions,
                                                     textureInfo,
                                                     "TargetProxy",
                                                     skgpu::Budgeted::kYes);
std::unique_ptr<DrawPass> drawPass = DrawPass::getTestDrawPass(targetProxy, ops, clearColor);
```
