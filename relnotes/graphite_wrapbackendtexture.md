* `SkSurfaces::WrapBackendTexture` no longer requires providing an `SkColorType`. A closest
   compatible SkColorType will be chosen, so long as the backend texture's format is supported as
   renderable.
* `SkImages::WrapTexture` no longer requires providing an `SkColorType`. A closest compatible
   SkColorType will be chosen as long as the texture's format is supported and the texture is
   sampleable. Additionally, wrapped textures can now be forced to opaque (possibly via shader
   logic) using a `forceOpaque` boolean. This is a no-op for texture formats that do not have an
   alpha channel. This is a more reliable way to sample an opaque texture than trying to use
   kRGB_888x_SkColorType as it will work for any supported texture format.
