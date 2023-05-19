`GrTextureGenerator` now has a subclass `GrExternalGLTextureGenerator` which can be subclassed by
clients and used with `SkImages::DeferredFromTextureGenerator` in order to create images from GL
textures that were created outside of skia.