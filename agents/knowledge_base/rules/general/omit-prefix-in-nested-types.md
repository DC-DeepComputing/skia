---
name: skia/omit-prefix-in-nested-types
description: When defining helper structs, metadata, or class-specific type aliases, nest them within the enclosing class and omit redundant class prefixes
rubric: readability
severity: should_fix
---

**Rule.** Declare or forward-declare helper structs, metadata types, and
class-specific type aliases (`using` or `typedef`) nested inside the class
definition they belong to (under `private:` when used strictly internally)
rather than at global or enclosing namespace scope. In source files (`.cpp`),
define nested types using the enclosing class scope (`Outer::Inner`). When
naming nested structs, classes, or type aliases, omit the enclosing class name
and `Sk` or `Gr` prefixes (e.g., prefer `Draw::BlitterChooser` or
`SkCapture::Metadata` over `SkBlitterChooser` or
`SkCapture::SkCaptureMetadata`), as the enclosing class scope already provides
the namespace context.

**Why.** Nesting closely coupled helper structs, metadata, and type aliases
within their enclosing class encapsulates implementation details, prevents
global namespace clutter, clarifies ownership, and limits symbol visibility.
Omitting library prefixes (`Sk` / `Gr`) and the enclosing class name in nested
types avoids redundant stutter and conforms to Skia style conventions.

## Examples

Bad:

```cpp
// Bad: Standalone top-level / file-scope helper struct with redundant prefix
struct SkCaptureMetadata {
    uint32_t version;
    uint32_t numPictures;
};

// Bad: Class-specific type alias declared in global namespace with Sk prefix
using SkBlitterChooser = SkBlitter* (*)(const SkPixmap&, const SkMatrix&);

class SkDrawBase {
public:
    SkBlitterChooser fBlitterChooser{nullptr};
};

// Bad: Forward-declared at namespace scope instead of nested in private:
struct SharedGeneratorData;
class ShaderInfo {
    void generateVertexSkSL(const SharedGeneratorData&);
};
```

Good:

```cpp
// Good: Nested within enclosing class with redundant prefixes omitted
class SkCapture : public SkRefCnt {
public:
    struct Metadata {
        uint32_t version;
        uint32_t numPictures;
    };
    Metadata fMetadata;
};

// Good: Class-specific type alias scoped inside class without Sk prefix
class SkDrawBase {
public:
    using BlitterChooser = SkBlitter* (*)(const SkPixmap&, const SkMatrix&);
    BlitterChooser fBlitterChooser{nullptr};
};

// Good: Internal helper forward-declared in private: and defined via class scope
class ShaderInfo {
private:
    struct SharedGeneratorData;
    void generateVertexSkSL(const SharedGeneratorData&);
};
struct ShaderInfo::SharedGeneratorData { ... };
```
