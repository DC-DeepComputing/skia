---
name: skia/api/name-factory-methods-after-specific-subtype
description: When declaring factory or conversion methods for specific subtypes or backing stores, name methods after the concrete subtype or representation rather than generic concepts
rubric: readability
severity: should_fix
---

**Rule.** When declaring factory or conversion methods that produce a specific
subtype or require a particular backing representation of a multi-backend type
(such as raster vs. texture images), name the method after the concrete subtype
or storage backing (e.g., makeScriptRunIterator, MakeFromRaster,
makeBitmapSurface) rather than generic base concepts (e.g., makeRunIterator,
MakeFromImage). Ensure factory names align with sibling methods and explicitly
document the specialized variant or storage representation accepted and created.

**Why.** Generic base names like makeRunIterator or MakeFromImage obscure the
concrete type created and mislead callers into expecting polymorphic support for
any backing store, masking runtime preconditions or null returns when
unsupported representations are passed. Naming factories after concrete subtypes
or backing stores aligns method names with exact types and makes call-site
intent unambiguous.

## Examples

Bad:

```cpp
// Bad: generic base concept obscures concrete subtype being created
class Factory {
public:
    virtual std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(...) const = 0;
    virtual std::unique_ptr<SkShaper::ScriptRunIterator> makeRunIterator(...) const = 0;
};

// Bad: generic type name implies support for all image backings
namespace SkSpecialImages {
sk_sp<SkSpecialImage> MakeFromImage(const SkIRect& subset,
                                    sk_sp<SkImage> image,
                                    const SkSurfaceProps& props);
}
```

Good:

```cpp
// Good: named after concrete subtype produced
class Factory {
public:
    virtual std::unique_ptr<SkShaper::BiDiRunIterator> makeBidiRunIterator(...) const = 0;
    virtual std::unique_ptr<SkShaper::ScriptRunIterator> makeScriptRunIterator(...) const = 0;
};

// Good: explicitly documents raster backing requirement
namespace SkSpecialImages {
sk_sp<SkSpecialImage> MakeFromRaster(const SkIRect& subset,
                                     sk_sp<SkImage> image,
                                     const SkSurfaceProps& props);
}
```
