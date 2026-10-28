---
name: skia/gpu/use-make-gl-external-for-external-textures
description: When creating backend formats for external GL textures, use GrBackendFormats::MakeGLExternal() instead of passing placeholder format enums to the deprecated MakeGL(format, target)
rubric: design
severity: should_fix
---

**Rule.** When creating backend formats for external GL textures (such as
imported Android HardwareBuffers), use GrBackendFormats::MakeGLExternal() rather
than passing a placeholder format enum to GrBackendFormats::MakeGL(format,
GR_GL_TEXTURE_EXTERNAL). Reserve MakeGL() for standard textures with
well-defined internal formats.

**Why.** External GL textures do not have a standard OpenGL internal format
enum. Requiring callers to supply an arbitrary placeholder format (such as
GR_GL_RGBA8) to MakeGL() is misleading and couples the backend format descriptor
to a fictitious layout; MakeGLExternal() encapsulates the external target
semantics cleanly.

## Examples

Bad:

```cpp
// Passing a placeholder GL format to MakeGL for an external texture
return GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_EXTERNAL);
```

Good:

```cpp
// Using the dedicated factory for external textures
return GrBackendFormats::MakeGLExternal();
```
